#include "rtnode-module-timer.h"
#include "rtnode-rtev-watcher.h"

static const jerry_object_native_info_t native_info = { NULL };

static void on_timeout(rtev_timer_t* timer) {
  jerry_value_t jobject = RTNODE_RTEV_WATCHER_DATA(timer)->jobject;
  jerry_value_t jcallback = rtnode_object_get_property(jobject, "handleTimeout");
  rtnode_call_function_void(jcallback, jobject, NULL, 0, NULL);
  jerry_release_value(jcallback);
}

RTNODE_FUNCTION(timer_start) {
  RTNODE_DECLARE_PTR(jthis, rtev_timer_t, timer);
  uint64_t timeout = (uint64_t) rtnode_object_to_number(jargv[0]);
  uint64_t repeat = (uint64_t) rtnode_object_to_number(jargv[1]);
  int r = rtev_timer_start(
    js_ctx->rtev,
    timer,
    timeout,
    repeat,
    on_timeout,
    timer->close_cb);
  return jerry_create_number(r);
}

RTNODE_FUNCTION(timer_stop) {
  RTNODE_DECLARE_PTR(jthis, rtev_watcher_t, watcher);
  int r = rtnode_rtev_watcher_close(watcher);
  return jerry_create_number(r);
}

RTNODE_FUNCTION(timer_constructor) {
  RTNODE_ASSERT(jerry_value_is_object(jthis));
  rtnode_rtev_watcher_bind(sizeof(rtev_timer_t), jthis, &native_info, 0, NULL);

  return jerry_create_undefined();
}

jerry_value_t js_init_timer() {
  jerry_value_t jtimer = jerry_create_external_function(timer_constructor);
  jerry_value_t jprototype = jerry_create_object();
  rtnode_object_set_property(jtimer, "prototype", jprototype);
  rtnode_object_set_method(jprototype, "start", timer_start);
  rtnode_object_set_method(jprototype, "stop", timer_stop);
  jerry_release_value(jprototype);

  return jtimer;
}
