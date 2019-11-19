#ifndef _RTNODE_H_
#define _RTNODE_H_

#include "rtev.h"
#include "rtnode-binding.h"
#include "rtnode-error.h"
#include "rtnode-logger.h"

#define NODE_MAJOR_VERSION 0
#define NODE_MINOR_VERSION 0
#define NODE_PATCH_VERSION 1

#if !defined(STRINGIFY)
#define STRINGIFY(x) #x
#endif /* STRINGIFY */

#if !defined(TOSTRING)
#define TOSTRING(x) STRINGIFY(x)
#endif /* TOSTRING */

#define RTNODE_ABORT() \
  do { RTNODE_LOG_E("abort message: %s:%d", __FILE__, __LINE__); abort(); } while (false)

#define RTNODE_ASSERT(exp) \
  do { \
    if(!(exp)) { \
      RTNODE_LOG_E("assert message: %s", #exp); \
      assert(0); \
    } \
  } while (false)

#define RTNODE_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(name)                  \
  static void rtnode_##name##_destroy(rtnode_##name##_t* wrap);              \
  static const jerry_object_native_info_t this_module_native_info = {      \
    .free_cb = (jerry_object_native_free_callback_t)rtnode_##name##_destroy \
  }

#define RTNODE_CHECK_FATAL_ERROR(jobj, source) \
  if (jerry_value_is_error(jobj)) { \
    rtnode_on_fatal_error(jobj, source); \
    assert(0); \
  }

#define RTNODE_CREATE_ERROR(TYPE, message) \
  jerry_create_error(JERRY_ERROR_##TYPE, (const jerry_char_t*)message)

#define RTNODE_DECLARE_PTR(JOBJ, TYPE, NAME)                              \
  TYPE* NAME = NULL;                                                  \
  do {                                                                \
    if (!jerry_get_object_native_pointer(JOBJ, (void**)&NAME,         \
                                         &native_info)) {             \
      return RTNODE_CREATE_ERROR(COMMON, "Internal");                     \
    }                                                                 \
  } while (0)

#define RTNODE_FUNCTION(name)                                \
  static jerry_value_t name(const jerry_value_t jfunc,   \
                            const jerry_value_t jthis,   \
                            const jerry_value_t jargv[], \
                            const jerry_length_t jargc)

#ifndef RTNODE_VM_HEAP_SIZE
#define RTNODE_VM_HEAP_SIZE 128 * 1024
#endif

#define RTNODE_BACKTRACE_DEPTH 11

/* Avoid compiler warnings if needed. */
#define RTNODE_UNUSED(x) ((void)(x))

typedef struct {
  void (*free_fn)(void *ptr);
  void* (*malloc_fn)(size_t size);
  void* (*realloc_fn)(void *ptr, size_t size);
  void* (*calloc_fn)(size_t count, size_t size);
} rtnode_allocator_t;

void rtnode_set_allocator(rtnode_allocator_t *allocator);

void rtnode_free(void* ptr);

void* rtnode_malloc(size_t size);

void* rtnode_realloc(void* ptr, size_t size);

void* rtnode_calloc(size_t count, size_t size);

void* rtnode_jerry_alloc(size_t size, void* cb_data);

uint64_t rtnode_get_memory_total();

uint64_t rtnode_get_memory_alloc_count();

typedef struct {
  rtev_ctx_t *rtev;
  jerry_context_t *jerry;
} rtnode_context_t;

rtnode_context_t* rtnode_get_context();

extern int rtnode_start();

#endif //_RTNODE_H_