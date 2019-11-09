#ifndef _JS_NATIVE_H_
#define _JS_NATIVE_H_

#include "rt-node.h"

typedef void (*rtnode_builtin_fn)(jerry_value_t jexports,
  jerry_value_t jmodule, const char *filename);
typedef jerry_value_t (*rtnode_global_fn)();
typedef jerry_value_t (*rtnode_binding_fn)();

enum rtnode_native_module_type {
  JS_NATIVE_BUILTIN,
  JS_NATIVE_GLOBAL,
  JS_NATIVE_BINDING,
};

typedef struct {
  enum rtnode_native_module_type type;
  union {
    rtnode_builtin_fn builtin; // require(name).fn
    rtnode_global_fn global; // global.fn
    rtnode_binding_fn binding; // native.fn
  } fn;
  const char *name;
} rtnode_native_module_t;

extern const rtnode_native_module_t native_modules[];
extern const size_t native_modules_size;

#endif // _JS_NATIVE_H_
