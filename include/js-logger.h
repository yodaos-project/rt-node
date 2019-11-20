#ifndef _JS_LOGGER_H_
#define _JS_LOGGER_H_
#include <stdio.h>

typedef enum {
  JS_LOG_LEVEL_V = 0,
  JS_LOG_LEVEL_I = 1,
  JS_LOG_LEVEL_W = 2,
  JS_LOG_LEVEL_E = 3,
  JS_LOG_LEVEL_F = 4,
} js_log_level;

#define JS_LOG_V(...) js_do_log(LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define JS_LOG_I(...) js_do_log(JS_LOG_LEVEL_I, __VA_ARGS__)
#define JS_LOG_W(...) js_do_log(JS_LOG_LEVEL_W, __VA_ARGS__)
#define JS_LOG_E(...) js_do_log(JS_LOG_LEVEL_E, __VA_ARGS__)
#define JS_LOG_F(...) js_do_log(LOG_LEVEL_FATAL, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void js_do_log(js_log_level level, const char *fmt, ...);

void js_set_logger_file_directory(const char *directory);

// default is verbose
void js_set_logger_level(js_log_level level);

#ifdef __cplusplus
}
#endif

#endif // _JS_LOGGER_H_