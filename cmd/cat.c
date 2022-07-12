#include "cmd.h"

#include <stdio.h>
#include <bs/bs.h>
#include <io/io.h>

int cmd_cat(int argc, char **argv) {
	IoFile *disk = cmd_open_disk();

	Bs bs;
	bs_init(&bs, disk);

	IoFile *root = bs_root(&bs);
	if (root == nil) {
		fprintf(stderr, "bsfs.cat: could not access /\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		IoFile *f = cmd_walk_at(root, argv[i]);
		if (f == nil)
			continue;

		ssize_t c;
		char tmp[65];
		while ((c = io_read(f, tmp, 64)) > 0) {
			tmp[c] = '\0';
			printf("%s", tmp);
		}

		io_clunk(f);
	}
	
	io_clunk(root);
	bs_finish(&bs);
	io_clunk(disk);

	return 0;
}