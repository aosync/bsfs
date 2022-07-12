#ifndef MAP_MAP_H
#define MAP_MAP_H

#include <io/io.h>

typedef struct bs_bs Bs;

struct map {
	IoFile *file;
	u64 addrs[8];
	u16 idx[8];
	u16 blksz;
	u8 invalidate;
	u64 miss;
	Bs *allocator;
	u8 *level;
	u64 *base;
};
typedef struct map Map;

u64 map_translate(Map *ctx, size_t addr);
void map_map(Map *ctx, u64 off);
void map_unmap(Map *ctx, u64 off);
void map_levelup(Map *ctx);
void map_leveldown(Map *ctx);

i8 map_idx(Map *ctx, u64 addr);
void map_clear(Map *ctx);

#endif