#include "cmd.h"

#include <string.h>
#include <io/io.h>

char *cmd_disk = "";
char *cmd_cmd_name = "";

char *cmd_islocal(char *name) {
	int len = strlen(name);
	if (len < 2 || name[0] != ':' || name[1] != ':')
		return nil;
	
	return name + 2;
}

// Attempts to open the disk, creates it if it doesn't exist
IoFile *cmd_create_disk() {
	IoFile *disk = file_create(cmd_disk, "rb+");
	if (disk == nil)
		disk = file_create(cmd_disk, "wb+");
	if (disk == nil)
		fprintf(stderr, "bsfs.%s: could not access disk %s\n", cmd_cmd_name, cmd_disk);
	return disk;
}

// Attempts to open the disk, errors if it doesn't exist
IoFile *cmd_open_disk() {
	IoFile *disk = file_create(cmd_disk, "rb+");
	if (disk == nil)
		fprintf(stderr, "bsfs.%s: could not access disk %s\n", cmd_cmd_name, cmd_disk);
	return disk;
}

// Walks at the desired name, handles local names starting with :: 
IoFile *cmd_walk_at(IoFile *root, char *name) {
	IoFile *file;
	char *local = nil;

	if ((local = cmd_islocal(name)))
		file = file_create(local, "rb+");
	else
		file = io_walk_at(root, name);

	if (file == nil)
		fprintf(stderr, "bsfs.%s: could not access file %s\n", cmd_cmd_name, name);

	return file;
}

// Creates a file with name, handles local names starting with ::
IoFile *cmd_create_at(IoFile *root, char *name) {
	IoFile *file;
	char *local;

	if ((local = cmd_islocal(name))) {
		file = file_create(local, "rb+");
		if (file == nil)
			file = file_create(local, "wb+");
	} else
		file = io_create_at(root, name);

	if (file == nil)
		fprintf(stderr, "bsfs.%s: could not access file %s\n", cmd_cmd_name, name);

	return file;
}

#ifdef _WIN64
#include <windows.h>
#define mkdir _mkdir
#else
#include <unistd.h>
// Call mkdir with 0o755
// XXX annoying assumption
#define mkdir(X) mkdir((X), 493)
#endif

// Makes a directory with name
int cmd_mkdir_at(IoFile *root, char *name) {
	char *local;
	int ret;

	if ((local = cmd_islocal(name)))
		ret = mkdir(local);
	else {
		IoFile *dir = io_mkdir_at(root, name);
		if (dir) {
			ret = 0;
			io_clunk(dir);
		} else
			ret = 1;
	}

	if (ret)
		fprintf(stderr, "bsfs.%s: could not create directory %s\n", cmd_cmd_name, name);

	return ret;
}