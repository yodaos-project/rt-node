#include "rv.h"

#define RV_CHECK_QUEUE(name) \
  RV_ASSERT(QUEUE_EMPTY(&ctx->name), "ctx->" #name " is not empty");

#define RV_CHECK_LINK(name) \
  RV_ASSERT(!ctx->name, "ctx->" #name " is not empty");

static void _rv_set_next_timeout(rv_ctx_t *ctx, struct timespec *spec) {
  spec->tv_sec = 0;
  spec->tv_nsec = 0;
#if !(defined(__APPLE__) && defined(__MACH__))
  clock_gettime(CLOCK_MONOTONIC, spec);
#endif
  QUEUE *q;
  rv_timer_t *timer = NULL;
  QUEUE_FOREACH(q, &ctx->timer_queue) {
    rv_timer_t *t = QUEUE_DATA(q, rv_timer_t, node);
    if (t->state == RV_STATE_RUNNING) {
      RV_ASSERT(t->timeout >= ctx->time, "unexpected timeout");
      timer = t;
      break;
    }
  }
  uint64_t next_timeout;
  if (timer) {
    next_timeout = timer->timeout - ctx->time;
  } else if (!QUEUE_EMPTY(&ctx->tick_queue)) {
    next_timeout = 0;
  } else {
    next_timeout = UINT32_MAX;
  }
  spec->tv_sec += next_timeout / 1000;
  spec->tv_nsec += (long) (next_timeout % 1000) * 1000000;
}

static void _rv_goto_sleep(rv_ctx_t *ctx) {
  if (ctx->closing_watchers || ctx->pending_watchers) {
    return;
  }
  pthread_mutex_t *lock = &ctx->async_lock;
  pthread_cond_t *cond = &ctx->async_cond;
  pthread_mutex_lock(lock);
  if (!ctx->done_asyncs) {
    struct timespec next_time;
    _rv_set_next_timeout(ctx, &next_time);
#if defined(__APPLE__) && defined(__MACH__)
    pthread_cond_timedwait_relative_np(cond, lock, &next_time);
#else
    pthread_cond_timedwait(cond, lock, &next_time);
#endif
  }
  pthread_mutex_unlock(&ctx->async_lock);
}

static void _rv_check_watchers(rv_ctx_t *ctx) {
  RV_CHECK_QUEUE(timer_queue);
  RV_CHECK_QUEUE(async_queue);
  RV_CHECK_QUEUE(tick_queue);
  RV_CHECK_QUEUE(worker_queue);
  RV_CHECK_LINK(done_asyncs);
  RV_CHECK_LINK(pending_watchers);
  RV_CHECK_LINK(closing_watchers);
}

int rv_ctx_init(rv_ctx_t *ctx) {
  ctx->is_running = false;
  QUEUE_INIT(&ctx->timer_queue);
  QUEUE_INIT(&ctx->async_queue);
  ctx->done_asyncs = NULL;
  QUEUE_INIT(&ctx->tick_queue);
  QUEUE_INIT(&ctx->worker_queue);
  pthread_mutex_init(&ctx->worker_lock, NULL);
  pthread_cond_init(&ctx->worker_cond, NULL);
  ctx->worker_count = 0;
  ctx->watcher_count = 0;
  pthread_mutex_init(&ctx->async_lock, NULL);
  pthread_condattr_t cond_attr;
  pthread_condattr_init(&cond_attr);
#if !(defined(__APPLE__) && defined(__MACH__))
  pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
#endif
  pthread_cond_init(&ctx->async_cond, &cond_attr);
  // FIXME: link error in esp-idf
#ifndef __ESP_IDF__
  pthread_condattr_destroy(&cond_attr);
#endif
  ctx->closing_watchers = NULL;
  ctx->pending_watchers = NULL;
  _rv_update_time(ctx);
  return 0;
}

int rv_ctx_loop(rv_ctx_t *ctx, rv_run_type_t type) {
  RV_ASSERT(!ctx->is_running, "ctx is running");
  ctx->is_running = true;
  _rv_threadpool_init(ctx);

  while (ctx->watcher_count > 0) {
    _rv_add_pending_watchers(ctx);
    _rv_run_timers(ctx);
    _rv_goto_sleep(ctx);
    _rv_run_done_async(ctx);
    _rv_run_ticks(ctx);
    _rv_close_watchers(ctx);
  }

  _rv_check_watchers(ctx);

  ctx->is_running = false;
  _rv_threadpool_stop(ctx);
  return 0;
}
