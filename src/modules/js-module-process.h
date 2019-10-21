#ifndef _JS_MODULE_PROCESS_H_
#define _JS_MODULE_PROCESS_H_

#include "js-common.h"

jerry_value_t js_init_process();

void js_run_next_tick();

#endif // _JS_MODULE_PROCESS_H_
