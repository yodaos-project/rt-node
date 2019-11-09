#include "rt-node.h"

#ifdef __FREERTOS__
#include "freertos/FreeRTOS.h"
#define MEM_FLAGS MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
#define FN_MALLOC(s) heap_caps_malloc(s + sizeof(extra_memory_t), MEM_FLAGS)
#define FN_REALLOC(p, s) heap_caps_realloc(p, s + sizeof(extra_memory_t), MEM_FLAGS)
#define FN_CALLOC(c, s) heap_caps_calloc(c, s + sizeof(extra_memory_t), MEM_FLAGS)
#else
#define FN_MALLOC(s) malloc(s + sizeof(extra_memory_t))
#define FN_REALLOC(p, s) realloc(p, s + sizeof(extra_memory_t))
#define FN_CALLOC(c, s) calloc(c, s + sizeof(extra_memory_t))
#endif

rtnode_context_t *js_ctx = NULL;

typedef struct {
  size_t size;
} extra_memory_t;

static uint64_t mem_total = 0;
static uint64_t alloc_count = 0;

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
  void *ptr = FN_MALLOC(size);
  RTNODE_ASSERT(ptr != NULL);
  return init_mem(ptr, size);
}

void* rtnode_realloc(void *ptr, size_t size) {
  void *old_ptr = ptr;
  ptr = FN_REALLOC(ptr, size);
  // ignore size == 0
  RTNODE_ASSERT(ptr != NULL);
  if (ptr != old_ptr && old_ptr != NULL) {
    uninit_mem(old_ptr);
  }
  return init_mem(ptr, size);
}

void* rtnode_calloc(size_t count, size_t size) {
  void *ptr = FN_CALLOC(count, size);
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
