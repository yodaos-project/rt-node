#include "rtev.h"

int rtev_async_start(rtev_ctx_t *ctx, rtev_async_t *async,
  rtev_async_cb cb, rtev_close_cb close_cb) {
  rtev_watcher_t *w = (rtev_watcher_t *) async;
  int r = _rtev_watcher_init(ctx, w, RTEV_TYPE_ASYNC, close_cb);
  if (r != 0) {
    return r;
  }
  async->cb = cb;
  return _rtev_watcher_pending(w);
}

int rtev_async_send(rtev_async_t *async) {
  RTEV_ASSERT(async->state == RTEV_STATE_RUNNING, "invalid async state");
  pthread_mutex_lock(&async->ctx->async_lock);
  rtev_watcher_t **a = (rtev_watcher_t **) &async->ctx->done_asyncs;
  while (*a) {
    if (*a == (rtev_watcher_t *) async) {
      break;
    }
    a = &(*a)->next_watcher;
  }
  if (!*a) {
    *a = (rtev_watcher_t *) async;
  }
  pthread_cond_signal(&async->ctx->async_cond);
  pthread_mutex_unlock(&async->ctx->async_lock);
  return 0;
}

void _rtev_run_done_async(rtev_ctx_t *ctx) {
  rtev_async_t *async;
  pthread_mutex_lock(&ctx->async_lock);
  async = ctx->done_asyncs;
  ctx->done_asyncs = NULL;
  pthread_mutex_unlock(&ctx->async_lock);

  rtev_async_t *a;
  while (async != NULL) {
    if (async->state == RTEV_STATE_RUNNING) {
      async->cb(async);
    }
    a = (rtev_async_t *) async->next_watcher;
    async->next_watcher = NULL;
    async = a;
  }
}

int rtev_async_close(rtev_async_t *async) {
  return _rtev_watcher_close((rtev_watcher_t *) async);
}
