#ifndef _JS_ERROR_H_
#define _JS_ERROR_H_

#include "js-binding.h"

void js_on_fatal_error(jerry_type_t jerror, const jerry_char_t *source);

void js_print_error(jerry_value_t jerror, const jerry_char_t *source);

#endif // _JS_ERROR_H_
