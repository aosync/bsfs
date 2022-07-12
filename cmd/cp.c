#include "cmd.h"

#include <stdio.h>

#include <bs/bs.h>
#include <io/io.h>

int cmd_cp(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "bsfs.cp: at least 2 operands are needed\n");
		return 1;
	}

	IoFile *disk = cmd_open_disk();

	Bs bs;
	bs_init(&bs, disk);

	IoFile *root = bs_root(&bs);
	if (root == nil) {
		fprintf(stderr, "bsfs.cp: could not get /\n");
		return 1;
	}

	IoFile *src = cmd_walk_at(root, argv[1]);
	IoFile *dst = cmd_create_at(root, argv[argc - 1]);

	io_copy(dst, src);

	io_clunk(dst);
	io_clunk(src);
	io_clunk(root);

	bs_finish(&bs);

	io_clunk(disk);

	return 0;
}