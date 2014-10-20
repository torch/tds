#include <string.h>
#include "tommy.h"
#include <stdio.h>

/* malloc and free */

/* better to replace malloc by something else dude */
/* we do a lot of predictible small allocation, that is bad */
void* tds_malloc(size_t size)
{
  return malloc(size);
}

void tds_free(void *ptr)
{
  free(ptr);
}

/* basic elements contained in data structures */
struct tds_elem_;

struct tds_elem_vtable_ {
  void (*free)(struct tds_elem_*);
  uint32_t (*hashkey)(struct tds_elem_*);
  const char* typename;
};

typedef struct tds_elem_ {
  struct tds_elem_vtable_ *vtable;
} tds_elem;

void tds_elem_free(tds_elem *elem)
{
  elem->vtable->free(elem);
}

uint32_t tds_elem_hashkey(tds_elem *elem)
{
  return elem->vtable->hashkey(elem);
}

const char* tds_elem_typename(tds_elem *elem)
{
  return elem->vtable->typename;
}

/* string element */
typedef struct tds_string_ {
  tds_elem elem;
  char *data;
  long size;
} tds_string;

static void tds_elem_string_free(tds_elem *elem)
{
  tds_string *tstr = (tds_string*)elem;
  if(tstr->data)
    tds_free(tstr->data);
  tds_free(tstr);
}

static uint32_t tds_elem_string_hashkey(tds_elem *elem)
{
  tds_string *tstr = (tds_string*)elem;
  return tommy_hash_u32(0, tstr->data, tstr->size);
}

static const char tds_elem_string_typename[] = "string";

static struct tds_elem_vtable_ tds_elem_string_vtable = {
  tds_elem_string_free,
  tds_elem_string_hashkey,
  tds_elem_string_typename
};

tds_elem* tds_elem_string_new(const char* str, long size)
{
  tds_string *tstr = tds_malloc(sizeof(tds_string));
  if(tstr) {
    tstr->data = tds_malloc(size);
    memcpy(tstr->data, str, size);
    tstr->size = size;
    tstr->elem.vtable = &tds_elem_string_vtable;
  }
  return (tds_elem*)tstr;
}

/* number element */
typedef struct tds_number_ {
  tds_elem elem;
  double value;
} tds_number;

static void tds_elem_number_free(tds_elem *elem)
{
  tds_free(elem);
}

static uint32_t tds_elem_number_hashkey(tds_elem *elem)
{
  tds_number *tnum = (tds_number*)elem;
  return tommy_hash_u32(0, &tnum->value, sizeof(double));
}

static const char tds_elem_number_typename[] = "number";

static struct tds_elem_vtable_ tds_elem_number_vtable = {
  tds_elem_number_free,
  tds_elem_number_hashkey,
  tds_elem_number_typename
};

tds_elem* tds_elem_number_new(double value)
{
  tds_number *tnum = tds_malloc(sizeof(tds_number));
  if(tnum) {
    tnum->value = value;
    tnum->elem.vtable = &tds_elem_number_vtable;
  }
  return (tds_elem*)tnum;
}

/* pointer element */
typedef struct tds_pointer_ {
  tds_elem elem;
  void *addr;
  void (*free)(void*);
} tds_pointer;

static void tds_elem_pointer_free(tds_elem *elem)
{
  tds_pointer *tptr = (tds_pointer*)elem;
  if(tptr->free)
    tptr->free(tptr->addr);
  tds_free(tptr);
}

static uint32_t tds_elem_pointer_hashkey(tds_elem *elem)
{
  tds_pointer *tptr = (tds_pointer*)elem;
  return tommy_hash_u32(0, &tptr->addr, sizeof(void*));
}

static const char tds_elem_pointer_typename[] = "pointer";

static struct tds_elem_vtable_ tds_elem_pointer_vtable = {
  tds_elem_pointer_free,
  tds_elem_pointer_hashkey,
  tds_elem_pointer_typename
};

tds_elem* tds_elem_pointer_new(void *addr, void (*free)(void*))
{
  tds_pointer *tptr = tds_malloc(sizeof(tds_pointer));
  if(tptr) {
    tptr->addr = addr;
    tptr->free = free;
    tptr->elem.vtable = &tds_elem_pointer_vtable;
  }
  return (tds_elem*)tptr;
}

/* hash object */
typedef struct tds_hash_object_ {
//  tommy_node list_node;
  tommy_node hash_node;

  tds_elem *key;
  tds_elem *value;

} tds_hash_object;

tds_elem* tds_hash_object_key(tds_hash_object *obj)
{
  return obj->key;
}

tds_elem* tds_hash_object_value(tds_hash_object *obj)
{
  return obj->value;
}

void tds_hash_object_set_key(tds_hash_object *obj, tds_elem *key)
{
  if(obj->key)
    tds_elem_free(obj->key);
  obj->key = key;
}

void tds_hash_object_set_value(tds_hash_object *obj, tds_elem *value)
{
  if(obj->value)
    tds_elem_free(obj->value);
  obj->value = value;
}

tds_hash_object *tds_hash_object_new(tds_elem *key, tds_elem *value)
{
  tds_hash_object *obj = tds_malloc(sizeof(tds_hash_object));
  if(obj) {
    obj->key = key;
    obj->value = value;
  }
  return obj;
}

void tds_hash_object_free(tds_hash_object *obj)
{
  if(obj->key)
    tds_elem_free(obj->key);
  if(obj->value)
    tds_elem_free(obj->value);
  tds_free(obj);
}

/* hash structure */
typedef struct tds_hash_ {
  tommy_hashlin *hash;
//  tommy_list *list;
} tds_hash;


