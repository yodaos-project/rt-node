#include "console.h"

JS_FUNCTION(print_stdout) {
  char *str = js_object_to_string(jargv[0]);
  JS_LOG_I("🤩  %s", str);
  js_free(str);
  return jerry_create_undefined();
}

JS_FUNCTION(print_stderr) {
  char *str = js_object_to_string(jargv[0]);
  JS_LOG_E("❌ %s", str);
  js_free(str);
  return jerry_create_undefined();
}

jerry_value_t js_init_console() {
  jerry_value_t jconsole = jerry_create_object();
  // console.log
  js_object_set_method(jconsole, "stdout", print_stdout);
  // console.error
  js_object_set_method(jconsole, "stderr", print_stderr);
  return jconsole;
}
