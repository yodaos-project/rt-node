#ifndef _JS_BINDING_H_
#define _JS_BINDING_H_

#include "jerryscript.h"
#include "jerryscript-ext/handler.h"
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"
#include "jerryscript-ext/handle-scope.h"

jerry_value_t js_object_get_property(jerry_value_t jobj, const char *name);

// remember free return value
char* js_object_to_string(jerry_value_t jobj);

double js_object_to_number(jerry_value_t jobj);

void js_object_set_property(jerry_value_t jobj, const char *name,
                                jerry_value_t jprop);

void js_object_set_method(jerry_value_t jobj, const char *name,
                              jerry_external_handler_t handler);

void js_object_set_number(jerry_value_t jobj, const char *name, double value);

void js_object_set_string(jerry_value_t jobj,
                              const char *name, const char *value);

char* js_object_get_string(jerry_value_t jobj, const char *name);

double js_object_get_number(jerry_value_t jobj, const char *name);

jerry_value_t js_create_string(const char *str);

jerry_value_t js_call_function(jerry_value_t jfunc,
                                   jerry_value_t jthis,
                                   const jerry_value_t *jargv,
                                   jerry_size_t jargc,
                                   const jerry_char_t *source);

void js_call_function_void(jerry_value_t jfunc,
                               jerry_value_t jthis,
                               const jerry_value_t *jargv,
                               jerry_size_t jargc,
                               const jerry_char_t *source);

jerry_value_t js_call_function_no_tick(jerry_value_t jfunc,
                                           jerry_value_t jthis,
                                           const jerry_value_t *jargv,
                                           jerry_size_t jargc,
                                           const jerry_char_t *source);

#endif // _JS_BINDING_H_
