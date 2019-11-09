#include <stdio.h>
#include <string.h>
#include "rt-node.h"
#include "rtnode-modules.h"
#include "node_api.h"

int rtnode_start() {
  RTNODE_LOG_I("#####  rtnode start 🐒 #####");
  srand((unsigned)jerry_port_get_current_time());

  js_ctx = (rtnode_context_t *) rtnode_malloc(sizeof(rtnode_context_t));

  js_ctx->rtev = (rtev_ctx_t *) rtnode_malloc(sizeof(rtev_ctx_t));
  rtev_set_allocator(rtnode_malloc, rtnode_free);
  rtev_ctx_init(js_ctx->rtev);

  js_ctx->jerry = jerry_create_context(RTNODE_VM_HEAP_SIZE, rtnode_jerry_alloc,
                                       NULL);
  jerry_port_default_set_current_context(js_ctx->jerry);
  jerry_init(JERRY_INIT_EMPTY);
  rtnode_load_global_modules();
  const char *entry_name = "global";
  const rtnode_snapshot_module_t *snapshot = rtnode_get_js_module(entry_name);
  jerry_value_t jglobal = jerry_get_global_object();
  jerry_value_t jfunc = jerry_parse(
    (const jerry_char_t *)entry_name, strlen(entry_name),
    (const jerry_char_t *)snapshot->code, snapshot->length,
    JERRY_PARSE_STRICT_MODE);
  RTNODE_CHECK_FATAL_ERROR(jfunc, snapshot->code);
  rtnode_call_function_void(jfunc, jglobal, NULL, 0, snapshot->code);
  jerry_release_value(jfunc);
  jerry_release_value(jglobal);

  rtev_ctx_loop(js_ctx->rtev, RTEV_RUN_DEFAULT);

  jerry_cleanup();
  rtnode_free(js_ctx->jerry);
  rtnode_free(js_ctx->rtev);
  rtnode_free(js_ctx);

  uint64_t mem_left = rtnode_get_memory_total();
  if (mem_left > 0) {
    uint64_t alloc_count = rtnode_get_memory_alloc_count();
    RTNODE_LOG_W("memory leak, left: %ld, count: %ld", mem_left, alloc_count);
  }
  RTNODE_LOG_I("#####  rtnode end 🙈 #####");

  return 0;
}
