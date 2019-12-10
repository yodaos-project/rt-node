#ifndef _JS_COMMON_H_
#define _JS_COMMON_H_

#include "rv.h"
#include "js-binding.h"
#include "js-error.h"
#include "js-logger.h"

#define NODE_MAJOR_VERSION 0
#define NODE_MINOR_VERSION 0
#define NODE_PATCH_VERSION 1

#if !defined(STRINGIFY)
#define STRINGIFY(x) #x
#endif /* STRINGIFY */

#if !defined(TOSTRING)
#define TOSTRING(x) STRINGIFY(x)
#endif /* TOSTRING */

#define JS_ABORT(msg)                                            \
  do {                                                           \
    JS_LOG_E("fatal error: %s:%d\n%s", __FILE__, __LINE__, msg); \
    abort();                                                     \
  } while (false)

#define JS_ASSERT(exp)           \
  do {                           \
    if (!(exp)) {                \
      JS_ABORT("assert: " #exp); \
      abort();                   \
    }                            \
  } while (false)

#define JS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(name)                  \
  static void js_##name##_destroy(js_##name##_t* wrap);                 \
  static const jerry_object_native_info_t this_module_native_info = {   \
    .free_cb = (jerry_object_native_free_callback_t)js_##name##_destroy \
  }

#define JS_CHECK_FATAL_ERROR(jobj, source) \
  if (jerry_value_is_error(jobj)) {        \
    js_on_fatal_error(jobj, source);       \
    JS_ABORT("fatal error occurred");      \
  }

#define JS_CREATE_ERROR(TYPE, message) \
  jerry_create_error(JERRY_ERROR_##TYPE, (const jerry_char_t*)message)

#define JS_DECLARE_PTR(JOBJ, TYPE, NAME)                                       \
  TYPE* NAME = NULL;                                                           \
  do {                                                                         \
    if (!jerry_get_object_native_pointer(JOBJ, (void**)&NAME, &native_info)) { \
      return JS_CREATE_ERROR(COMMON, "Internal");                              \
    }                                                                          \
  } while (0)

#define JS_FUNCTION(name)                                \
  static jerry_value_t name(const jerry_value_t jfunc,   \
                            const jerry_value_t jthis,   \
                            const jerry_value_t jargv[], \
                            const jerry_length_t jargc)

#ifndef JS_VM_HEAP_SIZE
#define JS_VM_HEAP_SIZE 128 * 1024
#endif

#define JS_BACKTRACE_DEPTH 11

/* Avoid compiler warnings if needed. */
#define JS_UNUSED(x) ((void)(x))

typedef struct {
  void (*free_fn)(void* ptr);
  void* (*malloc_fn)(size_t size);
  void* (*realloc_fn)(void* ptr, size_t size);
  void* (*calloc_fn)(size_t count, size_t size);
} js_allocator_t;

void js_set_allocator(js_allocator_t* allocator);

void js_free(void* ptr);

void* js_malloc(size_t size);

void* js_realloc(void* ptr, size_t size);

void* js_calloc(size_t count, size_t size);

void* js_jerry_alloc(size_t size, void* cb_data);

uint64_t js_get_memory_total();

uint64_t js_get_memory_alloc_count();

typedef struct {
  rv_ctx_t* rv;
  jerry_context_t* jerry;
} js_context_t;

js_context_t* js_get_context();

#endif //_JS_COMMON_H_
