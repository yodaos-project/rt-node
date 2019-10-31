#ifndef _RTEV_H_
#define _RTEV_H_

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "queue.h"

#define RTEV_ASSERT(exp, reason)                                \
  do {                                                          \
    if (!(exp)) {                                               \
      fprintf(stderr, #exp ", %s\n", reason);                   \
      abort();                                                  \
    }                                                           \
  } while (0)

typedef enum {
  RTEV_RUN_DEFAULT = 1,
  RTEV_RUN_ONCE,
} rtev_run_type_t;

typedef enum {
  RTEV_TYPE_TIMER = 1,
  RTEV_TYPE_ASYNC,
} rtev_watcher_type_t;

typedef enum {
  RTEV_STATE_PENDING = 1, // out of queue, insert at tick start
  RTEV_STATE_RUNNING,     // in the queue, able to tirgger
  RTEV_STATE_CLOSING,     // in the queue, remove at tick end
  RTEV_STATE_CLOSED,      // removed from queue
} rtev_watcher_state_t;

typedef struct rtev_ctx_t rtev_ctx_t;
typedef struct rtev_watcher_t rtev_watcher_t;
typedef struct rtev_async_t rtev_async_t;
typedef struct rtev_timer_t rtev_timer_t;

// context structure start
struct rtev_ctx_t {
  bool is_running;
  QUEUE timer_queue;
  QUEUE async_queue;
  rtev_watcher_t *closing_watchers;
  rtev_watcher_t *pending_watchers;
  uint64_t watcher_count;
  uint64_t time;          // in ms
  pthread_mutex_t mutex;
  pthread_cond_t cond;
};

// watcher close callback
typedef void (*rtev_watcher_close_cb)(rtev_watcher_t *watcher);

// watcher common fields
#define RTEV_WATCHER_FIELDS                                     \
  /* public fields */                                           \
  void *data;                                                   \
  rtev_watcher_close_cb close_cb;                               \
  /* private fields */                                          \
  QUEUE node;                                                   \
  rtev_watcher_state_t state;                                   \
  rtev_ctx_t *ctx;                                              \
  rtev_watcher_t *next_watcher;                                 \
  rtev_watcher_type_t type;

// common structure for watchers
struct rtev_watcher_t {
  RTEV_WATCHER_FIELDS;
};

// timer structure start
typedef void (*rtev_timer_cb)(rtev_timer_t *timer);
struct rtev_timer_t {
  RTEV_WATCHER_FIELDS;
  uint64_t timeout;       // in ms
  uint64_t repeat;
  rtev_timer_cb cb;
};

// async structure start
typedef void (*rtev_async_cb)(rtev_async_t *async, void *data);
struct rtev_async_t {
  RTEV_WATCHER_FIELDS;
  rtev_async_cb cb;
  rtev_async_cb after_cb;
};

// core fn start
int rtev_ctx_init(rtev_ctx_t *ctx);
int rtev_ctx_run(rtev_ctx_t *ctx, rtev_run_type_t type);

// memory allocator fn start
int rtev_set_allocator(void* (*malloc_fn)(size_t), void (*free_fn)(void *));
void* rtev_malloc(size_t size);
void rtev_free(void *ptr);

// timer fn start
int rtev_timer_start(rtev_ctx_t *ctx, rtev_timer_t *timer, uint64_t timeout,
  uint64_t repeat, rtev_timer_cb cb, rtev_watcher_close_cb close_cb);
int rtev_timer_close(rtev_timer_t *timer);

// async fn start
int rtev_async_start(rtev_ctx_t *ctx, rtev_async_t *async, rtev_async_cb cb,
  rtev_async_cb after_cb, rtev_watcher_close_cb close_cb);
int rtev_async_send(rtev_async_t *async);
int rtev_async_close(rtev_async_t *async);

// internal fn start
int _rtev_watcher_init(rtev_ctx_t *ctx, rtev_watcher_t *watcher,
  rtev_watcher_type_t type, rtev_watcher_close_cb close_cb);
int _rtev_watcher_start(rtev_watcher_t *watcher);
int _rtev_watcher_close(rtev_watcher_t *watcher);
void _rtev_add_pending_watchers(rtev_ctx_t *ctx);
void _rtev_close_watchers(rtev_ctx_t *ctx);
void _rtev_set_next_timeout(rtev_ctx_t *ctx, struct timespec *spec);
void _rtev_run_timers(rtev_ctx_t *ctx);
void _rtev_update_time(rtev_ctx_t *ctx);

#endif // _RTEV_H_
