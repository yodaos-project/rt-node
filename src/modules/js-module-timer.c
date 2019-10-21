#include "js-module-timer.h"
#include "js-uv-handle.h"

static const jerry_object_native_info_t this_module_native_info = { NULL };

static void timeout_handler(uv_timer_t* handle) {
  JS_ASSERT(handle != NULL);

  jerry_value_t jobject = JS_UV_HANDLE_DATA(handle)->jobject;
  jerry_value_t jcallback = js_object_get_property(jobject, "handleTimeout");
  js_call_function_void(jcallback, jobject, NULL, 0, NULL);
  jerry_release_value(jcallback);
}

JS_FUNCTION(timer_start){
  JS_DECLARE_PTR(jthis, uv_timer_t, timer_handle);

  uint64_t timeout = (uint64_t)js_object_to_number(jargv[0]);
  uint64_t repeat = (uint64_t)js_object_to_number(jargv[1]);
  int res = uv_timer_start(timer_handle, timeout_handler, timeout, repeat);
  return jerry_create_number(res);
}

JS_FUNCTION(timer_stop) {
  JS_DECLARE_PTR(jthis, uv_handle_t, timer_handle);
  if (!uv_is_closing(timer_handle)) {
    js_uv_handle_close(timer_handle, NULL);
  }

  return jerry_create_number(0);
}

JS_FUNCTION(timer_constructor) {
  JS_ASSERT(jerry_value_is_object(jthis));

  uv_handle_t* handle = js_uv_handle_create(sizeof(uv_timer_t),
    jthis,
    &this_module_native_info,
    0);

  uv_timer_init(uv_default_loop(), (uv_timer_t  *)handle);

  return jerry_create_undefined();
}

jerry_value_t js_init_timer() {
  jerry_value_t jtimer = jerry_create_external_function(timer_constructor);
  jerry_value_t jprototype = jerry_create_object();
  js_object_set_property(jtimer, "prototype", jprototype);
  js_object_set_method(jprototype, "start", timer_start);
  js_object_set_method(jprototype, "stop", timer_stop);
  jerry_release_value(jprototype);

  return jtimer;
}
