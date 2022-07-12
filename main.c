#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include <io/io.h>
#include <bs/bs.h>
// #include <page/file.h>
#include <map/map.h>
#include <rt.h>

#include <cmd/cmd.h>

#define BLANK(X) (~((u64)-1 << (X)))

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "bsfs: need disk and command\n");
		return 1;
	}

	cmd_disk = argv[1];
	cmd_cmd_name = argv[2];

	argc -= 2;
	argv += 2;

	if (strcmp(argv[0], "format") == 0)
		return cmd_format(argc, argv);
	else if (strcmp(argv[0], "cat") == 0)
		return cmd_cat(argc, argv);
	else if (strcmp(argv[0], "cp") == 0)
		return cmd_cp(argc, argv);
	else if (strcmp(argv[0], "mkdir") == 0)
		return cmd_mkdir(argc, argv);
	else {
		fprintf(stderr, "bsfs: no such command %s\n", argv[0]);
		return 1;
	}

	IoFile *file = file_create("sample.iso", "rb+");
	//IoFile *kernel = file_create("kernel.elf", "rb");
	Bs bs;
	bs_init(&bs, file);
	//bs_build(&bs, 1);

	IoFile *root = bs_root(&bs);
	IoFile *kern = io_walk_at(root, argv[1]);
	//IoFile *boot = io_walk(root, "boot");
	//IoFile *kern = io_walk(boot, "kernel");
	/*IoFile *boot = io_mkdir(root, "boot");
	IoFile *kern = io_create(boot, "kernel");
	
	io_copy(kern, kernel);*/

	char tmp[100];
	ssize_t rc = io_read(kern, tmp, 100);
	tmp[rc] = '\0';
	printf("Read %lld bytes:\n%s\n", rc, tmp);

	io_clunk(kern);
	//io_clunk(boot);
	io_clunk(root);

	bs_commit(&bs);
	bs_finish(&bs);

	// io_clunk(kernel);
	io_clunk(file);
	



	/*char c;
	while (io_read(kern, &c, 1) > 0) {
		printf("%c", c);
	}*/

	/*IoFile *boot = io_mkdir(root, "boot");
	IoFile *kern = io_create(boot, "kernel");*/

	/*IoFile *file = file_create("test.txt", "rb+");
	if (!file)
		return 1;

	Bs bs;
	bs_init(&bs, file);
	bs_fetch(&bs);
	IoFile *rsvd = bs_rsvd(&bs);
	io_write(rsvd, "bruh", 4);

	// bs_build(&bs, 10);

	printf("got here %lld\n", bs_block_alloc(&bs));

	bs_commit(&bs);
	bs_finish(&bs);

	io_clunk(rsvd);
	io_clunk(file);*/

	return 0;
}
