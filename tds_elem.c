#include <string.h>
#include <stdio.h>

#include "tds_utils.h"
#include "tds_elem.h"

/*
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the `Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 */
static uint32_t fnv32_buf(void *buf, size_t len, uint32_t hval)
{
  unsigned char *bp = (unsigned char *)buf;/* start of buffer */
  unsigned char *be = bp + len;/* beyond end of buffer */

  /*
   * FNV-1 hash each octet in the buffer
   */
  while (bp < be) {

    /* multiply by the 32 bit FNV magic prime mod 2^32 */
    hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);

    /* xor the bottom with the current octet */
    hval ^= (uint32_t)*bp++;
  }

  /* return our new hash value */
  return hval;
}


tds_elem *tds_elem_new(void)
{
  return tds_malloc(sizeof(tds_elem));
}

void tds_elem_free(tds_elem *elem)
{
  tds_free(elem);
}

uint32_t tds_elem_hashkey(tds_elem *elem)
{
  switch(elem->type) {
    case 'n':
      return fnv32_buf(&elem->value.num, sizeof(double), 0);
    case 'b':
      return fnv32_buf(&elem->value.flag, sizeof(bool), 0);
    case 's':
      return fnv32_buf(elem->value.str.data, elem->value.str.size, 0);
    case 'p':
      return fnv32_buf(elem->value.ptr.data, sizeof(void*), 0);
  }
  return 0;
}

int tds_elem_isequal(tds_elem *elem1, tds_elem *elem2)
{
  if(elem1->type != elem2->type || elem1->subtype != elem1->subtype)
    return 0;
  switch(elem1->type) {
    case 'n':
      return elem1->value.num == elem2->value.num;
    case 'b':
      return elem1->value.flag == elem2->value.flag;
    case 's':
      if(elem1->value.str.size != elem2->value.str.size)
        return 0;
      return !memcmp(elem1->value.str.data, elem2->value.str.data, elem1->value.str.size);
    case 'p':
      return elem1->value.ptr.data == elem2->value.ptr.data;
    default:
      printf("[tds hash] internal error: unknown type\n");
      return 0;
  }
}

void tds_elem_set_subtype(tds_elem *elem, char subtype)
{
  elem->subtype = subtype;
}

void tds_elem_set_number(tds_elem *elem, double num)
{
  elem->type = 'n';
  elem->subtype = 0;
  elem->value.num = num;
}

void tds_elem_set_boolean(tds_elem *elem, bool flag)
{
  elem->type = 'b';
  elem->subtype = 0;
  elem->value.flag = flag;
}

void tds_elem_set_string(tds_elem *elem, const char *str, size_t size)
{
  elem->type = 's';
  elem->subtype = 0;
  elem->value.str.data = tds_malloc(size);
  if(elem->value.str.data) {
    memcpy(elem->value.str.data, str, size);
    elem->value.str.size = size;
  }
}

void tds_elem_set_pointer(tds_elem *elem, void *ptr, void (*free)(void*))
{
  elem->type = 'p';
  elem->subtype = 0;
  elem->value.ptr.data = ptr;
  elem->value.ptr.free = free;
}

double tds_elem_get_number(tds_elem *elem)
{
  return elem->value.num;
}

bool tds_elem_get_boolean(tds_elem *elem)
{
  return elem->value.flag;
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

char tds_elem_subtype(tds_elem *elem)
{
  return elem->subtype;
}

void tds_elem_free_content(tds_elem *elem)
{
  if(elem->type == 's')
    tds_free(elem->value.str.data);
  if(elem->type == 'p' && elem->value.ptr.free)
    elem->value.ptr.free(elem->value.ptr.data);
  elem->type = 0;
  elem->subtype = 0;
}

void tds_elem_set_nil(tds_elem *elem)
{
  elem->type = 0;
  elem->subtype = 0;
}

int tds_elem_isnil(tds_elem *elem)
{
  return (elem->type == 0);
}
