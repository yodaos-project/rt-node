#include "rtnode-module-console.h"

RTNODE_FUNCTION(print_stdout) {
  char *str = rtnode_object_to_string(jargv[0]);
  RTNODE_LOG_I("🤩  %s", str);
  rtnode_free(str);
  return jerry_create_undefined();
}

RTNODE_FUNCTION(print_stderr) {
  char *str = rtnode_object_to_string(jargv[0]);
  RTNODE_LOG_E("❌ %s", str);
  rtnode_free(str);
  return jerry_create_undefined();
}

jerry_value_t js_init_console() {
  jerry_value_t jconsole = jerry_create_object();
  // console.log
  rtnode_object_set_method(jconsole, "stdout", print_stdout);
  // console.error
  rtnode_object_set_method(jconsole, "stderr", print_stderr);
  return jconsole;
}
