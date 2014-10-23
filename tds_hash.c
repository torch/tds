#include <stdio.h>
#include <string.h>
#include "tommy.h"
#include "tds_utils.h"
#include "tds_hash.h"

/* hash object */
struct tds_hash_object_ {
  tommy_node hash_node;

  tds_elem key;
  tds_elem val;

};

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
  tds_elem_free_content(&obj->key);
  tds_elem_free_content(&obj->val);
  tds_free(obj);
}

/* hash structure */
/* i keep it like that for now,
   thinking about optimizing allocations
   of elements/nodes */
struct tds_hash_ {
  tommy_hashlin *hash;
  long refcount;
};


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
    hash->refcount = 1;
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
  tstr.type = 's';
  tstr.value.str.data = (char*)str; /* i know what i am doing */
  tstr.value.str.size = size;
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

void tds_hash_retain(tds_hash *hash)
{
  hash->refcount++;
}

void tds_hash_free(tds_hash* hash)
{
  hash->refcount--;
  if(hash->refcount == 0)
  {
    tommy_hashlin_foreach(hash->hash, (void(*)(void*))tds_hash_object_free);
    tommy_hashlin_done(hash->hash);
    tds_free(hash->hash);
    tds_free(hash);
  }
  else if(hash->refcount < 0)
    printf("[tds hash] warning: refcount issue\n");
}

/* iterator */
struct tds_hash_iterator_node {
  tommy_node node;
  tds_hash_object *obj;
};

struct tds_hash_iterator_ {
  tds_hash *hash;
  tommy_arrayof *array;
  size_t index;
  size_t size;
};

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

  /* retain hash */
  iterator->hash = hash;
  tds_hash_retain(hash);

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
  tds_hash_free(iterator->hash);
  tds_free(iterator);
}
