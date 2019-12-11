#include <stdio.h>
#include <string.h>
#include "js-common.h"
#include "js-modules.h"
#include "node_api.h"
#include "node_api_internal.h"

int js_start() {
  JS_LOG_I("#####  js start 🐒 #####");
  srand((unsigned)jerry_port_get_current_time());
  js_context_t* js_ctx = js_get_context();

  js_ctx->rv = (rv_ctx_t*)js_malloc(sizeof(rv_ctx_t));
  rv_set_allocator(js_malloc, js_free);
  rv_ctx_init(js_ctx->rv);

  js_ctx->jerry = jerry_create_context(JS_VM_HEAP_SIZE, js_jerry_alloc, NULL);
  jerry_port_default_set_current_context(js_ctx->jerry);
  jerry_init(JERRY_INIT_EMPTY);
  js_setup_napi();
  js_load_global_modules();
  const char* entry_name = "global";
  const js_snapshot_module_t* snapshot = js_get_js_module(entry_name);
  jerry_value_t jglobal = jerry_get_global_object();
  jerry_value_t jfunc =
    jerry_parse((const jerry_char_t*)entry_name, strlen(entry_name),
                (const jerry_char_t*)snapshot->code, snapshot->length,
                JERRY_PARSE_STRICT_MODE);
  JS_CHECK_FATAL_ERROR(jfunc, snapshot->code);
  js_call_function_void(jfunc, jglobal, NULL, 0, snapshot->code);
  jerry_release_value(jfunc);
  jerry_release_value(jglobal);

  rv_ctx_loop(js_ctx->rv, RV_RUN_DEFAULT);

  js_cleanup_napi();
  jerry_cleanup();
  js_free(js_ctx->jerry);
  js_free(js_ctx->rv);
  js_free(js_ctx);

  uint64_t mem_left = js_get_memory_total();
  if (mem_left > 0) {
    uint64_t alloc_count = js_get_memory_alloc_count();
    JS_LOG_W("memory leak, left: %ld, count: %ld", mem_left, alloc_count);
  }
  JS_LOG_I("#####  js end 🙈 #####");

  return 0;
}
