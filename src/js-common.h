#ifndef _JS_COMMON_H_
#define _JS_COMMON_H_

#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "js-binding.h"
#include "js-error.h"
#include "js-logger.h"

#define JS_ABORT() \
  do { LOG_ERROR("abort message: %s:%d", __FILE__, __LINE__); abort(); } while (false)

#define JS_ASSERT(exp) \
  do { \
    if(!(exp)) { \
      LOG_ERROR("assert message: %s", #exp); \
      assert(0); \
    } \
  } while (false)

#define JS_CHECK_FATAL_ERROR(jobj, source) \
  if (jerry_value_is_error(jobj)) { \
    js_on_fatal_error(jobj, source); \
    assert(0); \
  }

#define JS_CREATE_ERROR(TYPE, message) \
  jerry_create_error(JERRY_ERROR_##TYPE, (const jerry_char_t*)message)

#define JS_DECLARE_PTR(JOBJ, TYPE, NAME)                              \
  TYPE* NAME = NULL;                                                  \
  do {                                                                \
    if (!jerry_get_object_native_pointer(JOBJ, (void**)&NAME,         \
                                         &this_module_native_info)) { \
      return JS_CREATE_ERROR(COMMON, "Internal");                     \
    }                                                                 \
  } while (0)

#define JS_FUNCTION(name)                                \
  static jerry_value_t name(const jerry_value_t jfunc,   \
                            const jerry_value_t jthis,   \
                            const jerry_value_t jargv[], \
                            const jerry_length_t jargc)

#define JS_VM_HEAP_SIZE 512 * 1024

#define JS_BACKTRACE_DEPTH 11

void js_free(void *ptr);

void* js_malloc(size_t size);

void* js_realloc(void *ptr, size_t size);

void* js_calloc(size_t count, size_t size);

void* js_jerry_alloc(size_t size, void *cb_data);

uint64_t js_get_memory_total();

uint64_t js_get_memory_alloc_count();

#endif // _JS_COMMON_H_
