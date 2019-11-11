#include "rtev.h"

int rtev_tick_start(rtev_ctx_t *ctx, rtev_tick_t *tick, rtev_tick_cb cb,
  rtev_close_cb close_cb) {
  rtev_watcher_t *w = (rtev_watcher_t *) tick;
  int r = _rtev_watcher_init(ctx, w, RTEV_TYPE_TICK, close_cb);
  if (r != 0) {
    return r;
  }
  tick->cb = cb;
  return _rtev_watcher_pending(w);
}

int rtev_tick_close(rtev_tick_t *tick) {
  return _rtev_watcher_close((rtev_watcher_t *) tick);
}

void _rtev_run_ticks(rtev_ctx_t *ctx) {
  QUEUE *q;
  rtev_tick_t *t;
  QUEUE_FOREACH(q, &ctx->tick_queue) {
    t = QUEUE_DATA(q, rtev_tick_t, node);
    if (t->state != RTEV_STATE_RUNNING) {
      continue;
    }
    t->cb(t);
  }
}
