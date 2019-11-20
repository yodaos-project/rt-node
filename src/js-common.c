#include "js-common.h"

js_context_t *js_ctx = NULL;

js_context_t *js_get_context() {
  if (!js_ctx) {
    js_ctx = (js_context_t *) js_malloc(sizeof(js_context_t));
  }
  return js_ctx;
}

typedef struct {
  size_t size;
} extra_memory_t;

static uint64_t mem_total = 0;
static uint64_t alloc_count = 0;
static js_allocator_t allocator = {
  .malloc_fn = malloc,
  .realloc_fn = realloc,
  .calloc_fn = calloc,
  .free_fn = free
};

void js_set_allocator(js_allocator_t *a) {
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

void js_free(void *ptr) {
  ptr = uninit_mem(ptr);
  free(ptr);
}

void* js_malloc(size_t size) {
  void *ptr = allocator.malloc_fn(size + sizeof(extra_memory_t));
  JS_ASSERT(ptr != NULL);
  return init_mem(ptr, size);
}

void* js_realloc(void *ptr, size_t size) {
  void *old_ptr = ptr;
  ptr = allocator.realloc_fn(ptr, size + sizeof(extra_memory_t));
  // ignore size == 0
  JS_ASSERT(ptr != NULL);
  if (ptr != old_ptr && old_ptr != NULL) {
    uninit_mem(old_ptr);
  }
  return init_mem(ptr, size);
}

void* js_calloc(size_t count, size_t size) {
  void *ptr = allocator.calloc_fn(count, size + sizeof(extra_memory_t));
  JS_ASSERT(ptr != NULL);
  return init_mem(ptr, size * count);
}

void* js_jerry_alloc(size_t size, void *cb_data) {
  void *ptr = js_malloc(size);
  JS_ASSERT(ptr != NULL);
  return ptr;
}

uint64_t js_get_memory_total() {
  return mem_total;
}

uint64_t js_get_memory_alloc_count() {
  return alloc_count;
}
