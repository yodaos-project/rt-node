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
#include "atomic-ops.h"

#ifdef __FREERTOS__
#include "freertos/FreeRTOS.h"
#endif

#define RTEV_ASSERT(exp, reason)                                \
  do {                                                          \
    if (!(exp)) {                                               \
      fprintf(stderr, #exp ", %s\n", reason);                   \
      assert(0);                                                \
    }                                                           \
  } while (0)

typedef enum {
  RTEV_RUN_DEFAULT = 1,
  RTEV_RUN_ONCE,
} rtev_run_type_t;

typedef enum {
  RTEV_TYPE_TIMER = 1,
  RTEV_TYPE_ASYNC,
  RTEV_TYPE_TICK,
  RTEV_TYPE_WORKER,
} rtev_watcher_type_t;

typedef enum {
  RTEV_STATE_PENDING = 1, // out of queue, insert at tick start
  RTEV_STATE_RUNNING,     // in the queue
  RTEV_STATE_CLOSING,     // in the queue, remove at tick end
  RTEV_STATE_CLOSED,      // removed from queue
} rtev_watcher_state_t;

typedef struct rtev_ctx_t rtev_ctx_t;
typedef struct rtev_watcher_t rtev_watcher_t;
typedef struct rtev_async_t rtev_async_t;
typedef struct rtev_timer_t rtev_timer_t;
typedef struct rtev_tick_t rtev_tick_t;
typedef struct rtev_worker_t rtev_worker_t;

#ifndef RTEV_THREADPOOL_SIZE
#define RTEV_THREADPOOL_SIZE 2
#endif

// context structure start
struct rtev_ctx_t {
  bool is_running;
  QUEUE timer_queue;
  QUEUE async_queue;
  QUEUE tick_queue;
  QUEUE worker_queue;
  int worker_count;
  pthread_mutex_t worker_lock;
  pthread_cond_t worker_cond;
  rtev_async_t *worker_async;
  QUEUE worker_done_queue;
  pthread_mutex_t worker_done_lock;
  rtev_watcher_t *closing_watchers;
  rtev_watcher_t *pending_watchers;
  int watcher_count;
  uint64_t time;          // in ms
  pthread_mutex_t async_lock;
  pthread_cond_t async_cond;
  int async_pending;
};

// watcher close callback
typedef void (*rtev_close_cb)(rtev_watcher_t *watcher);

// watcher common fields
#define RTEV_WATCHER_FIELDS                                     \
  /* public fields */                                           \
  void *data;                                                   \
  /* private fields */                                          \
  rtev_close_cb close_cb;                                       \
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
typedef void (*rtev_async_cb)(rtev_async_t *async);
struct rtev_async_t {
  RTEV_WATCHER_FIELDS;
  int pending;
  rtev_async_cb cb;       // run in main thread
};

// tick structure start
typedef void (*rtev_tick_cb)(rtev_tick_t *tick);
struct rtev_tick_t {
  RTEV_WATCHER_FIELDS;
  rtev_tick_cb cb;
};

typedef void (*rtev_worker_cb)(rtev_worker_t *worker);
struct rtev_worker_t {
  RTEV_WATCHER_FIELDS;
  int pending;
  rtev_worker_cb cb;
  rtev_worker_cb done_cb;
};

// core fn start
int rtev_ctx_init(rtev_ctx_t *ctx);
int rtev_ctx_loop(rtev_ctx_t *ctx, rtev_run_type_t type);

// watcher
int rtev_watcher_close(rtev_watcher_t *watcher);

// memory allocator fn start
int rtev_set_allocator(void* (*malloc_fn)(size_t), void (*free_fn)(void *));
void* rtev_malloc(size_t size);
void rtev_free(void *ptr);

// timer fn start
int rtev_timer_start(rtev_ctx_t *ctx, rtev_timer_t *timer, uint64_t timeout,
  uint64_t repeat, rtev_timer_cb cb, rtev_close_cb close_cb);
int rtev_timer_close(rtev_timer_t *timer);

// async fn start
int rtev_async_start(rtev_ctx_t *ctx, rtev_async_t *async, rtev_async_cb cb,
  rtev_close_cb close_cb);
/* call in other threads to make an async callback in main thread */
int rtev_async_send(rtev_async_t *async);
int rtev_async_close(rtev_async_t *async);

// tick fn start
int rtev_tick_start(rtev_ctx_t *ctx, rtev_tick_t *tick, rtev_tick_cb cb,
  rtev_close_cb close_cb);
int rtev_tick_close(rtev_tick_t *tick);

// worker fn start
int rtev_worker_start(rtev_ctx_t *ctx, rtev_worker_t *worker, rtev_worker_cb cb,
  rtev_worker_cb done_cb, rtev_close_cb close_cb);
int rtev_worker_close(rtev_worker_t *worker);

// internal fn start
int _rtev_watcher_init(rtev_ctx_t *ctx, rtev_watcher_t *watcher,
  rtev_watcher_type_t type, rtev_close_cb close_cb);
int _rtev_watcher_pending(rtev_watcher_t *watcher);
int _rtev_watcher_close(rtev_watcher_t *watcher);
void _rtev_add_pending_watchers(rtev_ctx_t *ctx);
void _rtev_close_watchers(rtev_ctx_t *ctx);
void _rtev_update_time(rtev_ctx_t *ctx);
void _rtev_threadpool_init(rtev_ctx_t *ctx);
void _rtev_threadpool_stop(rtev_ctx_t *ctx);
void _rtev_run_async(rtev_ctx_t *ctx);
void _rtev_run_timers(rtev_ctx_t *ctx);
void _rtev_run_ticks(rtev_ctx_t *ctx);

#endif // _RTEV_H_
