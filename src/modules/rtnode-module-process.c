#include "rtnode-module-process.h"

void js_run_next_tick() {
  RTNODE_ABORT();
  // jerry_value_t jglobal = jerry_get_global_object();
  // jerry_value_t jprocess = rtnode_object_get_property(jglobal, "process");
  // jerry_value_t jfunc = rtnode_object_get_property(jprocess, "_onNextTick");
  // rtnode_call_function_void(jfunc, jprocess, NULL, 0, NULL);
  // jerry_release_value(jfunc);
  // jerry_release_value(jprocess);
  // jerry_release_value(jglobal);
}

RTNODE_FUNCTION(on_uncaught_exception) {
  // jargv[0] alway an error
  RTNODE_ASSERT(jerry_value_is_error(jargv[0]));
  RTNODE_CHECK_FATAL_ERROR(jargv[0], NULL);
  return jerry_create_undefined();
}

RTNODE_FUNCTION(memory_usage) {
  jerry_heap_stats_t stats;
  memset(&stats, 0, sizeof(jerry_heap_stats_t));
  jerry_get_memory_stats(&stats);
  jerry_value_t jret = jerry_create_object();

  uint64_t rss = rtnode_get_memory_total();
  uint64_t count = rtnode_get_memory_alloc_count();
  rtnode_object_set_number(jret, "rss", rss);
  rtnode_object_set_number(jret, "allocCount", count);
  rtnode_object_set_number(jret, "peakHeapTotal", stats.peak_allocated_bytes);
  rtnode_object_set_number(jret, "heapTotal", stats.size);
  rtnode_object_set_number(jret, "heapUsed", stats.allocated_bytes);
  return jret;
}

jerry_value_t js_init_process() {
  jerry_value_t jprocess = jerry_create_object();
  rtnode_object_set_method(jprocess, "memoryUsage", memory_usage);
  rtnode_object_set_method(jprocess,
                           "_on_uncaught_exception", on_uncaught_exception);

  return jprocess;
}
