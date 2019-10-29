#include "rtev.h"

int rtev_timer_start(rtev_ctx_t *ctx, rtev_timer_t *timer, uint64_t timeout,
  uint64_t repeat, rtev_timer_cb cb) {
  int r = _rtev_watcher_init(ctx, (rtev_watcher_t *) timer, RTEV_TYPE_TIMER);
  if (r != 0) {
    return r;
  }
  timer->timeout = ctx->time + timeout;
  timer->repeat = repeat;
  timer->cb = cb;

  return _rtev_watcher_start((rtev_watcher_t *) timer);
}

int rtev_timer_stop(rtev_timer_t *timer) {
  return _rtev_watcher_stop((rtev_watcher_t *) timer);
}

int rtev_timer_close(rtev_timer_t *timer) {
  return _rtev_watcher_close((rtev_watcher_t *) timer);
}

static void rtev_update_time(rtev_ctx_t *ctx) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  ctx->time = ((uint64_t) ts.tv_sec) * 1000 + ((uint64_t) ts.tv_nsec) / 1000000;
}

void _rtev_run_timers(rtev_ctx_t *ctx) {
  rtev_update_time(ctx);
  QUEUE *q;
  rtev_timer_t *t;
  QUEUE_FOREACH(q, &ctx->timer_queue) {
    t = QUEUE_DATA(q, rtev_timer_t, node);
    if (t->state != RTEV_STATE_RUNNING) {
      continue;
    }
    if (t->repeat > 1) {
      t->timeout = t->repeat + ctx->time;
    } else {
      rtev_timer_stop(t);
    }
    if (t->timeout >= ctx->time) {
      t->cb(t);
    }
  }
}
