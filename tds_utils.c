#include "tds_utils.h"

void* tds_malloc(size_t size)
{
  return malloc(size);
}

void* tds_realloc(void *ptr, size_t size)
{
  return realloc(ptr, size);
}

void tds_free(void *ptr)
{
  free(ptr);
}
