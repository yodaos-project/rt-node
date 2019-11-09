#include "rtnode-binding.h"
#include "rt-node.h"
#include "rtnode-module-process.h"

jerry_value_t rtnode_object_get_property(jerry_value_t jobj, const char *name) {
  RTNODE_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jprop_name = rtnode_create_string(name);
  jerry_value_t jprop = jerry_get_property(jobj, jprop_name);
  jerry_release_value(jprop_name);

  if (jerry_value_is_error(jprop)) {
    jerry_release_value(jprop);
    return jerry_acquire_value(jerry_create_undefined());
  }

  return jprop;
}

double rtnode_object_to_number(jerry_value_t jobj) {
  RTNODE_ASSERT(jerry_value_is_number(jobj));
  return jerry_get_number_value(jobj);
}

char* rtnode_object_to_string(jerry_value_t jobj) {
  RTNODE_ASSERT(jerry_value_is_string(jobj));
  jerry_size_t len = jerry_get_string_size(jobj);
  char *str = (char *) rtnode_malloc(len + 1);
  jerry_string_to_char_buffer(jobj, (jerry_char_t *)str, len);
  str[len] = '\0';
  return str;
}

void rtnode_object_set_property(jerry_value_t jobj, const char *name,
                                jerry_value_t jprop) {
  RTNODE_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jprop_name = rtnode_create_string(name);
  jerry_value_t jret = jerry_set_property(jobj, jprop_name, jprop);

  RTNODE_CHECK_FATAL_ERROR(jret, NULL);
  jerry_release_value(jret);
  jerry_release_value(jprop_name);
}

void rtnode_object_set_method(jerry_value_t jobj, const char *name,
                              jerry_external_handler_t handler) {
  RTNODE_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jfunc = jerry_create_external_function(handler);
  rtnode_object_set_property(jobj, name, jfunc);
  jerry_release_value(jfunc);
}

void rtnode_object_set_number(jerry_value_t jobj, const char *name, double value) {
  RTNODE_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jvalue = jerry_create_number(value);
  rtnode_object_set_property(jobj, name, jvalue);
  jerry_release_value(jvalue);
}

void rtnode_object_set_string(jerry_value_t jobj,
                              const char *name, const char *value) {
  RTNODE_ASSERT(jerry_value_is_object(jobj));

  jerry_value_t jvalue = rtnode_create_string(value);
  rtnode_object_set_property(jobj, name, jvalue);
  jerry_release_value(jvalue);
}

jerry_value_t rtnode_call_function(jerry_value_t jfunc,
                                   jerry_value_t jthis,
                                   const jerry_value_t *jargv,
                                   jerry_size_t jargc,
                                   const jerry_char_t *source) {
  jerry_value_t jret =
    rtnode_call_function_no_tick(jfunc, jthis, jargv, jargc, source);
  // js_run_next_tick();
  return jret;
}

void rtnode_call_function_void(jerry_value_t jfunc,
                               jerry_value_t jthis,
                               const jerry_value_t *jargv,
                               jerry_size_t jargc,
                               const jerry_char_t *source) {
  jerry_value_t jret = rtnode_call_function(jfunc, jthis, jargv, jargc, source);
  jerry_release_value(jret);
}

jerry_value_t rtnode_call_function_no_tick(jerry_value_t jfunc,
                                           jerry_value_t jthis,
                                           const jerry_value_t *jargv,
                                           jerry_size_t jargc,
                                           const jerry_char_t *source) {
  RTNODE_ASSERT(jerry_value_is_function(jfunc));
  jerry_value_t jret = jerry_call_function(jfunc, jthis, jargv, jargc);
  RTNODE_CHECK_FATAL_ERROR(jret, source);
  return jret;
}

jerry_value_t rtnode_create_string(const char *str) {
  jerry_size_t size = strlen(str);
  RTNODE_ASSERT(jerry_is_valid_utf8_string((jerry_char_t *)str, size));
  return jerry_create_string_sz_from_utf8((jerry_char_t *)str, size);
}

char* rtnode_object_get_string(jerry_value_t jobj, const char *name) {
  jerry_value_t jvalue = rtnode_object_get_property(jobj, name);
  char *value = rtnode_object_to_string(jvalue);
  jerry_release_value(jvalue);
  return value;
}

double rtnode_object_get_number(jerry_value_t jobj, const char *name) {
  jerry_value_t jvalue = rtnode_object_get_property(jobj, name);
  double value = rtnode_object_to_number(jvalue);
  jerry_release_value(jvalue);
  return value;
}