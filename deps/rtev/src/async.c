#include "rtev.h"
#include "atomic-ops.h"

int rtev_async_start(rtev_ctx_t *ctx, rtev_async_t *async,
  rtev_async_cb cb, rtev_close_cb close_cb) {
  rtev_watcher_t *w = (rtev_watcher_t *) async;
  int r = _rtev_watcher_init(ctx, w, RTEV_TYPE_ASYNC, close_cb);
  if (r != 0) {
    return r;
  }
  async->cb = cb;
  async->pending = 0;
  return _rtev_watcher_start(w);
}

int rtev_async_send(rtev_async_t *async) {
  if (cmpxchgi(&async->pending, 0, 1) == 0) {
    pthread_mutex_lock(&async->ctx->mutex);
    pthread_cond_signal(&async->ctx->cond);
    pthread_mutex_unlock(&async->ctx->mutex);
  }
  return 0;
}

void _rtev_run_async(rtev_ctx_t *ctx) {
  QUEUE *q;
  rtev_async_t *a;
  QUEUE_FOREACH(q, &ctx->async_queue) {
    a = QUEUE_DATA(q, rtev_async_t, node);
    if (a->state != RTEV_STATE_RUNNING) {
      continue;
    }
    if (cmpxchgi(&a->pending, 1, 0) == 0) {
      continue;
    }
    a->cb(a);
  }
}

int rtev_async_close(rtev_async_t *async) {
  return _rtev_watcher_close((rtev_watcher_t *) async);
}
