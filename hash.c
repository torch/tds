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
typedef struct tds_elem_ {
  union {
    double num;

    struct {
      char *data;
      size_t size;
    } str;

    struct {
      void *data;
      void (*free)(void*);
    } ptr;

  } value;

  char type;

} tds_elem;


uint32_t tds_elem_hashkey(tds_elem *elem)
{
  switch(elem->type) {
    case 'n':
      return tommy_hash_u32(0, &elem->value.num, sizeof(double));
    case 's':
      return tommy_hash_u32(0, elem->value.str.data, elem->value.str.size);
    case 'p':
      return tommy_hash_u32(0, elem->value.ptr.data, sizeof(void*));
  }
  return 0;
}

void tds_elem_set_number(tds_elem *elem, double num)
{
  elem->type = 'n';
  elem->value.num = num;
}

void tds_elem_set_string(tds_elem *elem, const char *str, size_t size)
{
  elem->type = 's';
  elem->value.str.data = tds_malloc(size);
  if(elem->value.str.data) {
    memcpy(elem->value.str.data, str, size);
    elem->value.str.size = size;
  }
}

void tds_elem_set_pointer(tds_elem *elem, void *ptr, void (*free)(void*))
{
  elem->type = 'p';
  elem->value.ptr.data = ptr;
  elem->value.ptr.free = free;
}

double tds_elem_get_number(tds_elem *elem)
{
  return elem->value.num;
}

const char* tds_elem_get_string(tds_elem *elem)
{
  return elem->value.str.data;
}

size_t tds_elem_get_string_size(tds_elem *elem)
{
  return elem->value.str.size;
}

void* tds_elem_get_pointer(tds_elem *elem)
{
  return elem->value.ptr.data;
}

char tds_elem_type(tds_elem *elem)
{
  return elem->type;
}

void tds_elem_free(tds_elem *elem)
{
  if(elem->type == 's')
    tds_free(elem->value.str.data);
  elem->type = 0;
}

/* hash object */
typedef struct tds_hash_object_ {
  tommy_node hash_node;

  tds_elem key;
  tds_elem val;

} tds_hash_object;

tds_hash_object *tds_hash_object_new(void)
{
  tds_hash_object *obj = tds_malloc(sizeof(tds_hash_object));
  obj->key.type = 0;
  obj->val.type = 0;
  return obj;
}

tds_elem* tds_hash_object_key(tds_hash_object *obj)
{
  return &obj->key;
}

tds_elem* tds_hash_object_value(tds_hash_object *obj)
{
  return &obj->val;
}

void tds_hash_object_free(tds_hash_object *obj)
{
  tds_elem_free(&obj->key);
  tds_elem_free(&obj->val);
  tds_free(obj);
}

/* hash structure */
/* i keep it like that for now,
   thinking about optimizing allocations
   of elements/nodes */
typedef struct tds_hash_ {
  tommy_hashlin *hash;
} tds_hash;


tds_hash* tds_hash_new()
{
  tds_hash *hash = tds_malloc(sizeof(tds_hash));
  if(hash) {
    hash->hash = tds_malloc(sizeof(tommy_hashlin));
    if(!hash->hash) {
      free(hash);
      return NULL;
    }
    tommy_hashlin_init(hash->hash);
  }
  return hash;
}

void tds_hash_insert(tds_hash *hash, tds_hash_object *obj)
{
  tommy_hashlin_insert(hash->hash, &obj->hash_node, obj, tds_elem_hashkey(&obj->key));
}

static int tds_hash_search_string_callback(const void *arg_, const void *obj_)
{
  const tds_elem *astr = arg_; /* must be a string */
  tds_hash_object *obj = (tds_hash_object*)obj_;
  long size = astr->value.str.size;

  if(obj->key.type != 's')
    return 1;

  if(size != obj->key.value.str.size)
    return 1;

  return memcmp(astr->value.str.data, obj->key.value.str.data, size);
}

tds_hash_object* tds_hash_search_string(tds_hash *hash, const char *str, long size)
{
  tds_elem tstr;
  tds_elem_set_string(&tstr, str, size);
  return tommy_hashlin_search(hash->hash, tds_hash_search_string_callback, &tstr, tommy_hash_u32(0, str, size));
}

tds_hash_object* tds_hash_remove_string(tds_hash *hash, const char *str, long size)
{
  tds_elem tstr;
  tds_elem_set_string(&tstr, str, size);
  return tommy_hashlin_remove(hash->hash, tds_hash_search_string_callback, &tstr, tommy_hash_u32(0, str, size));
}

static int tds_hash_search_number_callback(const void *arg_, const void *obj_)
{
  double anumber = *((double*)arg_);
  tds_hash_object *obj = (tds_hash_object*)obj_;

  if(obj->key.type != 'n')
    return 1;

  return anumber != obj->key.value.num;
}

tds_hash_object* tds_hash_search_number(tds_hash *hash, double number)
{
  return tommy_hashlin_search(hash->hash, tds_hash_search_number_callback, &number, tommy_hash_u32(0, &number, sizeof(double)));
}

tds_hash_object* tds_hash_remove_number(tds_hash *hash, double number)
{
  return tommy_hashlin_remove(hash->hash, tds_hash_search_number_callback, &number, tommy_hash_u32(0, &number, sizeof(double)));
}

static int tds_hash_search_pointer_callback(const void *arg, const void *obj_)
{
  tds_hash_object *obj = (tds_hash_object*)obj_;

  if(obj->key.type != 'p')
    return 1;

  return arg != obj->key.value.ptr.data;
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
  tds_free(hash->hash);
  tds_free(hash);
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
