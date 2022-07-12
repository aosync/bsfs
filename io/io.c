#include "io.h"

// Performs a write relative to the cursor
ssize_t io_write(IoFile *file, void *buf, size_t n) {
	if (file->write == nil)
		return 0;
	
	ssize_t dis = file->write(file, buf, n, file->pos);
	if (dis >= 0)
		file->pos += dis;
	return dis;
}

// Performs a read relative to the cursor
ssize_t io_read(IoFile *file, void *buf, size_t n) {
	if (file->read == nil)
		return 0;
	
	ssize_t dis = file->read(file, buf, n, file->pos);
	if (dis >= 0)
		file->pos += dis;
	return dis;
}

// Performs an absolute write in a file
ssize_t io_pwrite(IoFile *file, void *buf, size_t n, off_t off) {
	if (file->write == nil)
		return 0;
	
	return file->write(file, buf, n, off);
}

// Performs an absolute read in a file
ssize_t io_pread(IoFile *file, void *buf, size_t n, off_t off) {
	if (file->read == nil)
		return 0;
	
	return file->read(file, buf, n, off);
}

// Creates a file
IoFile *io_create(IoFile *file, char *name) {
	if (file->create == nil)
		return nil;
	
	return file->create(file, name);
}

// Walks in a file
IoFile *io_walk(IoFile *file, char *name) {
	if (file->walk == nil)
		return nil;
	
	return file->walk(file, name);
}

// Clunks a file (frees it)
void io_clunk(IoFile *file) {
	if (file->clunk == nil)
		return;
		
	// printf("lmfao\n");
	
	file->clunk(file);
}

// Create a directory in a file
IoFile *io_mkdir(IoFile *file, char *name) {
	if (file->mkdir == nil)
		return nil;
	
	return file->mkdir(file, name);
}

#include <string.h>

IoFile *io_basedir(IoFile *file, char *name, char *basename) {
	IoFile *tmp, *prev = file;

	char *orig = alloc(strlen(name) + 1);
	strcpy(orig, name);

	char *tok, copy[128];
	strcpy(copy, "");

	for (tok = strtok(name, "/"); tok; strcpy(copy, tok), tok = strtok(nil, "/")) {
		if (strcmp(copy, "") == 0)
			continue;

		tmp = io_walk(prev, copy);

		if (prev != file)
			io_clunk(prev);
		
		if (tmp == nil)
			break;

		prev = tmp;
	}

	if (basename)
		strcpy(basename, copy);

	strcpy(name, orig);
	free(orig);

	return prev;
}

IoFile *io_walk_at(IoFile *file, char *name) {
	char base[128];
	IoFile *tmp = io_basedir(file, name, base);
	if (tmp == nil)
		return nil;

	IoFile *new = io_walk(tmp, base);

	if (tmp != file)
		io_clunk(tmp);
	
	return new;
}

IoFile *io_create_at(IoFile *file, char *name) {
	char base[128];
	IoFile *tmp = io_basedir(file, name, base);
	if (tmp == nil)
		return nil;
	
	IoFile *new = io_create(tmp, base);

	if (tmp != file)
		io_clunk(tmp);
	
	return new;
}

IoFile *io_mkdir_at(IoFile *file, char *name) {
	char base[128];
	IoFile *tmp = io_basedir(file, name, base);
	if (tmp == nil)
		return nil;

	IoFile *new = io_mkdir(tmp, base);

	if (tmp != file)
		io_clunk(tmp);
	
	return new;
}

// Performs the memset operation at the file cursor
ssize_t io_memset(IoFile *file, int c, size_t n) {
	ssize_t count = io_pmemset(file, c, n, file->pos);
	if (count >= 0)
		file->pos += count;
	return count;
}

// Sets the n bytes at off in file to c
ssize_t io_pmemset(IoFile *file, int c, size_t n, off_t off) {
	void *data = alloc(n);
	
	memset(data, c, n);

	ssize_t count = io_pwrite(file, data, n, off);
	
	free(data);
	return count;
}

ssize_t io_copy(IoFile *dst, IoFile *src) {
	ssize_t count = 0;
	char buf[512];
	ssize_t snt;
	ssize_t rcv;

	do {
		snt = io_read(src, buf, 512);
		if (snt < 0)
			break;

		rcv = io_write(dst, buf, snt);
		if (rcv < 0)
			break;

		count += rcv;
	} while (snt && rcv && snt == rcv);

	return count;
}

IoFile *file_create(char *name, char *mode) {
	FILE *fp = fopen(name, mode);
	if (fp == nil)
		return nil;
	
	IoFileInner *inner = alloc(sizeof(IoFileInner));
	inner->fp = fp;

	IoFile *file = alloc(sizeof(IoFile));

	*file = (IoFile) {
		.inner = inner,
		.write = file_write,
		.read = file_read,
		.clunk = file_clunk
	};

	return file;
}

ssize_t file_write(IoFile *file, void *buf, size_t n, off_t off) {
	IoFileInner *inner = file->inner;
	if (fseek(inner->fp, off, SEEK_SET))
		return -1;
	return fwrite(buf, 1, n, inner->fp);
}

ssize_t file_read(IoFile *file, void *buf, size_t n, off_t off) {
	IoFileInner *inner = file->inner;
	if (fseek(inner->fp, off, SEEK_SET))
		return -1;
	return fread(buf, 1, n, inner->fp);
}

void file_clunk(IoFile *file) {
	IoFileInner *inner = file->inner;
	fclose(inner->fp);
	free(inner);
	free(file);
}