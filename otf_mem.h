#ifndef OTF_MEM_H
#define OTF_MEM_H

#include <string.h>
#include <stdlib.h>

void *x_malloc_(size_t size, const char *func, int line);
#define x_malloc(a) x_malloc_(a,__func__,__LINE__)

void *x_calloc_(size_t nmemb, size_t size, const char *func, int line);
#define x_calloc(a,b) x_calloc_(a,b,__func__,__LINE__)

#endif /* OTF_MEM_H */
