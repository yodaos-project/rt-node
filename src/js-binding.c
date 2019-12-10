#include "js-binding.h"
#include "js-common.h"

jerry_value_t js_object_get_property(jerry_value_t jobj, const char* name) {
  JS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jprop_name = js_create_string(name);
  jerry_value_t jprop = jerry_get_property(jobj, jprop_name);
  jerry_release_value(jprop_name);

  if (jerry_value_is_error(jprop)) {
    jerry_release_value(jprop);
    return jerry_acquire_value(jerry_create_undefined());
  }

  return jprop;
}

double js_object_to_number(jerry_value_t jobj) {
  JS_ASSERT(jerry_value_is_number(jobj));
  return jerry_get_number_value(jobj);
}

char* js_object_to_string(jerry_value_t jobj) {
  JS_ASSERT(jerry_value_is_string(jobj));
  jerry_size_t len = jerry_get_string_size(jobj);
  char* str = (char*)js_malloc(len + 1);
  jerry_string_to_char_buffer(jobj, (jerry_char_t*)str, len);
  str[len] = '\0';
  return str;
}

void js_object_set_property(jerry_value_t jobj, const char* name,
                            jerry_value_t jprop) {
  JS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jprop_name = js_create_string(name);
  jerry_value_t jret = jerry_set_property(jobj, jprop_name, jprop);

  JS_CHECK_FATAL_ERROR(jret, NULL);
  jerry_release_value(jret);
  jerry_release_value(jprop_name);
}

void js_object_set_method(jerry_value_t jobj, const char* name,
                          jerry_external_handler_t handler) {
  JS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jfunc = jerry_create_external_function(handler);
  js_object_set_property(jobj, name, jfunc);
  jerry_release_value(jfunc);
}

void js_object_set_number(jerry_value_t jobj, const char* name, double value) {
  JS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jvalue = jerry_create_number(value);
  js_object_set_property(jobj, name, jvalue);
  jerry_release_value(jvalue);
}

void js_object_set_string(jerry_value_t jobj, const char* name,
                          const char* value) {
  JS_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jvalue = js_create_string(value);
  js_object_set_property(jobj, name, jvalue);
  jerry_release_value(jvalue);
}

jerry_value_t js_call_function(jerry_value_t jfunc, jerry_value_t jthis,
                               const jerry_value_t* jargv, jerry_size_t jargc,
                               const jerry_char_t* source) {
  jerry_value_t jret =
    js_call_function_no_tick(jfunc, jthis, jargv, jargc, source);
  // js_run_next_tick();
  return jret;
}

void js_call_function_void(jerry_value_t jfunc, jerry_value_t jthis,
                           const jerry_value_t* jargv, jerry_size_t jargc,
                           const jerry_char_t* source) {
  jerry_value_t jret = js_call_function(jfunc, jthis, jargv, jargc, source);
  jerry_release_value(jret);
}

jerry_value_t js_call_function_no_tick(jerry_value_t jfunc, jerry_value_t jthis,
                                       const jerry_value_t* jargv,
                                       jerry_size_t jargc,
                                       const jerry_char_t* source) {
  JS_ASSERT(jerry_value_is_function(jfunc));
  jerry_value_t jret = jerry_call_function(jfunc, jthis, jargv, jargc);
  JS_CHECK_FATAL_ERROR(jret, source);
  return jret;
}

jerry_value_t js_create_string(const char* str) {
  jerry_size_t size = strlen(str);
  JS_ASSERT(jerry_is_valid_utf8_string((jerry_char_t*)str, size));
  return jerry_create_string_sz_from_utf8((jerry_char_t*)str, size);
}

char* js_object_get_string(jerry_value_t jobj, const char* name) {
  jerry_value_t jvalue = js_object_get_property(jobj, name);
  char* value = js_object_to_string(jvalue);
  jerry_release_value(jvalue);
  return value;
}

double js_object_get_number(jerry_value_t jobj, const char* name) {
  jerry_value_t jvalue = js_object_get_property(jobj, name);
  double value = js_object_to_number(jvalue);
  jerry_release_value(jvalue);
  return value;
}
