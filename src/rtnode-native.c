#include "rtnode-native.h"
#include "rtnode-module-console.h"
#include "rtnode-module-require.h"
#include "rtnode-module-timer.h"
#include "rtnode-module-process.h"

const rtnode_native_module_t native_modules[] = {
  { 
    .type = JS_NATIVE_GLOBAL,
    .fn.global = rtnode_init_require,
    .name = "require"
  },
  { 
    .type = JS_NATIVE_BINDING,
    .fn.global = rtnode_init_process,
    .name = "process"
  },
  { 
    .type = JS_NATIVE_BINDING,
    .fn.binding = rtnode_init_console,
    .name = "console"
  },
  { 
    .type = JS_NATIVE_BINDING,
    .fn.binding = rtnode_init_timer,
    .name = "timers"
  }
};

const size_t native_modules_size = sizeof(native_modules) / sizeof(rtnode_native_module_t);
