#ifndef IO_IO_H
#define IO_IO_H

#include <rt.h>
#include <stdio.h>

#include "file.h"

ssize_t io_write(IoFile *file, void *buf, size_t n);
ssize_t io_read(IoFile *file, void *buf, size_t n);
ssize_t io_pwrite(IoFile *file, void *buf, size_t n, off_t off);
ssize_t io_pread(IoFile *file, void *buf, size_t n, off_t off);
void io_clunk(IoFile *file);
IoFile *io_mkdir(IoFile *file, char *name);
IoFile *io_create(IoFile *file, char *name);
IoFile *io_walk(IoFile *file, char *name);

// XXX Clean and rename the following functions
IoFile *io_walk_at(IoFile *file, char *name);
IoFile *io_basedir(IoFile *file, char *name, char *basename);
IoFile *io_create_at(IoFile *file, char *name);
IoFile *io_mkdir_at(IoFile *file, char *name);

ssize_t io_copy(IoFile *dst, IoFile *src);
ssize_t io_memset(IoFile *file, int c, size_t n);
ssize_t io_pmemset(IoFile *file, int c, size_t n, off_t off);

struct io_file_inner {
	FILE *fp;
};
typedef struct io_file_inner IoFileInner;

IoFile *file_create(char *name, char *mode);
ssize_t file_write(IoFile *file, void *buf, size_t n, off_t off);
ssize_t file_read(IoFile *file, void *buf, size_t n, off_t off);
void file_clunk(IoFile *file);

#endif
