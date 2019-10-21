#ifndef _JS_MODULE_GPIO_H_
#define _JS_MODULE_GPIO_H_

#include "js-common.h"

// js native module
void js_init_gpio(jerry_value_t jexports, 
  jerry_value_t jmodule, const char *filename);

#endif // _JS_MODULE_GPIO_H_