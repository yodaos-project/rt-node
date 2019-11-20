#include "js.h"
#include "freertos/FreeRTOS.h"

#define MEM_FLAGS MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT

void* espidf_malloc(size_t size) {
  return heap_caps_malloc(size, MEM_FLAGS);
}

void* espidf_realloc(void *ptr, size_t size) {
  return heap_caps_realloc(ptr, size, MEM_FLAGS);
}

void* espidf_calloc(size_t count, size_t size) {
  return heap_caps_calloc(count, size, MEM_FLAGS);
}

int app_main(int argc ,char **argv) {
  js_allocator_t allocator = {
    .malloc_fn = espidf_malloc,
    .realloc_fn = espidf_realloc,
    .calloc_fn = espidf_calloc,
    .free_fn = free
  };
  js_set_allocator(&allocator);
  js_start();
  return 0;
}