local ffi = require 'ffi'

ffi.cdef[[

struct tds_elem_;

struct tds_elem_vtable_ {
  void (*free)(struct tds_elem_*);
  uint32_t (*hashkey)(struct tds_elem_*);
  const char* typename;
};

typedef struct tds_elem_ {
  struct tds_elem_vtable_ *vtable;
} tds_elem;

void tds_elem_free(tds_elem *elem);
uint32_t tds_elem_hashkey(tds_elem *elem);
const char* tds_elem_typename(tds_elem *elem);

typedef struct tds_string_ {
  tds_elem elem;
  char *data;
  long size;
} tds_string;

tds_elem* tds_elem_string_new(const char* str, long size);

typedef struct tds_number_ {
  tds_elem elem;
  double value;
} tds_number;

tds_elem* tds_elem_number_new(double value);

typedef struct tds_pointer_ {
  tds_elem elem;
  void *addr;
  void (*free)(void*);
} tds_pointer;

tds_elem* tds_elem_pointer_new(void *addr, void (*free)(void*));

typedef struct tds_hash_object_ tds_hash_object;
tds_hash_object *tds_hash_object_new(tds_elem *key, tds_elem *value);
tds_elem* tds_hash_object_key(tds_hash_object *obj);
tds_elem* tds_hash_object_value(tds_hash_object *obj);
void tds_hash_object_set_key(tds_hash_object *obj, tds_elem *key);
void tds_hash_object_set_value(tds_hash_object *obj, tds_elem *value);
void tds_hash_object_free(tds_hash_object *obj);

typedef struct tds_hash_  tds_hash;
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
void tds_hash_free(tds_hash* hash);

typedef struct tds_hash_iterator_  tds_hash_iterator;
tds_hash_iterator* tds_hash_iterator_new(tds_hash* hash);
tds_hash_object* tds_hash_iterator_next(tds_hash_iterator* iterator);
void tds_hash_iterator_free(tds_hash_iterator* iterator);

]]
