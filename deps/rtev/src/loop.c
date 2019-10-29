#include "rtev.h"

int rtev_loop_init(rtev_ctx_t *ctx) {
  ctx->is_running = false;
  QUEUE_INIT(&ctx->timer_queue);
  QUEUE_INIT(&ctx->async_queue);
  ctx->watcher_count = 0;
  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_cond_init(&ctx->cond, NULL);
  ctx->closing_watchers = NULL;
  ctx->pending_watchers = NULL;
  return 0;
}

int rtev_loop_run(rtev_ctx_t *ctx, rtev_run_type_t type) {
  RTEV_ASSERT(ctx, "ctx is null");
  RTEV_ASSERT(ctx->is_running, "ctx is running");
  ctx->is_running = true;
  struct timespec next_timeout;

  while (ctx->watcher_count > 0) {
    _rtev_add_pending_watchers(ctx);
    _rtev_run_timers(ctx);
    _rtev_get_next_timeout(ctx, &next_timeout);
    pthread_mutex_lock(&ctx->mutex);
    pthread_cond_timedwait(&ctx->cond, &ctx->mutex, &next_timeout);
    pthread_mutex_unlock(&ctx->mutex);
    _rtev_close_watchers(ctx);
  }

#define RTEV_CHECK_QUEUE(name) \
  RTEV_ASSERT(QUEUE_EMPTY(&ctx->name), #name " is not empty");

  RTEV_CHECK_QUEUE(timer_queue);
  RTEV_CHECK_QUEUE(async_queue);

#undef RTEV_CHECK_QUEUE

  return 0;
}