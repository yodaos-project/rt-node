#include "rt-node.h"

rtnode_context_t *rtnode_ctx = NULL;

rtnode_context_t *rtnode_get_context() {
  if (!rtnode_ctx) {
    rtnode_ctx = (rtnode_context_t *) rtnode_malloc(sizeof(rtnode_context_t));
  }
  return rtnode_ctx;
}

typedef struct {
  size_t size;
} extra_memory_t;

static uint64_t mem_total = 0;
static uint64_t alloc_count = 0;
static rtnode_allocator_t allocator = {
  .malloc_fn = malloc,
  .realloc_fn = realloc,
  .calloc_fn = calloc,
  .free_fn = free
};

void rtnode_set_allocator(rtnode_allocator_t *a) {
  allocator = *a;
}

static void* init_mem(void *ptr, size_t size) {
  mem_total += size;
  ++alloc_count;
  extra_memory_t *ext = (extra_memory_t *) ptr;
  ext->size = size;
  ptr = (char *) ptr + sizeof(extra_memory_t);
  return ptr;
}

static void* uninit_mem(void *ptr) {
  if (ptr == NULL) {
    return ptr;
  }
  void *p = (char *) ptr - sizeof(extra_memory_t);
  extra_memory_t *ext = (extra_memory_t *) p;
  mem_total -= ext->size;
  --alloc_count;
  return p;
}

void rtnode_free(void *ptr) {
  ptr = uninit_mem(ptr);
  free(ptr);
}

void* rtnode_malloc(size_t size) {
  void *ptr = allocator.malloc_fn(size + sizeof(extra_memory_t));
  RTNODE_ASSERT(ptr != NULL);
  return init_mem(ptr, size);
}

void* rtnode_realloc(void *ptr, size_t size) {
  void *old_ptr = ptr;
  ptr = allocator.realloc_fn(ptr, size + sizeof(extra_memory_t));
  // ignore size == 0
  RTNODE_ASSERT(ptr != NULL);
  if (ptr != old_ptr && old_ptr != NULL) {
    uninit_mem(old_ptr);
  }
  return init_mem(ptr, size);
}

void* rtnode_calloc(size_t count, size_t size) {
  void *ptr = allocator.calloc_fn(count, size + sizeof(extra_memory_t));
  RTNODE_ASSERT(ptr != NULL);
  return init_mem(ptr, size * count);
}

void* rtnode_jerry_alloc(size_t size, void *cb_data) {
  void *ptr = rtnode_malloc(size);
  RTNODE_ASSERT(ptr != NULL);
  return ptr;
}

uint64_t rtnode_get_memory_total() {
  return mem_total;
}

uint64_t rtnode_get_memory_alloc_count() {
  return alloc_count;
}
