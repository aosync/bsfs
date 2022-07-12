#include "limit.h"

#include <rt.h>

IoFile *io_limit_create(IoFile *parent, u64 base, u64 limit) {
	IoLimitInner *inner = alloc(sizeof(IoLimitInner));
	
	inner->parent = parent;
	inner->base = base;
	inner->limit = limit;

	IoFile *self = alloc(sizeof(IoFile));
	*self = (IoFile) {
		.inner = inner,
		.write = io_limit_write,
		.read = io_limit_read,
		.clunk = io_limit_clunk
	};

	return self;
}

ssize_t io_limit_write(IoFile *self, void *buf, size_t n, off_t off) {
	IoLimitInner *inner = self->inner;
	
	if (off >= inner->limit)
		return 0;
	if (off + n > inner->limit)
		n = inner->limit - off;
	
	return io_pwrite(inner->parent, buf, n, inner->base + off);
}

ssize_t io_limit_read(IoFile *self, void *buf, size_t n, off_t off) {
	IoLimitInner *inner = self->inner;

	if (off >= inner->limit)
		return 0;
	if (off + n > inner->limit)
		n = inner->limit - off;
	
	return io_pread(inner->parent, buf, n, inner->base + off);
}

void io_limit_clunk(IoFile *self) {
	free(self->inner);
	free(self);
}