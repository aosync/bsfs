#include "bs.h"

#include <io/limit.h>
#include <stdio.h>

#define BLKSZ(X) (1 << (X))
#define BLANK(X) (~((u64)-1 << (X)))

// Initializes a Bs structure from the contents
// of a file.
void bs_init(Bs *self, IoFile *file) {
	self->file = file;
	self->bootsec = alloc(sizeof(BsBootsec));
	bs_fetch(self);
}

int bs_validate(Bs *self) {
	if (self->bootsec->magic != 0xAA55)
		return 1;

	if (self->bootsec->signature != 0xCAAA)
		return 2;
	
	return 0;
}

int bs_build(Bs *self, u64 rsvd) {
	self->bootsec->signature = 0xCAAA;
	self->bootsec->free = 0;
	self->bootsec->next = rsvd + 1;
	self->bootsec->rsvd = rsvd + 1;
	self->bootsec->blksz = 12;
	self->bootsec->nextin = 0;
	self->bootsec->freein = 0;
	self->bootsec->rootin = 0;
	self->bootsec->magic = 0xAA55;
	return bs_commit(self);
}

// Fetches the state of the bootsector from the disk
int bs_fetch(Bs *self) {
	return io_pread(self->file, self->bootsec, sizeof(BsBootsec), 0);
}

// Commits the state of the bootsector to disk
int bs_commit(Bs *self) {
	return io_pwrite(self->file, self->bootsec, sizeof(BsBootsec), 0);
}

void bs_finish(Bs *self) {
	free(self->bootsec);
}

u64 bs_block_alloc(Bs *self) {
	u16 size = BLKSZ(self->bootsec->blksz);

	u64 blkid;

	if (self->bootsec->free) {
		blkid = self->bootsec->free;

		u64 free0;
		io_pread(self->file, &free0, sizeof(u64), blkid * size);

		self->bootsec->free = free0;
	} else
		blkid = self->bootsec->next++;

	// Clean the block
	io_pmemset(self->file, 0, size, blkid * size);

	// Update the bootsector
	bs_commit(self);

	// Debug message
	printf("Allocation of block(%lld) at %lld\n", blkid, blkid * size);

	return blkid;
}

void bs_block_free(Bs *self, u64 blkid) {
	u16 size = BLKSZ(self->bootsec->blksz);

	u64 free0 = self->bootsec->free;
	self->bootsec->free = blkid;

	io_pwrite(self->file, &free0, sizeof(u64), blkid * size);

	// Update the bootsector
	bs_commit(self);

	// Debug message
	printf("Freeing block(%lld) at %lld\n", blkid, blkid * size);
}

// Allocates an inode
u64 bs_inode_alloc(Bs *self) {
	u64 inid;

	if (self->bootsec->freein) {
		inid = self->bootsec->freein;

		BsInode in;
		bs_inode_fetch(self, &in, inid);

		// When an inode is free, blk represents the free1 inid
		self->bootsec->freein = in.blk;
	} else {
		if (self->bootsec->nextin == 0)
			self->bootsec->nextin = (bs_block_alloc(self) << self->bootsec->blksz) >> 6;
		
		inid = self->bootsec->nextin++;

		if (!((self->bootsec->nextin << 6) % BLKSZ(self->bootsec->blksz)))
			self->bootsec->nextin = 0;
	}

	// Clean the anode and set allocated bit to one
	BsInode in = (BsInode) {
		.rsvd = 1,
	};

	bs_inode_commit(self, &in, inid);
	
	// Update the bootsector
	bs_commit(self);

	//Debug message
	printf("Allocation of inode(%lld) at %lld\n", inid, inid << 6);

	return inid;
}

