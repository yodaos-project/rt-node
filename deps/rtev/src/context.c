#include "rtev.h"

int rtev_ctx_init(rtev_ctx_t *ctx) {
  ctx->is_running = false;
  QUEUE_INIT(&ctx->timer_queue);
  QUEUE_INIT(&ctx->async_queue);
  QUEUE_INIT(&ctx->tick_queue);
  ctx->watcher_count = 0;
  _rtev_threadpool_init();
  pthread_mutex_init(&ctx->async_mutex, NULL);
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
    pthread_mutex_lock(&ctx->async_mutex);
#if defined(__APPLE__) && defined(__MACH__)
    r = pthread_cond_timedwait_relative_np(
      &ctx->async_cond, &ctx->async_mutex, &next_time);
#else
    r = pthread_cond_timedwait(&ctx->async_cond, &ctx->async_mutex, &next_time);
#endif
    RTEV_ASSERT(r != 0 || r != ETIMEDOUT, "unexpected time wait error");
    pthread_mutex_unlock(&ctx->async_mutex);
    _rtev_run_async(ctx);
    _rtev_run_ticks(ctx);
    _rtev_close_watchers(ctx);
  }

#define RTEV_CHECK_QUEUE(name) \
  RTEV_ASSERT(QUEUE_EMPTY(&ctx->name), "ctx->" #name " is not empty");

#define RTEV_CHECK_LINK(name) \
  RTEV_ASSERT(!ctx->name, "ctx->" #name " is not empty");

  RTEV_CHECK_QUEUE(timer_queue);
  RTEV_CHECK_QUEUE(async_queue);
  RTEV_CHECK_QUEUE(tick_queue);
  RTEV_CHECK_LINK(pending_watchers);
  RTEV_CHECK_LINK(closing_watchers);

#undef RTEV_CHECK_LINK
#undef RTEV_CHECK_QUEUE
  rtev_threadpool_stop();
  return 0;
}
