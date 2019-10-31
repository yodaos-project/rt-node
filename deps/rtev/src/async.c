#include "rtev.h"

int rtev_async_start(rtev_ctx_t *ctx, rtev_async_t *async,
  rtev_async_cb cb, rtev_async_cb after_cb, rtev_watcher_close_cb close_cb) {
  _rtev_watcher_init(ctx, (rtev_watcher_t *) async, RTEV_TYPE_ASYNC, close_cb);
  async->cb = cb;
  async->after_cb = after_cb;
  return 0;
}

int rtev_async_send(rtev_async_t *async) {
  return 0;
}
