#ifndef TDS_HASH_H
#define TDS_HASH_H

#include "tds_elem.h"

/* note: elem *contents* (key or val) given to hash belongs to hash */

/* hash */
typedef struct tds_hash_ tds_hash;
tds_hash* tds_hash_new(void);
unsigned long tds_hash_size(tds_hash *hash);
int tds_hash_insert(tds_hash *hash, tds_elem *key, tds_elem *val);
int tds_hash_search(tds_hash *hash, tds_elem *key, tds_elem *val);
int tds_hash_remove(tds_hash *hash, tds_elem *key);
void tds_hash_retain(tds_hash *hash);
void tds_hash_free(tds_hash* hash);

/* iterator */
typedef struct tds_hash_iterator_  tds_hash_iterator;
tds_hash_iterator* tds_hash_iterator_new(tds_hash* hash);
int tds_hash_iterator_next(tds_hash_iterator* iterator, tds_elem *key, tds_elem *val);
void tds_hash_iterator_free(tds_hash_iterator* iterator);

#endif
