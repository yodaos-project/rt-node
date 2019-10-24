#include <stdio.h>
#include <string.h>
#include "js-common.h"
#include "js-modules.h"
#include "js-error.h"

int js_main() {
  LOG_INFO("#####  jsruntime start 🐒 #####");
  srand((unsigned)jerry_port_get_current_time());

  jerry_context_t* context = jerry_create_context(JS_VM_HEAP_SIZE,
                                                  js_jerry_alloc,
                                                  NULL);
  jerry_port_default_set_current_context(context);
  jerry_init(JERRY_INIT_EMPTY);

  js_load_global_modules();

  const char *entry_name = "global";
  const js_snapshot_module_t *snapshot = js_get_js_module(entry_name);
  
  jerry_value_t jglobal = jerry_get_global_object();
  jerry_value_t jfunc = jerry_parse(
    (const jerry_char_t *)entry_name, strlen(entry_name),
    (const jerry_char_t *)snapshot->code, snapshot->length,
    JERRY_PARSE_STRICT_MODE);

  JS_CHECK_FATAL_ERROR(jfunc, snapshot->code);

  js_call_function_void(jfunc, jglobal, NULL, 0, snapshot->code);

  jerry_cleanup();
  js_free(context);
  uint64_t mem_left = js_get_memory_total();
  if (mem_left > 0) {
    uint64_t alloc_count = js_get_memory_alloc_count();
    LOG_WARN("memory leak, left: %ld, count: %ld", mem_left, alloc_count);
  }
  LOG_INFO("#####  jsruntime end 🙈 #####");
  return 0;
}
