#include "js-logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int redirect_fd = -1;
static js_log_level min_level = JS_LOG_LEVEL_V;

static const char* level_colors[] = { "\x1b[0m", "\x1b[32m", "\x1b[33m",
                                      "\x1b[31m", "\x1b[35m" };

static const char* level_des[] = { "verbose", "info", "warn", "error",
                                   "fatal" };

void js_do_log(js_log_level level, const char* fmt, ...) {
  if (level < min_level) {
    return;
  }
  if (level < JS_LOG_LEVEL_V || level > JS_LOG_LEVEL_F) {
    level = JS_LOG_LEVEL_I;
  }
  struct timeval cur_time = {};
  gettimeofday(&cur_time, NULL);
  struct tm* tm = localtime(&cur_time.tv_sec);
  int year = tm->tm_year + 1900;
  int mon = tm->tm_mon + 1;
  int day = tm->tm_mday;
  int hour = tm->tm_hour;
  int min = tm->tm_min;
  int sec = tm->tm_sec;
  int ms = cur_time.tv_usec / 1000;
  size_t fmt_len = 64 + strlen(fmt);
  char fmt_buf[fmt_len];
  FILE* log_file;
  if (level >= JS_LOG_LEVEL_E) {
    log_file = stderr;
  } else {
    log_file = stdout;
  }
  const char* start_color;
  const char* end_color;
  const char* des = level_des[level];
  if (redirect_fd > -1) {
    start_color = "";
    end_color = "";
  } else {
    start_color = level_colors[level];
    end_color = level_colors[0];
  }
  sprintf(fmt_buf, "%s%04d-%02d-%02d %02d:%02d:%02d:%03d [%s] %s%s\n",
          start_color, year, mon, day, hour, min, sec, ms, des, fmt, end_color);

  va_list args;
  va_start(args, fmt);
  vfprintf(log_file, fmt_buf, args);
  va_end(args);
}

void js_set_logger_level(js_log_level level) {
  min_level = level;
}
