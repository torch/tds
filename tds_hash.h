#ifndef TDS_HASH_H
#define TDS_HASH_H

#include "tds_elem.h"

/* hash object */
typedef struct tds_hash_object_ tds_hash_object;

tds_hash_object *tds_hash_object_new(void);
tds_elem* tds_hash_object_key(tds_hash_object *obj);
tds_elem* tds_hash_object_value(tds_hash_object *obj);
void tds_hash_object_free(tds_hash_object *obj);

/* hash */
typedef struct tds_hash_ tds_hash;
tds_hash* tds_hash_new();
void tds_hash_insert(tds_hash *hash, tds_hash_object *obj);
tds_hash_object* tds_hash_search_string(tds_hash *hash, const char *str, long size);
tds_hash_object* tds_hash_remove_string(tds_hash *hash, const char *str, long size);
tds_hash_object* tds_hash_search_number(tds_hash *hash, double number);
tds_hash_object* tds_hash_remove_number(tds_hash *hash, double number);
tds_hash_object* tds_hash_search_pointer(tds_hash *hash, void* ptr);
tds_hash_object* tds_hash_remove_pointer(tds_hash *hash, void* ptr);
unsigned long tds_hash_size(tds_hash *hash);
void tds_hash_remove(tds_hash *hash, tds_hash_object *obj);
void tds_hash_retain(tds_hash *hash);
void tds_hash_free(tds_hash* hash);

/* iterator */
typedef struct tds_hash_iterator_  tds_hash_iterator;
tds_hash_iterator* tds_hash_iterator_new(tds_hash* hash);
tds_hash_object* tds_hash_iterator_next(tds_hash_iterator* iterator);
void tds_hash_iterator_free(tds_hash_iterator* iterator);

#endif
