#include <string.h> /* for memmove */
#include "tds_utils.h"
#include "tds_vec.h"

#if HAS_TORCH
#include "THAtomic.h"
#endif

/* note: size_t >= 0 */

struct tds_vec_ {
  tds_elem *data;
  size_t n; /* number of elements */
  size_t allocn; /* number of allocated elements */
  int refcount;
};

tds_vec* tds_vec_new(void)
{
  tds_vec *vec = tds_malloc(sizeof(tds_vec));
  if(!vec)
    return NULL;
  vec->data = NULL;
  vec->allocn = 0;
  vec->n = 0;
  vec->refcount = 1;
  return vec;
}

size_t tds_vec_size(tds_vec *vec)
{
  return vec->n;
}

int tds_vec_insert(tds_vec *vec, size_t idx, tds_elem *val)
{
  if(tds_vec_resize(vec, vec->n+1))
      return 1;
  if(vec->n-idx-1 > 0)
    memmove(vec->data+idx+1, vec->data+idx, sizeof(tds_elem)*(vec->n-idx-1));
  vec->data[idx] = *val;
  return 0;
}

int tds_vec_set(tds_vec *vec, size_t idx, tds_elem *val)
{
  if(idx >= vec->n) {
    if(tds_vec_resize(vec, idx+1))
      return 1;
  }
  tds_elem_free_content(&vec->data[idx]);
  vec->data[idx] = *val;
  return 0;
}

int tds_vec_get(tds_vec *vec, size_t idx, tds_elem *val)
{
  if(idx >= vec->n) {
    tds_elem_set_nil(val);
    return 0;
  }
  *val = vec->data[idx];
  return 0;
}

int tds_vec_remove(tds_vec *vec, size_t idx)
{
  if(idx >= vec->n)
    return 0;
  tds_elem_free_content(&vec->data[idx]);
  if(vec->n-idx-1 > 0)
    memmove(vec->data+idx, vec->data+idx+1, sizeof(tds_elem)*(vec->n-idx-1));
  vec->n--;
  return 0;
}

int tds_vec_resize(tds_vec *vec, size_t size)
{
  size_t k;
  if(size == vec->n) {
    return 0;
  } else if(size > vec->n) {
    if(size > vec->allocn) {
      /* always allocate at least 1.5x more at each time */
      size_t allocn = (vec->allocn*1.5 > size ? vec->allocn*1.5 : size);
      tds_elem *data = tds_realloc(vec->data, sizeof(tds_elem)*allocn);
      if(!data)
        return 1;
      vec->data = data;
      vec->allocn = allocn;
    }
    for(k = vec->n; k < size; k++)
      tds_elem_set_nil(&vec->data[k]);
    vec->n = size;
  } else {
    for(k = size; k < vec->n; k++)
      tds_elem_free_content(&vec->data[k]);
    vec->n = size;
  }
  return 0;
}

void tds_vec_sort(tds_vec *vec, int (*compare)(tds_elem *elem1, tds_elem *elem2))
{
  qsort(vec->data, vec->n, sizeof(tds_elem), (int (*)(const void*, const void*))(compare));
}

void tds_vec_retain(tds_vec *vec)
{
#if HAS_TORCH
  THAtomicIncrementRef(&vec->refcount);
#else
  vec->refcount++;
#endif
}

void tds_vec_free(tds_vec* vec)
{
  size_t k;
#if HAS_TORCH
  if(THAtomicDecrementRef(&vec->refcount))
#else
  vec->refcount--;
  if(vec->refcount == 0)
#endif
  {
    for(k = 0; k < vec->n; k++)
      tds_elem_free_content(&vec->data[k]);
    tds_free(vec->data);
    tds_free(vec);
  }
}
