#include "rtev.h"

int rtev_loop_init(rtev_ctx_t *ctx) {
  ctx->is_running = false;
  QUEUE_INIT(&ctx->timer_queue);
  QUEUE_INIT(&ctx->async_queue);
  ctx->watcher_count = 0;
  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_condattr_t cond_attr;
  pthread_condattr_init(&cond_attr);
#if !(defined(__APPLE__) && defined(__MACH__))
  pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
#endif
  pthread_cond_init(&ctx->cond, &cond_attr);
  pthread_condattr_destroy(&cond_attr);
  ctx->closing_watchers = NULL;
  ctx->pending_watchers = NULL;
  _rtev_update_time(ctx);
  return 0;
}

int rtev_loop_run(rtev_ctx_t *ctx, rtev_run_type_t type) {
  RTEV_ASSERT(!ctx->is_running, "ctx is running");
  ctx->is_running = true;
  struct timespec next_time;

  int r;
  while (ctx->watcher_count > 0) {
    _rtev_add_pending_watchers(ctx);
    _rtev_run_timers(ctx);
    _rtev_set_next_timeout(ctx, &next_time);
    pthread_mutex_lock(&ctx->mutex);
#if defined(__APPLE__) && defined(__MACH__)
    r = pthread_cond_timedwait_relative_np(&ctx->cond, &ctx->mutex, &next_time);
#else
    r = pthread_cond_timedwait(&ctx->cond, &ctx->mutex, &next_time);
#endif
    RTEV_ASSERT(r != 0 || r != ETIMEDOUT, "unexpected time wait error");
    pthread_mutex_unlock(&ctx->mutex);
    _rtev_close_watchers(ctx);
  }

#define RTEV_CHECK_QUEUE(name) \
  RTEV_ASSERT(QUEUE_EMPTY(&ctx->name), "ctx->" #name " is not empty");

  RTEV_CHECK_QUEUE(timer_queue);
  RTEV_CHECK_QUEUE(async_queue);

#undef RTEV_CHECK_QUEUE

  return 0;
}
