#include "rv.h"

int rv_tick_start(rv_ctx_t *ctx, rv_tick_t *tick, rv_tick_cb cb,
                    rv_close_cb close_cb) {
  rv_watcher_t *w = (rv_watcher_t *) tick;
  int r = _rv_watcher_init(ctx, w, RV_TYPE_TICK, close_cb);
  if (r != 0) {
    return r;
  }
  tick->cb = cb;
  return _rv_watcher_pending(w);
}

int rv_tick_close(rv_tick_t *tick) {
  return _rv_watcher_close((rv_watcher_t *) tick);
}

void _rv_run_ticks(rv_ctx_t *ctx) {
  QUEUE *q;
  rv_tick_t *t;
  QUEUE_FOREACH(q, &ctx->tick_queue) {
    t = QUEUE_DATA(q, rv_tick_t, node);
    if (t->state != RV_STATE_RUNNING) {
      continue;
    }
    t->cb(t);
  }
}
