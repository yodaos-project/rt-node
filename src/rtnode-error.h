#ifndef _RTNODE_ERROR_H_
#define _RTNODE_ERROR_H_

#include "rtnode-binding.h"

void rtnode_on_fatal_error(jerry_type_t jerror, const jerry_char_t *source);

void rtnode_print_error(jerry_value_t jerror, const jerry_char_t *source);

#endif // _RTNODE_ERROR_H_
