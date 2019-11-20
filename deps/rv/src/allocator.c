#include "rv.h"

static void* (*malloc_fn)(size_t) = malloc;
static void (*free_fn)(void *) = free;

int rv_set_allocator(void* (*m_fn)(size_t), void (*f_fn)(void *)) {
  malloc_fn = m_fn;
  free_fn = f_fn;
  return 0;
}

void* rv_malloc(size_t size) {
  return malloc_fn(size);
}

void rv_free(void *ptr) {
  free_fn(ptr);
}
