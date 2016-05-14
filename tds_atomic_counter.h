#ifndef TDS_ATOMIC_H
#define TDS_ATOMIC_H

char tds_has_atomic(void);

#if HAS_TORCH
typedef struct tds_atomic_counter_ tds_atomic_counter;
tds_atomic_counter* tds_atomic_new(void);
long tds_atomic_inc(tds_atomic_counter *atomic);
long tds_atomic_get(tds_atomic_counter *atomic);
void tds_atomic_set(tds_atomic_counter *atomic, long value);
void tds_atomic_retain(tds_atomic_counter *atomic);
void tds_atomic_free(tds_atomic_counter *atomic);
#endif

#endif