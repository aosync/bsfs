#include "cmd.h"

#include <rt.h>
#include <stdio.h>

#include <bs/bs.h>
#include <io/io.h>

// Formats a disk beginning with a bootsector
// by taking into account the amount of reserved
// space needed.
// XXX stop assuming blksz = 12
int cmd_format(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "bsfs.format: need src\n");
		return 1;
	}

	char *srcname = cmd_islocal(argv[1]);
	if (srcname == nil) {
		fprintf(stderr, "bsfs.format: only local (::) paths are supported as a source for the format command\n");
		return 1;
	}
	IoFile *src = cmd_walk_at(nil, argv[1]);
	if (src == nil)
		return 1;

	IoFile *dst = cmd_create_disk();
	if (dst == nil)
		return 1;

	ssize_t count = io_copy(dst, src);
	if (count < 0) {
		fprintf(stderr, "bsfs.format: could not copy the data\n");
		return 1;
	}
	
	size_t rsvd = (count - 1) / 4096 + 1;

	fprintf(stderr, "bsfs.format: copied %lld bytes, reserving %llu blocks\n", count, rsvd);

	Bs bs;
	bs_init(&bs, dst);
	bs_build(&bs, rsvd);

	bs_finish(&bs);
	
	// Clunking the files
	io_clunk(dst);
	io_clunk(src);

	return 0;
}