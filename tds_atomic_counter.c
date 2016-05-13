#include "tds_atomic_counter.h"
#include "tds_utils.h"

#if HAS_TORCH
#include "THAtomic.h"
#else
#include <pthread.h>
static pthread_mutex_t ptm = PTHREAD_MUTEX_INITIALIZER;
#endif

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
#if HAS_TORCH
  return THAtomicAddLong(&atomic->count, 1);
#else
  pthread_mutex_lock(&ptm);
  long lastValue = atomic->count;
  atomic->count++;
  pthread_mutex_unlock(&ptm);
  return lastValue;
#endif
}

long tds_atomic_get(tds_atomic_counter *atomic) {
#if HAS_TORCH
  return THAtomicGetLong(&atomic->count);
#else
  pthread_mutex_lock(&ptm);
  long value = atomic->count;
  pthread_mutex_unlock(&ptm);
  return value;
#endif
}

void tds_atomic_set(tds_atomic_counter *atomic, long value) {
#if HAS_TORCH
  THAtomicSetLong(&atomic->count, value);
#else
  pthread_mutex_lock(&ptm);
  atomic->count = value;
  pthread_mutex_unlock(&ptm);
#endif
}

void tds_atomic_retain(tds_atomic_counter *atomic) {
#if HAS_TORCH
  THAtomicIncrementRef(&atomic->refcount);
#else
  pthread_mutex_lock(&ptm);
  atomic->refcount++;
  pthread_mutex_unlock(&ptm);
#endif
}

void tds_atomic_free(tds_atomic_counter *atomic) {
#if HAS_TORCH
  if(THAtomicDecrementRef(&atomic->refcount))
  {
    tds_free(atomic);
  }
#else
  pthread_mutex_lock(&ptm);
  atomic->refcount--;
  if(atomic->refcount == 0)
  {
    tds_free(atomic);
  }
  pthread_mutex_unlock(&ptm);
#endif
}