// Frees an inode
// BUG: the current design of the inode freelist prohibits
// the deallocation of an inode block, meaning that it is not
// possible to reuse an unused inode block for other data.
void bs_inode_free(Bs *self, u64 inid) {
	// Commits the inode in the freelist
	BsInode in = (BsInode) {
		.blk = self->bootsec->freein,
		.rsvd = 0,
	};

	bs_inode_commit(self, &in, inid);

	self->bootsec->freein = inid;

	// Update the bootsector
	bs_commit(self);

	// Debug message
	printf("Freeing inode(%lld) at %lld\n", inid, inid << 6);
}

// Fetches the inode with inid
void bs_inode_fetch(Bs *self, BsInode *inode, u64 inid) {
	u64 inaddr = inid << 6;
	io_pread(self->file, inode, 64, inaddr);
}

// Commits the inode with inid
void bs_inode_commit(Bs *self, BsInode *inode, u64 inid) {
	u64 inaddr = inid << 6;
	io_pwrite(self->file, inode, 64, inaddr);
}

// Get the file corresponding to inode with inid
IoFile *bs_get_file(Bs *self, u64 inid) {
	if (inid == 0)
		return nil;

	BsInode in;
	bs_inode_fetch(self, &in, inid);
	if (in.rsvd == 0)
		return nil;
	
	BsFileInner *inner = alloc(sizeof(BsFileInner));
	*inner = (BsFileInner) {
		.bs = self,
		.inid = inid,
		.inode = in,
		.map = (Map) {
			.file = self->file,
			.blksz = self->bootsec->blksz,
			.allocator = self,
			.level = &inner->inode.level,
			.base = &inner->inode.blk
		}
	};
	map_clear(&inner->map);

	IoFile *file = alloc(sizeof(IoFile));
	*file = (IoFile) {
		.inner = inner,
		.write = bs_file_write,
		.read = bs_file_read,
		.walk = bs_file_walk,
		.create = bs_file_create,
		.mkdir = bs_file_mkdir,
		.clunk = bs_file_clunk,
	};

	return file;
}

// Creates a raw file
IoFile *bs_create_file(Bs *self) {
	u64 inid = bs_inode_alloc(self);
	if (inid == 0)
		return nil;

	IoFile *file = bs_get_file(self, inid);
	if (file == nil)
		goto file;

	// Allocate a first block associated with the file
	BsInode *inode = &((BsFileInner *)file->inner)->inode;
	inode->blk = bs_block_alloc(self);
	if (inode->blk == 0)
		goto inodeblk;

	bs_inode_commit(self, inode, inid);

	return file;

inodeblk:
	io_clunk(file);
file:
	bs_inode_free(self, inid);
	return nil;
}

// Turns a file into a directory
void bs_file_trdir(IoFile *self) {
	BsFileInner *inner = self->inner;

	// Flip directory bit (this allows walking and creating and such)
	inner->inode.attr |= 1;

	BsDirhead dirhead = (BsDirhead) {
		.next = 1,
	};

	// Initialize the directory head
	io_pwrite(self, &dirhead, sizeof(BsDirhead), 0);

	// Update the inode
	bs_inode_commit(inner->bs, &inner->inode, inner->inid);
}

// Get the root of the filesystem
IoFile *bs_root(Bs *self) {
	if (self->bootsec->rootin == 0) {
		// If no root, create it and make it a directory
		IoFile *root = bs_create_file(self);

		bs_file_trdir(root);

		// Update the bootsector
		self->bootsec->rootin = ((BsFileInner*)(root->inner))->inid;
		bs_commit(self);

		return root;
	} else
		return bs_get_file(self, self->bootsec->rootin);
}

