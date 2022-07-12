#include "rt.h"

#include <stdlib.h>
#include <stdio.h>

void *alloc(size_t n) {
	void *ptr = malloc(n);
	if (ptr == nil)
		exit(1);
	
	return ptr;
}

void *ralloc(void *ptr, size_t n) {
	ptr = realloc(ptr, n);
	if (ptr == nil)
		exit(1);
	
	return ptr;
}