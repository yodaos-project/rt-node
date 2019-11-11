#ifndef _RTNODE_LOGGER_H_
#define _RTNODE_LOGGER_H_
#include <stdio.h>

typedef enum {
  RTNODE_LOG_LEVEL_V = 0,
  RTNODE_LOG_LEVEL_I = 1,
  RTNODE_LOG_LEVEL_W = 2,
  RTNODE_LOG_LEVEL_E = 3,
  RTNODE_LOG_LEVEL_F = 4,
} rtnode_log_level;

#define RTNODE_LOG_V(...) rtnode_do_log(LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define RTNODE_LOG_I(...) rtnode_do_log(RTNODE_LOG_LEVEL_I, __VA_ARGS__)
#define RTNODE_LOG_W(...) rtnode_do_log(RTNODE_LOG_LEVEL_W, __VA_ARGS__)
#define RTNODE_LOG_E(...) rtnode_do_log(RTNODE_LOG_LEVEL_E, __VA_ARGS__)
#define RTNODE_LOG_F(...) rtnode_do_log(LOG_LEVEL_FATAL, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void rtnode_do_log(rtnode_log_level level, const char *fmt, ...);

void rtnode_set_logger_file_directory(const char *directory);

// default is verbose
void rtnode_set_logger_level(rtnode_log_level level);

#ifdef __cplusplus
}
#endif

#endif // _RTNODE_LOGGER_H_