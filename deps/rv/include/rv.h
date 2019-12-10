#ifndef _RV_H_
#define _RV_H_

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

#define RV_ASSERT(exp, reason)                                  \
  do {                                                          \
    if (!(exp)) {                                               \
      fprintf(stderr, #exp ", %s\n", reason);                   \
      abort();                                                  \
    }                                                           \
  } while (0)

typedef enum {
  RV_RUN_DEFAULT = 1,
  RV_RUN_ONCE,
} rv_run_type_t;

typedef enum {
  RV_TYPE_TIMER = 1,
  RV_TYPE_ASYNC,
  RV_TYPE_TICK,
  RV_TYPE_WORKER,
} rv_watcher_type_t;

typedef enum {
  RV_STATE_PENDING = 1, // out of queue, insert at tick start
  RV_STATE_RUNNING,     // in the queue
  RV_STATE_CLOSING,     // in the queue, remove at tick end
  RV_STATE_CLOSED,      // removed from queue
} rv_watcher_state_t;

typedef struct rv_ctx_t rv_ctx_t;
typedef struct rv_watcher_t rv_watcher_t;
typedef struct rv_async_t rv_async_t;
typedef struct rv_timer_t rv_timer_t;
typedef struct rv_tick_t rv_tick_t;
typedef struct rv_worker_t rv_worker_t;

#ifndef RV_THREADPOOL_SIZE
#define RV_THREADPOOL_SIZE 2
#endif

// context structure start
struct rv_ctx_t {
  bool is_running;
  QUEUE timer_queue;
  QUEUE async_queue;
  rv_async_t *done_asyncs;
  QUEUE tick_queue;
  QUEUE worker_queue;
  int worker_count;
  pthread_mutex_t worker_lock;
  pthread_cond_t worker_cond;
  rv_watcher_t *closing_watchers;
  rv_watcher_t *pending_watchers;
  int watcher_count;
  uint64_t time;          // in ms
  pthread_mutex_t async_lock;
  pthread_cond_t async_cond;
};

// watcher close callback
typedef void (*rv_close_cb)(rv_watcher_t *watcher);

// watcher common fields
#define RV_WATCHER_FIELDS                                     \
  /* public fields */                                           \
  void *data;                                                   \
  /* private fields */                                          \
  rv_close_cb close_cb;                                       \
  QUEUE node;                                                   \
  rv_watcher_state_t state;                                   \
  rv_ctx_t *ctx;                                              \
  rv_watcher_t *next_watcher;                                 \
  rv_watcher_type_t type;

// common structure for watchers
struct rv_watcher_t {
  RV_WATCHER_FIELDS;
};

// timer structure start
typedef void (*rv_timer_cb)(rv_timer_t *timer);
struct rv_timer_t {
  RV_WATCHER_FIELDS;
  uint64_t timeout;       // in ms
  uint64_t repeat;
  rv_timer_cb cb;
};

// async structure start
typedef void (*rv_async_cb)(rv_async_t *async);
struct rv_async_t {
  RV_WATCHER_FIELDS;
  rv_async_cb cb;       // run in main thread
};

// tick structure start
typedef void (*rv_tick_cb)(rv_tick_t *tick);
struct rv_tick_t {
  RV_WATCHER_FIELDS;
  rv_tick_cb cb;
};

typedef void (*rv_worker_cb)(rv_worker_t *worker);
struct rv_worker_t {
  RV_WATCHER_FIELDS;
  rv_async_t *async;
  rv_worker_cb cb;
  rv_worker_cb done_cb;
  rv_close_cb user_close_cb;
};

// core fn start
int rv_ctx_init(rv_ctx_t *ctx);
int rv_ctx_loop(rv_ctx_t *ctx, rv_run_type_t type);

// watcher
int rv_watcher_close(rv_watcher_t *watcher);

// memory allocator fn start
int rv_set_allocator(void* (*malloc_fn)(size_t), void (*free_fn)(void *));
void* rv_malloc(size_t size);
void rv_free(void *ptr);

// timer fn start
int rv_timer_start(rv_ctx_t *ctx, rv_timer_t *timer, uint64_t timeout,
                     uint64_t repeat, rv_timer_cb cb, rv_close_cb close_cb);
int rv_timer_close(rv_timer_t *timer);

// async fn start
int rv_async_start(rv_ctx_t *ctx, rv_async_t *async, rv_async_cb cb,
                     rv_close_cb close_cb);
/* call in other threads to make an async callback in main thread */
int rv_async_send(rv_async_t *async);
int rv_async_close(rv_async_t *async);

// tick fn start
int rv_tick_start(rv_ctx_t *ctx, rv_tick_t *tick, rv_tick_cb cb,
                    rv_close_cb close_cb);
int rv_tick_close(rv_tick_t *tick);

// worker fn start
int rv_worker_start(rv_ctx_t *ctx, rv_worker_t *worker, rv_worker_cb cb,
                      rv_worker_cb done_cb, rv_close_cb close_cb);
int rv_worker_close(rv_worker_t *worker);

#define rv_watcher_init(w) \
  memset(w, sizeof(w), 0);

// internal fn start
int _rv_watcher_init(rv_ctx_t *ctx, rv_watcher_t *watcher,
                       rv_watcher_type_t type, rv_close_cb close_cb);
int _rv_watcher_pending(rv_watcher_t *watcher);
int _rv_watcher_close(rv_watcher_t *watcher);
void _rv_add_pending_watchers(rv_ctx_t *ctx);
void _rv_close_watchers(rv_ctx_t *ctx);
void _rv_update_time(rv_ctx_t *ctx);
void _rv_threadpool_init(rv_ctx_t *ctx);
void _rv_threadpool_stop(rv_ctx_t *ctx);
void _rv_run_done_async(rv_ctx_t *ctx);
void _rv_run_timers(rv_ctx_t *ctx);
void _rv_run_ticks(rv_ctx_t *ctx);

#endif // _RV_H_
