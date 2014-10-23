#include "tds_utils.h"

void* tds_malloc(size_t size)
{
  return malloc(size);
}

void tds_free(void *ptr)
{
  free(ptr);
}
