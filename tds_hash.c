#include <string.h>

#include "klib/khash.h"
#include "tds_utils.h"
#include "tds_elem.h"
#include "tds_hash.h"

#if HAS_TORCH
#include "THAtomic.h"
#endif

static uint32_t tds_elem_hashkey_(tds_elem elem)
{
  return tds_elem_hashkey(&elem);
}

static int tds_elem_isequal_(tds_elem elem1, tds_elem elem2)
{
  return tds_elem_isequal(&elem1, &elem2);
}

KHASH_INIT(tds_elem, tds_elem, tds_elem, 1, tds_elem_hashkey_, tds_elem_isequal_);

/* hash structure */
struct tds_hash_ {
  khash_t(tds_elem) *hash;
  int refcount;
};


tds_hash* tds_hash_new(void)
{
  tds_hash *hash = tds_malloc(sizeof(tds_hash));
  if(hash) {
    hash->hash = kh_init(tds_elem);
    if(!hash->hash) {
      free(hash);
      return NULL;
    }
    hash->refcount = 1;
  }
  return hash;
}

int tds_hash_insert(tds_hash *hash, tds_elem *key, tds_elem *val)
{
  if(!val) {
    tds_hash_remove(hash, key);
    return 0;
  }
  else {
    int ret;
    khiter_t k = kh_put(tds_elem, hash->hash, *key, &ret);
    if(ret == -1) {
      return 1; /* out of memory */
    }
    else if(ret == 0) { /* key present */
      tds_elem_free_content(key); /* as it has not been used */
      tds_elem_free_content(&kh_val(hash->hash, k)); /* as it will be overwritten */
    }
    kh_val(hash->hash, k) = *val;
  }
  return 0;
}

int tds_hash_search(tds_hash *hash, tds_elem *key, tds_elem *val)
{
  khiter_t k = kh_get(tds_elem, hash->hash, *key);
  tds_elem_free_content(key); /* your memory belongs to us */
  if(k == kh_end(hash->hash))
    return 1;
  *val = kh_val(hash->hash, k);
  return 0;
}

int tds_hash_remove(tds_hash *hash, tds_elem *key_)
{
  tds_elem *key = NULL;
  tds_elem *val = NULL;
  khiter_t k = kh_get(tds_elem, hash->hash, *key_);
  tds_elem_free_content(key_); /* your memory belongs to us */
  if(k == kh_end(hash->hash))
    return 1;
  key = &kh_key(hash->hash, k);
  val = &kh_val(hash->hash, k);
  tds_elem_free_content(key); /* your memory belongs to us */
  tds_elem_free_content(val); /* your memory belongs to us */
  kh_del(tds_elem, hash->hash, k);
  return 0;
}

unsigned long tds_hash_size(tds_hash *hash)
{
  return kh_size(hash->hash);
}

void tds_hash_retain(tds_hash *hash)
{
#if HAS_TORCH
  THAtomicIncrementRef(&hash->refcount);
#else
  hash->refcount++;
#endif
}

void tds_hash_free(tds_hash* hash)
{
#if HAS_TORCH
  if(THAtomicDecrementRef(&hash->refcount))
#else
  hash->refcount--;
  if(hash->refcount == 0)
#endif
  {
    khint_t k;
    for(k = kh_begin(hash->hash); k != kh_end(hash->hash); ++k) {
      if (kh_exist(hash->hash, k)) {
        tds_elem_free_content(&kh_key(hash->hash, k)); /* your memory belongs to us */
        tds_elem_free_content(&kh_val(hash->hash, k)); /* your memory belongs to us */
      }
    }
    kh_destroy(tds_elem, hash->hash);
    tds_free(hash);
  }
}

/* iterator */
struct tds_hash_iterator_ {
  tds_hash *hash;
  khint_t k;
};

tds_hash_iterator* tds_hash_iterator_new(tds_hash* hash)
{
  /* workaround possible hash expand when calling
     kh_put on an existing key while iterating
     (see kh_put)
  */
  {
    khash_t(tds_elem) *h = hash->hash;
    if(h->n_occupied >= h->upper_bound) { /* update the hash table */
      if (h->n_buckets > (h->size<<1)) {
        if (kh_resize_tds_elem(h, h->n_buckets - 1) < 0) { /* clear "deleted" elements */
          return NULL;
        }
      } else if (kh_resize_tds_elem(h, h->n_buckets + 1) < 0) { /* expand the hash table */
        return NULL;
      }
    }
  }
  tds_hash_iterator *iterator = tds_malloc(sizeof(tds_hash_iterator));
  if(!iterator)
    return NULL;
  iterator->k = kh_begin(hash->hash)-1;
  iterator->hash = hash;
  tds_hash_retain(hash);
  return iterator;
}

int tds_hash_iterator_next(tds_hash_iterator* iterator, tds_elem *key, tds_elem *val)
{
  tds_hash *hash = iterator->hash;

  if(iterator->k == kh_end(hash->hash))
    return 1;

  do {
    iterator->k++;
  } while((iterator->k != kh_end(hash->hash)) && !kh_exist(hash->hash, iterator->k));

  if(iterator->k == kh_end(hash->hash))
    return 1;

  *key = kh_key(hash->hash, iterator->k);
  *val = kh_val(hash->hash, iterator->k);
  return 0;
}

void tds_hash_iterator_free(tds_hash_iterator* iterator)
{
  tds_hash_free(iterator->hash);
  tds_free(iterator);
}
