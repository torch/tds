local ffi = require 'ffi'

ffi.cdef[[

/* elem */
typedef void (*tds_elem_pointer_free_ptrfunc)(void*);
typedef struct tds_elem_ tds_elem;
tds_elem *tds_elem_new(void);
void tds_elem_free(tds_elem *elem);
uint32_t tds_elem_hashkey(tds_elem *elem);
int tds_elem_isequal(tds_elem *elem1, tds_elem *elem2);
void tds_elem_set_subtype(tds_elem *elem, char subtype);
void tds_elem_set_number(tds_elem *elem, double num);
void tds_elem_set_boolean(tds_elem *elem, bool flag);
void tds_elem_set_string(tds_elem *elem, const char *str, size_t size);
void tds_elem_set_pointer(tds_elem *elem, void *ptr, void (*free)(void*));
double tds_elem_get_number(tds_elem *elem);
bool tds_elem_get_boolean(tds_elem *elem);
const char* tds_elem_get_string(tds_elem *elem);
size_t tds_elem_get_string_size(tds_elem *elem);
void* tds_elem_get_pointer(tds_elem *elem);
tds_elem_pointer_free_ptrfunc tds_elem_get_pointer_free(tds_elem *elem);
char tds_elem_type(tds_elem *elem);
char tds_elem_subtype(tds_elem *elem);
void tds_elem_free_content(tds_elem *elem);
void tds_elem_set_nil(tds_elem *elem);
int tds_elem_isnil(tds_elem *elem);

/* hash */
typedef struct tds_hash_ tds_hash;
tds_hash* tds_hash_new(void);
unsigned long tds_hash_size(tds_hash *hash);
int tds_hash_insert(tds_hash *hash, tds_elem *key, tds_elem *val);
int tds_hash_search(tds_hash *hash, tds_elem *key, tds_elem *val);
int tds_hash_remove(tds_hash *hash, tds_elem *key);
void tds_hash_retain(tds_hash *hash);
void tds_hash_free(tds_hash* hash);

/* hash iterator */
typedef struct tds_hash_iterator_  tds_hash_iterator;
tds_hash_iterator* tds_hash_iterator_new(tds_hash* hash);
int tds_hash_iterator_next(tds_hash_iterator* iterator, tds_elem *key, tds_elem *val);
void tds_hash_iterator_free(tds_hash_iterator* iterator);

/* vec */
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

/* atomic counter */
char tds_has_atomic(void);
typedef struct tds_atomic_counter_ tds_atomic_counter;
tds_atomic_counter* tds_atomic_new(void);
long tds_atomic_inc(tds_atomic_counter *atomic);
long tds_atomic_get(tds_atomic_counter *atomic);
void tds_atomic_set(tds_atomic_counter *atomic, long value);
void tds_atomic_retain(tds_atomic_counter *atomic);
void tds_atomic_free(tds_atomic_counter *atomic);

]]
