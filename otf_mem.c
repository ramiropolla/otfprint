#include <stdio.h>

#include "otf_mem.h"

void *x_malloc_(size_t size, const char *func, int line)
{
	void *p = malloc(size);
	if (!p) {
		fprintf(stderr, "%s no mem (line: %d)\n", func, line);
		exit(-1);
	}
	return p;
}

void *x_calloc_(size_t nmemb, size_t size, const char *func, int line)
{
	void *p = calloc(nmemb, size);
	if (!p) {
		fprintf(stderr, "%s no mem (line: %d)\n", func, line);
		exit(-1);
	}
	return p;
}
