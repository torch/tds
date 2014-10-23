#include <string.h>
#include "tommy.h"
#include "tds_utils.h"
#include "tds_elem.h"

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

tds_elem_pointer_free_ptrfunc tds_elem_get_pointer_free(tds_elem *elem)
{
  return elem->value.ptr.free;
}

char tds_elem_type(tds_elem *elem)
{
  return elem->type;
}

void tds_elem_free_content(tds_elem *elem)
{
  if(elem->type == 's')
    tds_free(elem->value.str.data);
  if(elem->type == 'p' && elem->value.ptr.free)
    elem->value.ptr.free(elem->value.ptr.data);
  elem->type = 0;
}
