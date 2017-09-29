#ifndef TDS_VEC_H
#define TDS_VEC_H

#include "tds_elem.h"

typedef struct tds_vec_ tds_vec;

tds_vec* tds_vec_new(void);
size_t tds_vec_size(tds_vec *vec);
int tds_vec_insert(tds_vec *vec, size_t idx, tds_elem *val);
int tds_vec_set(tds_vec *vec, size_t idx, tds_elem *val);
int tds_vec_get(tds_vec *vec, size_t idx, tds_elem *val);
int tds_vec_remove(tds_vec *vec, size_t idx);
int tds_vec_resize(tds_vec *vec, size_t size);
void tds_vec_sort(tds_vec *vec, int (*compare)(tds_elem *elem1, tds_elem *elem2));
void tds_vec_retain(tds_vec *vec);
void tds_vec_free(tds_vec* vec);

#endif
