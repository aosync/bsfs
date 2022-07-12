#include <stdio.h>
#include <string.h>

#include <cmd/cmd.h>

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
}
