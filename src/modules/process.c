#include "process.h"

void js_run_next_tick() {
  JS_LOG_W("unimplemented method js_uv_work_cb");
  //  JS_ABORT();
  // jerry_value_t jglobal = jerry_get_global_object();
  // jerry_value_t jprocess = js_object_get_property(jglobal, "process");
  // jerry_value_t jfunc = js_object_get_property(jprocess, "_onNextTick");
  // js_call_function_void(jfunc, jprocess, NULL, 0, NULL);
  // jerry_release_value(jfunc);
  // jerry_release_value(jprocess);
  // jerry_release_value(jglobal);
}

JS_FUNCTION(on_uncaught_exception) {
  // jargv[0] alway an error
  JS_ASSERT(jerry_value_is_error(jargv[0]));
  JS_CHECK_FATAL_ERROR(jargv[0], NULL);
  return jerry_create_undefined();
}

JS_FUNCTION(memory_usage) {
  jerry_heap_stats_t stats;
  memset(&stats, 0, sizeof(jerry_heap_stats_t));
  jerry_get_memory_stats(&stats);
  jerry_value_t jret = jerry_create_object();

  uint64_t rss = js_get_memory_total();
  uint64_t count = js_get_memory_alloc_count();
  js_object_set_number(jret, "rss", rss);
  js_object_set_number(jret, "allocCount", count);
  js_object_set_number(jret, "peakHeapTotal", stats.peak_allocated_bytes);
  js_object_set_number(jret, "heapTotal", stats.size);
  js_object_set_number(jret, "heapUsed", stats.allocated_bytes);
  return jret;
}

jerry_value_t js_init_process() {
  jerry_value_t jprocess = jerry_create_object();
  js_object_set_method(jprocess, "memoryUsage", memory_usage);
  js_object_set_method(jprocess, "_on_uncaught_exception",
                       on_uncaught_exception);

  return jprocess;
}
