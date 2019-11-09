
#include "rtnode-module-require.h"
#include "rtnode-error.h"
#include "rtnode-modules.h"

static jerry_value_t require_js_module(const char *name) {
  static const char* args = "exports, module, native, __filename";
  const rtnode_snapshot_module_t *module = rtnode_get_js_module(name);
  if (module == NULL) {
    return jerry_create_undefined();
  }
  jerry_value_t jfunc = jerry_parse_function(
    (const jerry_char_t *)module->name, strlen((const char *)module->name),
    (const jerry_char_t *)args, strlen(args),
    (const jerry_char_t *)module->code, module->length,
    JERRY_PARSE_STRICT_MODE);

  RTNODE_CHECK_FATAL_ERROR(jfunc, module->code);

  jerry_value_t jnative = jerry_create_undefined();
  /**
    try to get native binding module for js file,
    call native.[method] in js file
  **/
  const rtnode_native_module_t *binding_module =
    rtnode_get_native_module((const char *) module->name, JS_NATIVE_BINDING);
  if (binding_module != NULL) {
    RTNODE_LOG_I("get binding native module %s", module->name);
    jnative = binding_module->fn.binding();
  }

  jerry_value_t jthis = jerry_create_undefined();
  jerry_value_t jmodule = jerry_create_object();
  jerry_value_t jexports = jerry_create_object();
  rtnode_object_set_property(jmodule, "exports", jexports);
  jerry_value_t jfilename = jerry_create_string((jerry_char_t *)module->name);
  jerry_value_t jargv[] = { jexports, jmodule, jnative, jfilename };
  jerry_size_t jargc = sizeof(jargv) / sizeof(jerry_value_t);

  rtnode_call_function_void(jfunc, jthis, jargv, jargc, module->code);

  jerry_release_value(jfunc);
  jerry_release_value(jfilename);
  jerry_release_value(jexports);
  jerry_release_value(jthis);
  jerry_release_value(jnative);

  return jmodule;
}

static jerry_value_t require_native_module(const char *name) {
  const rtnode_native_module_t *module =
    rtnode_get_native_module(name, JS_NATIVE_BUILTIN);
  if (module == NULL) {
    return jerry_create_undefined();
  }
  jerry_value_t jmodule = jerry_create_object();
  jerry_value_t jexports = jerry_create_object();
  rtnode_object_set_property(jmodule, "exports", jexports);
  module->fn.builtin(jexports, jmodule, name);
  jerry_release_value(jexports);
  return jmodule;
}

RTNODE_FUNCTION(require) {
  static const char *cache_key = "_cached_modules";
  char *name = rtnode_object_to_string(jargv[0]);

  jerry_value_t jglobal = jerry_get_global_object();
  jerry_value_t jcached_modules = rtnode_object_get_property(jglobal, cache_key);
  if (jerry_value_is_undefined(jcached_modules)) {
    jerry_release_value(jcached_modules);
    jcached_modules = jerry_create_object();
    rtnode_object_set_property(jglobal, cache_key, jcached_modules);
  }

  jerry_value_t jmodule = rtnode_object_get_property(jcached_modules, name);
  bool cached = !jerry_value_is_undefined(jmodule);
  RTNODE_LOG_I("require module %s, cached %d", name, cached);
  if (!cached) {
    jmodule = require_js_module(name);
    if (jerry_value_is_undefined(jmodule)) {
      jmodule = require_native_module(name);
    }
    RTNODE_ASSERT(!jerry_value_is_undefined(jmodule));
    rtnode_object_set_property(jcached_modules, name, jmodule);
  }
  jerry_value_t jexports = rtnode_object_get_property(jmodule, "exports");
  jerry_release_value(jmodule);
  jerry_release_value(jcached_modules);
  jerry_release_value(jglobal);
  rtnode_free(name);
  return jexports;
}

jerry_value_t js_init_require() {
  jerry_value_t jvalue = jerry_create_external_function(require);
  return jvalue;
}
