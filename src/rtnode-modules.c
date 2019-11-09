
#include "rtnode-modules.h"

void rtnode_load_global_modules() {
  jerry_value_t jglobal = jerry_get_global_object();
  for (size_t i = 0; i < native_modules_size; ++i) {
    const rtnode_native_module_t *native_module = &native_modules[i];
    if (native_module->type == JS_NATIVE_GLOBAL) {
      RTNODE_LOG_I("load global module %s", native_module->name);
      jerry_value_t jvalue = native_module->fn.global();
      rtnode_object_set_property(jglobal, native_module->name, jvalue);
      jerry_release_value(jvalue);
    }
  }
  jerry_release_value(jglobal);
}

const rtnode_snapshot_module_t* rtnode_get_js_module(const char *name) {
  const rtnode_snapshot_module_t *module = NULL;
  for (int i = 0; i < rtnode_modules_size; ++i) {
    if (strcmp(rtnode_modules[i].name, name) == 0) {
      module = &rtnode_modules[i];
      break;
    }
  }
  return module;
}

const rtnode_native_module_t* rtnode_get_native_module(const char *name,
                                                   enum rtnode_native_module_type type) {
  const rtnode_native_module_t *module = NULL;
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
