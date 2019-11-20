#include "rv.h"

void _rv_update_time(rv_ctx_t *ctx) {
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);
  ctx->time = spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}

int rv_timer_start(rv_ctx_t *ctx, rv_timer_t *timer, uint64_t timeout,
                     uint64_t repeat, rv_timer_cb cb, rv_close_cb close_cb) {
  rv_watcher_t *w = (rv_watcher_t *) timer;
  int r = _rv_watcher_init(ctx, w, RV_TYPE_TIMER, close_cb);
  if (r != 0) {
    return r;
  }
  timer->timeout = ctx->time + timeout;
  timer->repeat = repeat;
  timer->cb = cb;

  return _rv_watcher_pending(w);
}

int rv_timer_close(rv_timer_t *timer) {
  return _rv_watcher_close((rv_watcher_t *) timer);
}

void _rv_run_timers(rv_ctx_t *ctx) {
  _rv_update_time(ctx);
  QUEUE *q;
  rv_timer_t *t;
  QUEUE_FOREACH(q, &ctx->timer_queue) {
    t = QUEUE_DATA(q, rv_timer_t, node);
    if (t->state != RV_STATE_RUNNING) {
      continue;
    }
    if (t->timeout > ctx->time) {
      break;
    }
    if (t->repeat > 0) {
      t->timeout = ctx->time + t->repeat;
    } else {
      rv_timer_close(t);
    }
    t->cb(t);
  }
}
