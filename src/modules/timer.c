#include "timer.h"
#include "js-rv-watcher.h"

static const jerry_object_native_info_t native_info = { NULL };

static void on_timeout(rv_timer_t* timer) {
  jerry_value_t jobj = JS_RV_WATCHER_DATA(timer)->jobject;
  jerry_value_t jcallback = js_object_get_property(jobj, "handleTimeout");
  js_call_function_void(jcallback, jobj, NULL, 0, NULL);
  jerry_release_value(jcallback);
}

JS_FUNCTION(timer_start) {
  JS_DECLARE_PTR(jthis, rv_timer_t, timer);
  uint64_t timeout = (uint64_t)js_object_to_number(jargv[0]);
  uint64_t repeat = (uint64_t)js_object_to_number(jargv[1]);
  int r = rv_timer_start(js_get_context()->rv, timer, timeout, repeat,
                         on_timeout, timer->close_cb);
  return jerry_create_number(r);
}

JS_FUNCTION(timer_stop) {
  JS_DECLARE_PTR(jthis, rv_watcher_t, watcher);
  int r = js_rv_watcher_close(watcher);
  return jerry_create_number(r);
}

JS_FUNCTION(timer_constructor) {
  JS_ASSERT(jerry_value_is_object(jthis));
  js_rv_watcher_bind(sizeof(rv_timer_t), jthis, &native_info, 0, NULL);

  return jerry_create_undefined();
}

jerry_value_t js_init_timer() {
  jerry_value_t jtimer = jerry_create_external_function(timer_constructor);
  jerry_value_t jprop = jerry_create_object();
  js_object_set_property(jtimer, "prototype", jprop);
  js_object_set_method(jprop, "start", timer_start);
  js_object_set_method(jprop, "stop", timer_stop);
  jerry_release_value(jprop);

  return jtimer;
}
