#include "tds_atomic_counter.h"
#include "tds_utils.h"

char tds_has_atomic(void) {
#if HAS_TORCH
  return 1;
#else
  return 0;
#endif
}

#if HAS_TORCH

#include "THAtomic.h"

struct tds_atomic_counter_ {
  long count;
  int refcount;
};

tds_atomic_counter* tds_atomic_new(void) {
  tds_atomic_counter *atomic = tds_malloc(sizeof(tds_atomic_counter));
  if (!atomic)
    return NULL;
  atomic->count = 0;
  atomic->refcount = 1;
  return atomic;
}

long tds_atomic_inc(tds_atomic_counter *atomic) {
  return THAtomicAddLong(&atomic->count, 1);
}

long tds_atomic_get(tds_atomic_counter *atomic) {
  return THAtomicGetLong(&atomic->count);
}

void tds_atomic_set(tds_atomic_counter *atomic, long value) {
  THAtomicSetLong(&atomic->count, value);
}

void tds_atomic_retain(tds_atomic_counter *atomic) {
  THAtomicIncrementRef(&atomic->refcount);
}

void tds_atomic_free(tds_atomic_counter *atomic) {
  if(THAtomicDecrementRef(&atomic->refcount))
  {
    tds_free(atomic);
  }
}

#endif