#include "map.h"

#include <bs/bs.h>

#define BLKSZ(X) (1 << (X))
#define BLANK(X) (~((u64)-1 << (X)))

u64 map_translate(Map *ctx, size_t addr) {
	int highest = map_idx(ctx, addr);
	ctx->invalidate = 0;

	u8 level = 0;
	for (int i = 0; i < 8; i++)
		if (ctx->idx[i])
			level = i;
	
	if (level > *ctx->level)
		return 0;

	u16 size = BLKSZ(ctx->blksz);
	u64 next = ctx->addrs[highest];

	for (int i = highest; i > 0; i--) {
		ctx->addrs[i] = next;

		u64 loc = next * size + ctx->idx[i]*8;
		io_pread(ctx->file, &next, 8, loc);
	
		// In case of miss, set miss to be the address
		if (next == 0)
			return 0;
	}


	ctx->addrs[0] = next;
	return next * size + ctx->idx[0];
}

void map_map(Map *ctx, u64 off) {
	u16 size = BLKSZ(ctx->blksz);
	map_clear(ctx);

	// Translate the address to build a cache from which we
	// compute how many times we have to level up to map the block
	map_idx(ctx, off * size);

	u8 level = 0;
	for (int i = 0; i < 8; i++)
		if (ctx->idx[i])
			level = i;

	// Level up
	for (int i = *ctx->level; i < level; i++)
		map_levelup(ctx);
	
	// Map all the levels
	map_idx(ctx, off * size);
	for (int i = *ctx->level; i > 0; i--) {
		u64 loc = ctx->addrs[i] * size + ctx->idx[i] * 8;
		u64 next;
		io_pread(ctx->file, &next, 8, loc);

		// If already mapped, we can continue
		if (next != 0) {
			ctx->addrs[i - 1] = next;
			continue;
		}
		
		// Else, allocate
		next = bs_block_alloc(ctx->allocator);
		io_pwrite(ctx->file, &next, 8, loc);

		// Update the cache
		ctx->addrs[i - 1] = next;
	}
}

void map_unmap(Map *ctx, u64 off) {
	if (*ctx->level == 0)
		return;

	u16 size = BLKSZ(ctx->blksz);
	map_clear(ctx);

	u64 ret = map_translate(ctx, off * size);
	if (ret == 0)
		return;
	
	// Free first top-level block
	bs_block_free(ctx->allocator, ctx->addrs[0]);
	// Unlink current block from next block
	u64 loc = ctx->addrs[1] * size + ctx->idx[1] * 8;
	u64 zero = 0;
	io_pwrite(ctx->file, &zero, 8, loc);

	u8 *what = alloc(size);

	for (int i = 1; i < *ctx->level; i++) {
		// If mid-level block, see if empty, if yes, free
		io_pread(ctx->file, what, size, ctx->addrs[i] * size);
		int j;
		for (j = 0; j < size; j++)
			if (what[j] != 0)
				break;
		if (j == size) {
			// Unlink current block from next block
			loc = ctx->addrs[i + 1] * size + ctx->idx[i + 1] * 8;
			io_pwrite(ctx->file, &zero, 8, loc);
			bs_block_free(ctx->allocator, ctx->addrs[i]);
		}
	}

	// Unlevel as many times needed
redo:
	if (*ctx->level > 0) {
		io_pread(ctx->file, what, size, ctx->addrs[*ctx->level] * size);
		int i;
		for (i = 8; i < size; i++)
			if (what[i] != 0)
				break;
		if (i == size) {
			map_leveldown(ctx);
			goto redo;
		}
	}

	free(what);
}

#include <stdio.h>

void map_levelup(Map *ctx) {
	*ctx->base = bs_block_alloc(ctx->allocator);
	printf("levelup: %d -> %d\n", *ctx->level, *ctx->level + 1);


	u16 size = BLKSZ(ctx->blksz);
	io_pwrite(ctx->file, &ctx->addrs[*ctx->level], 8, *ctx->base * size);

	(*ctx->level)++;

	map_clear(ctx);
}

void map_leveldown(Map *ctx) {
	u16 size = BLKSZ(ctx->blksz);
	io_pread(ctx->file, &ctx->addrs[*ctx->level - 1], 8, ctx->addrs[*ctx->level] * size);
	bs_block_free(ctx->allocator, ctx->addrs[*ctx->level]);
	printf("leveldown: %d -> %d\n", *ctx->level, *ctx->level - 1);

	*ctx->base = ctx->addrs[--(*ctx->level)];

	map_clear(ctx);
}

i8 map_idx(Map *ctx, u64 addr) {
	u8 newidx;

	i8 highest = -1;

	newidx = addr & BLANK(ctx->blksz);
	if (newidx != ctx->idx[0])
		highest = 0;
	ctx->idx[0] = newidx;
	addr >>= ctx->blksz;

	for (int i = 1; i < 8; i++) {
		u8 newidx = addr & BLANK(9);
		
		if (newidx != ctx->idx[i])
			highest = i;

		ctx->idx[i] = newidx;
		addr >>= 9;
	}

	if (ctx->invalidate)
		highest = *ctx->level;

	return highest;
}

void map_clear(Map *ctx) {
	for (int i = 0; i < 8; i++)
		ctx->idx[i] = 0;
	for (int i = 0; i < 8; i++)
		ctx->addrs[i] = 0;
	ctx->addrs[*ctx->level] = *ctx->base;
	ctx->invalidate = 1;
}