ssize_t bs_file_write(IoFile *file, void *buf, size_t n, off_t off) {
	BsFileInner *inner = file->inner;
	u16 size = BLKSZ(inner->bs->bootsec->blksz);
	map_clear(&inner->map);

	u8 *src = buf;
	u8 *dst = alloc(size);

	ssize_t count = 0;

	while (count < n) {
		u64 addr = map_translate(&inner->map, count + off);
		if (addr == 0) {
			map_map(&inner->map, (count + off) >> inner->bs->bootsec->blksz);
			addr = map_translate(&inner->map, count + off);
			if (addr == 0)
				return -1;
		}

		addr -= addr % size;
		
		io_pread(inner->bs->file, dst, size, addr);
		size_t head = (count + off) % size;
		while (count < n && head < size) {
			dst[head] = src[count];

			count++;
			head++;
		}

		io_pwrite(inner->bs->file, dst, size, addr);
	}

	free(dst);

	// Update the size of the file
	if (off + count > inner->inode.size)
		inner->inode.size = off + count;

	// Commit the inode
	bs_inode_commit(inner->bs, &inner->inode, inner->inid);

	return count;
}

ssize_t bs_file_read(IoFile *file, void *buf, size_t n, off_t off) {
	BsFileInner *inner = file->inner;
	u16 size = BLKSZ(inner->bs->bootsec->blksz);

	// Adjust the number of bytes to read
	if (off > inner->inode.size)
		return 0;
	if (off + n > inner->inode.size)
		n = inner->inode.size - off;

	map_clear(&inner->map);

	u8 *dst = buf;
	u8 *src = alloc(size);

	ssize_t count = 0;

	while (count < n) {
		u64 addr = map_translate(&inner->map, count + off);
		if (addr == 0)
			return -1;

		addr -= addr % 4096;
		
		io_pread(inner->bs->file, src, size, addr);
		size_t head = (count + off) % size;
		while (count < n && head < size) {
			dst[count] = src[head];

			count++;
			head++;
		}
	}

	free(src);

	return count;
}

IoFile *bs_file_walk(IoFile *file, char *name) {
	BsFileInner *inner = file->inner;

	// If not dir
	if (!(inner->inode.attr & 1))
		return nil;

	BsDirhead dh;
	file->pos = 0;
	if (io_read(file, &dh, sizeof(BsDirhead)) != sizeof(BsDirhead))
		return nil;

	BsDirent ent;
	for (size_t i = 1; i < dh.next; i++) {
		if (io_read(file, &ent, sizeof(BsDirent)) != sizeof(BsDirent))
			return nil;

		if (strcmp(name, ent.name) == 0)
			return bs_get_file(inner->bs, ent.inid);
	}

	return nil;
}

IoFile *bs_file_create(IoFile *file, char *name) {
	BsFileInner *inner = file->inner;

	// If not dir
	if (!(inner->inode.attr & 1))
		return nil;

	BsDirhead dh;
	if (io_pread(file, &dh, sizeof(BsDirhead), 0) != sizeof(BsDirhead))
		return nil;

	u64 loc = dh.next++;

	IoFile *new = bs_create_file(inner->bs);

	BsDirent ent;
	ent.inid = ((BsFileInner*)new->inner)->inid;
	strcpy(ent.name, name);
	// XXX increment new->inner->refs

	if (io_pwrite(file, &ent, sizeof(BsDirent), 32*loc) != sizeof(BsDirent))
		return nil;

	if (io_pwrite(file, &dh, sizeof(BsDirhead), 0) != sizeof(BsDirhead))
		return nil;

	return new;
}

IoFile *bs_file_mkdir(IoFile *file, char *name) {
	IoFile *dir = io_create(file, name);
	if (dir == nil)
		return nil;

	bs_file_trdir(dir);

	return dir;
}


void bs_file_clunk(IoFile *file) {
	BsFileInner *inner = file->inner;

	// Do a last save of the inode
	bs_inode_commit(inner->bs, &inner->inode, inner->inid);

	free(inner);
	free(file);
}

IoFile *bs_rsvd(Bs *self) {
	u64 base = 512;
	u64 limit = self->bootsec->rsvd * BLKSZ(self->bootsec->blksz);
	limit -= base;

	return io_limit_create(self->file, base, limit);
}