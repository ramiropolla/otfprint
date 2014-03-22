#include <stdio.h>

#include "otf_mem.h"

void *x_malloc_(size_t size, const char *func, int line)
{
	void *p = malloc(size);
	if (!p) {
		fprintf(stderr, "%s no mem (line: %d, size %d)\n", func, line, (int) size);
		exit(-1);
	}
	return p;
}

void *x_calloc_(size_t nmemb, size_t size, const char *func, int line)
{
	void *p = calloc(nmemb, size);
	if (!p) {
		fprintf(stderr, "%s no mem (line: %d, size %d)\n", func, line, (int) size);
		exit(-1);
	}
	return p;
}

void *x_realloc_(void *ptr, size_t size, const char *func, int line)
{
	void *p = realloc(ptr, size);
	if (!p) {
		fprintf(stderr, "%s no mem (line: %d, size %d)\n", func, line, (int) size);
		exit(-1);
	}
	return p;
}
