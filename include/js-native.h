#ifndef _JS_NATIVE_H_
#define _JS_NATIVE_H_

#include "js-common.h"

typedef void (*js_builtin_fn)(jerry_value_t jexports, jerry_value_t jmodule,
                              const char* filename);
typedef jerry_value_t (*js_global_fn)();
typedef jerry_value_t (*js_binding_fn)();

enum js_native_module_type {
  JS_NATIVE_BUILTIN,
  JS_NATIVE_GLOBAL,
  JS_NATIVE_BINDING,
};

typedef struct {
  enum js_native_module_type type;
  union {
    js_builtin_fn builtin; // require(name).fn
    js_global_fn global;   // global.fn
    js_binding_fn binding; // native.fn
  } fn;
  const char* name;
} js_native_module_t;

extern const js_native_module_t native_modules[];
extern const size_t native_modules_size;

#endif // _JS_NATIVE_H_