tds_hash* tds_hash_new()
{
  tds_hash *hash = tds_malloc(sizeof(tds_hash));
  hash->hash = tds_malloc(sizeof(tommy_hashlin));
//  hash->list = tds_malloc(sizeof(tommy_list));
  tommy_hashlin_init(hash->hash);
//  tommy_list_init(hash->list);
  return hash;
}

void tds_hash_insert(tds_hash *hash, tds_hash_object *obj)
{
  tommy_hashlin_insert(hash->hash, &obj->hash_node, obj, tds_elem_hashkey(obj->key));
//  tommy_list_insert_tail(hash->list, &obj->list_node, obj);
}

static int tds_hash_search_string_callback(const void *arg, const void *obj)
{
  const tds_string *astr = arg;
  const tds_string *ostr = (tds_string*)((tds_hash_object*)obj)->key;
  long size = astr->size;

  if(size != ostr->size)
    return 1;

  return memcmp(astr->data, ostr->data, size);
}

tds_hash_object* tds_hash_search_string(tds_hash *hash, const char *str, long size)
{
  tds_string tstr;
  tstr.data = (char*)str; /* we do not touch it, i promise */
  tstr.size = size;
  return tommy_hashlin_search(hash->hash, tds_hash_search_string_callback, &tstr, tommy_hash_u32(0, str, size));
}

tds_hash_object* tds_hash_remove_string(tds_hash *hash, const char *str, long size)
{
  return tommy_hashlin_remove(hash->hash, tds_hash_search_string_callback, str, tommy_hash_u32(0, str, size));
}

static int tds_hash_search_number_callback(const void *arg, const void *obj)
{
  double anumber = *((double*)arg);
  double onumber = ((tds_number*)(((tds_hash_object*)obj)->key))->value;
  return anumber != onumber;
}

tds_hash_object* tds_hash_search_number(tds_hash *hash, double number)
{
  return tommy_hashlin_search(hash->hash, tds_hash_search_number_callback, &number, tommy_hash_u32(0, &number, sizeof(double)));
}

tds_hash_object* tds_hash_remove_number(tds_hash *hash, double number)
{
  return tommy_hashlin_remove(hash->hash, tds_hash_search_number_callback, &number, tommy_hash_u32(0, &number, sizeof(double)));
}

static int tds_hash_search_pointer_callback(const void *arg, const void *obj)
{
  tds_pointer *tptr = (tds_pointer*)obj;
  return arg != tptr->addr;
}

tds_hash_object* tds_hash_search_pointer(tds_hash *hash, void* ptr)
{
  return tommy_hashlin_search(hash->hash, tds_hash_search_pointer_callback, ptr, tommy_hash_u32(0, &ptr, sizeof(void*)));
}

tds_hash_object* tds_hash_remove_pointer(tds_hash *hash, void* ptr)
{
  return tommy_hashlin_remove(hash->hash, tds_hash_search_pointer_callback, ptr, tommy_hash_u32(0, &ptr, sizeof(void*)));
}

unsigned long tds_hash_size(tds_hash *hash)
{
  return tommy_hashlin_count(hash->hash);
}

void tds_hash_remove(tds_hash *hash, tds_hash_object *obj)
{
  tommy_hashlin_remove_existing(hash->hash, &obj->hash_node);
}

void tds_hash_free(tds_hash* hash)
{
  tommy_hashlin_foreach(hash->hash, (void(*)(void*))tds_hash_object_free);
  tommy_hashlin_done(hash->hash);
}

/* iterator */
struct tds_hash_iterator_node {
  tommy_node node;
  tds_hash_object *obj;
};

typedef struct tds_hash_iterator_ {
  tommy_arrayof *array;
  size_t index;
  size_t size;
} tds_hash_iterator;

static void tds_add_hash_node_to_list(void *iterator_, void *obj_)
{
  tds_hash_iterator *iterator = (tds_hash_iterator*)iterator_;
  struct tds_hash_iterator_node *node = tommy_arrayof_ref(iterator->array, iterator->index);
  tds_hash_object *obj = (tds_hash_object*)obj_;
  node->obj = obj;
  iterator->index++;
}

tds_hash_iterator* tds_hash_iterator_new(tds_hash* hash)
{
  size_t size = tds_hash_size(hash);
  tommy_arrayof *array;
  tds_hash_iterator *iterator;

  /* init a big array */
  iterator = tds_malloc(sizeof(tds_hash_iterator));
  if(!iterator)
    return NULL;

  array = tds_malloc(sizeof(tommy_arrayof));
  if(!array) {
    tds_free(iterator);
    return NULL;
  }
  tommy_arrayof_init(array, sizeof(struct tds_hash_iterator_node));
  tommy_arrayof_grow(array, size);

  /* fill it up with hash nodes */
  iterator->array = array;
  iterator->index = 0;
  iterator->size = size;
  tommy_hashlin_foreach_arg(hash->hash, tds_add_hash_node_to_list, iterator);

  /* reset iterator */
  iterator->index = 0;

  return iterator;
}

tds_hash_object* tds_hash_iterator_next(tds_hash_iterator* iterator)
{
  if(iterator->index >= iterator->size)
    return NULL;
  else {
    iterator->index++;
    return ((struct tds_hash_iterator_node*)tommy_arrayof_ref(iterator->array, iterator->index-1))->obj;
  }
}

void tds_hash_iterator_free(tds_hash_iterator* iterator)
{
  tommy_arrayof_done(iterator->array);
  tds_free(iterator->array);
  tds_free(iterator);
}
