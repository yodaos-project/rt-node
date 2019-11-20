#include "rv.h"

int rv_async_start(rv_ctx_t *ctx, rv_async_t *async,
                     rv_async_cb cb, rv_close_cb close_cb) {
  rv_watcher_t *w = (rv_watcher_t *) async;
  int r = _rv_watcher_init(ctx, w, RV_TYPE_ASYNC, close_cb);
  if (r != 0) {
    return r;
  }
  async->cb = cb;
  return _rv_watcher_pending(w);
}

int rv_async_send(rv_async_t *async) {
  RV_ASSERT(async->state == RV_STATE_RUNNING ||
              async->state == RV_STATE_PENDING, "invalid async state");
  pthread_mutex_lock(&async->ctx->async_lock);
  rv_watcher_t **a = (rv_watcher_t **) &async->ctx->done_asyncs;
  while (*a) {
    if (*a == (rv_watcher_t *) async) {
      break;
    }
    a = &(*a)->next_watcher;
  }
  if (!*a) {
    *a = (rv_watcher_t *) async;
  }
  pthread_cond_signal(&async->ctx->async_cond);
  pthread_mutex_unlock(&async->ctx->async_lock);
  return 0;
}

void _rv_run_done_async(rv_ctx_t *ctx) {
  rv_async_t *async;
  pthread_mutex_lock(&ctx->async_lock);
  async = ctx->done_asyncs;
  ctx->done_asyncs = NULL;
  pthread_mutex_unlock(&ctx->async_lock);

  rv_async_t *a;
  while (async != NULL) {
    if (async->state == RV_STATE_RUNNING) {
      async->cb(async);
    }
    a = (rv_async_t *) async->next_watcher;
    async->next_watcher = NULL;
    async = a;
  }
}

int rv_async_close(rv_async_t *async) {
  return _rv_watcher_close((rv_watcher_t *) async);
}
