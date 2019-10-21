#include <stdio.h>
#include <string.h>
#include "js-common.h"
#include "uv-common.h"
#include "js-modules.h"
#include "js-error.h"

static void uv_close_handle(uv_handle_t *handle, void *arg) {
  if (!uv_is_closing(handle)) {
    uv_close(handle, NULL);
  }
}

static void* run_js(void *data) {
  LOG_INFO("#####  jsruntime start 🐒 #####");
  uv_replace_allocator(js_malloc, js_realloc, js_calloc, js_free);
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

  uv_loop_t* loop = uv_default_loop();
  uv_run(loop, UV_RUN_DEFAULT);

  uv_walk(loop, uv_close_handle, NULL);
  uv_run(loop, UV_RUN_DEFAULT);

  JS_ASSERT(uv_loop_close(loop) == 0);

  jerry_cleanup();
  js_free(context);
  uint64_t mem_left = js_get_memory_total();
  if (mem_left > 0) {
    uint64_t alloc_count = js_get_memory_alloc_count();
    LOG_WARN("memory leak, left: %ld, count: %ld", mem_left, alloc_count);
  }
  LOG_INFO("#####  jsruntime end 🙈 #####");
  return NULL;
}

int jsruntime_init() {
  pthread_t th;
  pthread_create(&th, NULL, run_js, NULL);
  pthread_detach(th);
  return 0;
}
