#ifndef IO_LIMIT_H
#define IO_LIMIT_H

#include "io.h"

struct io_limit_inner {
	IoFile *parent;
	u64 base;
	u64 limit;
};
typedef struct io_limit_inner IoLimitInner;

IoFile *io_limit_create(IoFile *parent, u64 base, u64 limit);
ssize_t io_limit_write(IoFile *self, void *buf, size_t n, off_t off);
ssize_t io_limit_read(IoFile *self, void *buf, size_t n, off_t off);
void io_limit_clunk(IoFile *self);

#endif