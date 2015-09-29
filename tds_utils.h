#ifndef TDS_UTILS_H
#define TDS_UTILS_H

#include <stdlib.h>

/* malloc and free */

void* tds_malloc(size_t size);
void* tds_realloc(void *ptr, size_t size);
void tds_free(void *ptr);

#endif
