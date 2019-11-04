#include "js-native.h"
#include "js-module-console.h"
#include "js-module-require.h"
#include "js-module-timer.h"
#include "js-module-process.h"

const js_native_module_t native_modules[] = {
  { 
    .type = JS_NATIVE_GLOBAL,
    .fn.global = js_init_require,
    .name = "require"
  },
  { 
    .type = JS_NATIVE_BINDING,
    .fn.global = js_init_process,
    .name = "process"
  },
  { 
    .type = JS_NATIVE_BINDING,
    .fn.binding = js_init_console,
    .name = "console"
  },
  { 
    .type = JS_NATIVE_BINDING,
    .fn.binding = js_init_timer,
    .name = "timers"
  }
};

const size_t native_modules_size = sizeof(native_modules) / sizeof(js_native_module_t);
