#include "rtev.h"
#include "atomic-ops.h"

void _rtev_update_time(rtev_ctx_t *ctx) {
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);
  ctx->time = spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}

int rtev_timer_start(rtev_ctx_t *ctx, rtev_timer_t *timer, uint64_t timeout,
  uint64_t repeat, rtev_timer_cb cb, rtev_close_cb close_cb) {
  rtev_watcher_t *w = (rtev_watcher_t *) timer;
  int r = _rtev_watcher_init(ctx, w, RTEV_TYPE_TIMER, close_cb);
  if (r != 0) {
    return r;
  }
  timer->timeout = ctx->time + timeout;
  timer->repeat = repeat;
  timer->cb = cb;

  return _rtev_watcher_start(w);
}

int rtev_timer_close(rtev_timer_t *timer) {
  return _rtev_watcher_close((rtev_watcher_t *) timer);
}

void _rtev_set_next_timeout(rtev_ctx_t *ctx, struct timespec *spec) {
  spec->tv_sec = 0;
  spec->tv_nsec = 0;
  // handle async events asap
  if (cmpxchgi(&ctx->async_pending, 1, 0) == 1) {
    return;
  }
#if !(defined(__APPLE__) && defined(__MACH__))
  clock_gettime(CLOCK_MONOTONIC, spec);
#endif
  QUEUE *q;
  rtev_timer_t *timer = NULL;
  QUEUE_FOREACH(q, &ctx->timer_queue) {
    rtev_timer_t *t = QUEUE_DATA(q, rtev_timer_t, node);
    if (t->state == RTEV_STATE_RUNNING) {
      RTEV_ASSERT(t->timeout >= ctx->time, "unexpected timeout");
      timer = t;
      break;
    }
  }
  uint64_t next_timeout = timer ? timer->timeout - ctx->time : 0;
  spec->tv_sec += next_timeout / 1000;
  spec->tv_nsec += next_timeout % 1000 * 1000000;
}

void _rtev_run_timers(rtev_ctx_t *ctx) {
  _rtev_update_time(ctx);
  QUEUE *q;
  rtev_timer_t *t;
  QUEUE_FOREACH(q, &ctx->timer_queue) {
    t = QUEUE_DATA(q, rtev_timer_t, node);
    if (t->state != RTEV_STATE_RUNNING) {
      continue;
    }
    if (t->timeout > ctx->time) {
      break;
    }
    if (t->repeat > 0) {
      t->timeout = ctx->time + t->repeat;
    } else {
      rtev_timer_close(t);
    }
    t->cb(t);
  }
}
