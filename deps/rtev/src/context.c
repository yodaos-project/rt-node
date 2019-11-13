#include "rtev.h"

void _rtev_set_next_timeout(rtev_ctx_t *ctx, struct timespec *spec) {
  spec->tv_sec = 0;
  spec->tv_nsec = 0;
  if (cmpxchgi(&ctx->async_pending, 1, 0) == 1) {
    return;
  }
  if (ctx->closing_watchers) {
    return;
  }
  if (ctx->pending_watchers) {
    return;
  }
#if !(defined(__APPLE__) && defined(__MACH__))
  clock_gettime(CLOCK_MONOTONIC, spec);
#endif
  QUEUE *q;
  rtev_timer_t *timer = NULL;
  QUEUE_FOREACH(q, &ctx->timer_queue) {
    rtev_timer_t *t = QUEUE_DATA(q, rtev_timer_t, node);
    if (t->state == RTEV_STATE_RUNNING) {
      RTEV_ASSERT(t->timeout >= ctx->time, "unexpected timeout");
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

int rtev_ctx_init(rtev_ctx_t *ctx) {
  ctx->is_running = false;
  QUEUE_INIT(&ctx->timer_queue);
  QUEUE_INIT(&ctx->async_queue);
  QUEUE_INIT(&ctx->tick_queue);
  QUEUE_INIT(&ctx->worker_queue);
  QUEUE_INIT(&ctx->worker_done_queue);
  pthread_mutex_init(&ctx->worker_done_lock, NULL);
  pthread_mutex_init(&ctx->worker_lock, NULL);
  pthread_cond_init(&ctx->worker_cond, NULL);
  ctx->worker_count = 0;
  ctx->worker_async = (rtev_async_t *) rtev_malloc(sizeof(rtev_async_t));
  ctx->watcher_count = 0;
  pthread_mutex_init(&ctx->async_lock, NULL);
  pthread_condattr_t cond_attr;
  pthread_condattr_init(&cond_attr);
#if !(defined(__APPLE__) && defined(__MACH__))
  pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
#endif
  pthread_cond_init(&ctx->async_cond, &cond_attr);
  // FIXME: link error in esp-idf
#ifndef __FREERTOS__
  pthread_condattr_destroy(&cond_attr);
#endif
  ctx->closing_watchers = NULL;
  ctx->pending_watchers = NULL;
  ctx->async_pending = 0;
  _rtev_update_time(ctx);
  _rtev_threadpool_init(ctx);
  return 0;
}

int rtev_ctx_loop(rtev_ctx_t *ctx, rtev_run_type_t type) {
  RTEV_ASSERT(!ctx->is_running, "ctx is running");
  ctx->is_running = true;
  struct timespec next_time;

  int r;
  while (ctx->watcher_count > 0) {
    _rtev_add_pending_watchers(ctx);
    _rtev_run_timers(ctx);
    _rtev_set_next_timeout(ctx, &next_time);
    printf("timeout %ld.%lds \n", next_time.tv_sec, next_time.tv_nsec / 1000000);
    pthread_mutex_lock(&ctx->async_lock);
#if defined(__APPLE__) && defined(__MACH__)
    r = pthread_cond_timedwait_relative_np(
      &ctx->async_cond, &ctx->async_lock, &next_time);
#else
    r = pthread_cond_timedwait(&ctx->async_cond, &ctx->async_lock, &next_time);
#endif
    RTEV_ASSERT(r != 0 || r != ETIMEDOUT, "unexpected time wait error");
    pthread_mutex_unlock(&ctx->async_lock);
    printf("tick end\n");
    _rtev_run_async(ctx);
    _rtev_run_ticks(ctx);
    _rtev_close_watchers(ctx);
  }
  printf("loop finish\n");

#define RTEV_CHECK_QUEUE(name) \
  RTEV_ASSERT(QUEUE_EMPTY(&ctx->name), "ctx->" #name " is not empty");

#define RTEV_CHECK_LINK(name) \
  RTEV_ASSERT(!ctx->name, "ctx->" #name " is not empty");

  RTEV_CHECK_QUEUE(timer_queue);
  RTEV_CHECK_QUEUE(async_queue);
  RTEV_CHECK_QUEUE(tick_queue);
  RTEV_CHECK_QUEUE(worker_queue);
  RTEV_CHECK_LINK(pending_watchers);
  RTEV_CHECK_LINK(closing_watchers);

#undef RTEV_CHECK_LINK
#undef RTEV_CHECK_QUEUE
  ctx->is_running = false;
  _rtev_threadpool_stop(ctx);
  return 0;
}
