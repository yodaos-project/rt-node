#include "js-modules.h"

void js_load_global_modules() {
  jerry_value_t jglobal = jerry_get_global_object();
  for (size_t i = 0; i < native_modules_size; ++i) {
    const js_native_module_t *native_module = &native_modules[i];
    if (native_module->type == JS_NATIVE_GLOBAL) {
      JS_LOG_I("load global module %s", native_module->name);
      jerry_value_t jvalue = native_module->fn.global();
      js_object_set_property(jglobal, native_module->name, jvalue);
      jerry_release_value(jvalue);
    }
  }
  jerry_release_value(jglobal);
}

const js_snapshot_module_t* js_get_js_module(const char *name) {
  const js_snapshot_module_t *module = NULL;
  for (int i = 0; i < js_modules_size; ++i) {
    if (strcmp(js_modules[i].name, name) == 0) {
      module = &js_modules[i];
      break;
    }
  }
  return module;
}

const js_native_module_t* js_get_native_module(const char *name,
                                                   enum js_native_module_type type) {
  const js_native_module_t *module = NULL;
  for (int i = 0; i < native_modules_size; ++i) {
    if (native_modules[i].type != type) {
      continue;
    }
    if (strcmp(native_modules[i].name, name) == 0) {
      module = &native_modules[i];
      break;
    }
  }
  return module;
}
