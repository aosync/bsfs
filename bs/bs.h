#ifndef BS_BS_H
#define BS_BS_H

#include <stdio.h>
#include <assert.h>
#include <rt.h>
#include <io/io.h>
#include <map/map.h>

PACK(struct bs_bootsec {
	u8 bootstrap[446];
	u16 signature;
	u64 free;
	u64 next;
	u64 rsvd;
	u64 nextin;
	u64 freein;
	u64 rootin;
	u8 blksz;
	u8 pad2[13];
	u16 magic;
});
typedef struct bs_bootsec BsBootsec;
static_assert(sizeof(BsBootsec) == 512, "BsBootsec is 512 bytes.");

PACK(struct bs_inode {
	u64 blk;
	u64 size;
	u64 modif;
	u8 owner[17];
	u8 group[17];
	u16 refs;
	u8 perms;
	u8 attr;
	u8 level;
	u8 rsvd;
});
typedef struct bs_inode BsInode;
static_assert(sizeof(BsInode) == 64, "BsInode is not 64 bytes");

struct bs_bs {
	IoFile *file;
	BsBootsec *bootsec;
	Map inodes; // XXX may be unneeded
};
typedef struct bs_bs Bs;

struct bs_file_inner {
	Bs *bs;
	u64 inid;
	BsInode inode;
	Map map;
};
typedef struct bs_file_inner BsFileInner;

PACK(struct bs_dirent {
	char name[24];
	u64 inid;
});
typedef struct bs_dirent BsDirent;
static_assert(sizeof(BsDirent) == 32, "BsDirent is not 32 bytes");

PACK(struct bs_dirhead {
	u64 next;
	u64 pad0[3];
});
typedef struct bs_dirhead BsDirhead;
static_assert(sizeof(BsDirhead) == 32, "BsDirhead is not 32 bytes");

void bs_init(Bs *self, IoFile *file);
int bs_build(Bs *self, u64 rsvd);
int bs_fetch(Bs *self);
int bs_commit(Bs *self);
void bs_finish(Bs *self);

u64 bs_block_alloc(Bs *self);
void bs_block_free(Bs *self, u64 blkid);

u64 bs_inode_alloc(Bs *self);
void bs_inode_free(Bs *self, u64 inid);
void bs_inode_fetch(Bs *self, BsInode *inode, u64 inid);
void bs_inode_commit(Bs *self, BsInode *inode, u64 inid);

IoFile *bs_get_file(Bs *self, u64 inid);
IoFile *bs_create_file(Bs *self);

void bs_file_trdir(IoFile *self);

IoFile *bs_root(Bs *self);
IoFile *bs_file_walk(IoFile *file, char *name);
IoFile *bs_file_create(IoFile *file, char *name);
IoFile *bs_file_mkdir(IoFile *file, char *name);
ssize_t bs_file_write(IoFile *file, void *buf, size_t n, off_t off);
ssize_t bs_file_read(IoFile *file, void *buf, size_t n, off_t off);
void bs_file_clunk(IoFile *file);


IoFile *bs_rsvd(Bs *self);

#endif