local ffi = require 'ffi'

ffi.cdef[[

typedef struct tds_elem_ tds_elem;

uint32_t tds_elem_hashkey(tds_elem *elem);
void tds_elem_set_number(tds_elem *elem, double num);
void tds_elem_set_string(tds_elem *elem, const char *str, size_t size);
void tds_elem_set_pointer(tds_elem *elem, void *ptr, void (*free)(void*));
double tds_elem_get_number(tds_elem *elem);
const char* tds_elem_get_string(tds_elem *elem);
size_t tds_elem_get_string_size(tds_elem *elem);
void* tds_elem_get_pointer(tds_elem *elem);
typedef void (*tds_elem_pointer_free_ptrfunc)(void*);
tds_elem_pointer_free_ptrfunc tds_elem_get_pointer_free(tds_elem *elem);
char tds_elem_type(tds_elem *elem);
void tds_elem_free_content(tds_elem *elem);

typedef struct tds_hash_object_ tds_hash_object;
tds_hash_object *tds_hash_object_new(void);
tds_elem* tds_hash_object_key(tds_hash_object *obj);
tds_elem* tds_hash_object_value(tds_hash_object *obj);
void tds_hash_object_free(tds_hash_object *obj);

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
void tds_hash_free(tds_hash* hash);

typedef struct tds_hash_iterator_  tds_hash_iterator;
tds_hash_iterator* tds_hash_iterator_new(tds_hash* hash);
tds_hash_object* tds_hash_iterator_next(tds_hash_iterator* iterator);
void tds_hash_iterator_free(tds_hash_iterator* iterator);

]]
