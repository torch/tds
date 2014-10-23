#ifndef TDS_ELEM_H
#define TDS_ELEM_H

#include <stdint.h>

typedef void (*tds_elem_pointer_free_ptrfunc)(void*);

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

uint32_t tds_elem_hashkey(tds_elem *elem);
void tds_elem_set_number(tds_elem *elem, double num);
void tds_elem_set_string(tds_elem *elem, const char *str, size_t size);
void tds_elem_set_pointer(tds_elem *elem, void *ptr, void (*free)(void*));
double tds_elem_get_number(tds_elem *elem);
const char* tds_elem_get_string(tds_elem *elem);
size_t tds_elem_get_string_size(tds_elem *elem);
void* tds_elem_get_pointer(tds_elem *elem);
tds_elem_pointer_free_ptrfunc tds_elem_get_pointer_free(tds_elem *elem);
char tds_elem_type(tds_elem *elem);
void tds_elem_free_content(tds_elem *elem);

#endif
