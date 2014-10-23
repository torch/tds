#ifndef TDS_UTILS_H
#define TDS_UTILS_H

#include <stdlib.h>

/* malloc and free */

/* better to replace malloc by something else dude */
/* we do a lot of predictible small allocation, that is bad */
void* tds_malloc(size_t size);
void tds_free(void *ptr);

#endif
