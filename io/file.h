#ifndef IO_FILE_H
#define IO_FILE_H

#include <rt.h>

struct io_file;
typedef struct io_file IoFile;

typedef ssize_t (*IoWriter)(IoFile*, void*, size_t, off_t);
typedef ssize_t (*IoReader)(IoFile*, void*, size_t, off_t);
typedef IoFile *(*IoCreator)(IoFile*, char*);
typedef IoFile *(*IoWalker)(IoFile*, char*);
typedef IoFile *(*IoMkdirer)(IoFile*, char*);
typedef void (*IoClunker)(IoFile*);

struct io_file {
	void *inner;
	IoWriter write;
	IoReader read;
	IoCreator create;
	IoWalker walk;
	IoMkdirer mkdir;
	IoClunker clunk;
	off_t pos;
};
typedef struct io_file IoFile;

#endif