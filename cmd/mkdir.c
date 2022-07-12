#include "cmd.h"

#include <stdio.h>

#include <bs/bs.h>
#include <io/io.h>

// XXX Rewrite using a cmd_mkdir (analog to cmd_open) function for it to work
// with host and local filesystems

int cmd_mkdir(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "bsfs.mkdir: need at least 1 operand\n");
		return 1;
	}

	IoFile *disk = cmd_open_disk();

	Bs bs;
	bs_init(&bs, disk);

	IoFile *root = bs_root(&bs);

	for (int i = 1; i < argc; i++)
		cmd_mkdir_at(root, argv[i]);

	io_clunk(root);
	bs_finish(&bs);
	io_clunk(disk);

	return 0;
}