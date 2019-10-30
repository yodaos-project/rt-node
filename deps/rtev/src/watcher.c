#include "rtev.h"

static void rtev_watcher_on_close(rtev_watcher_t *watcher) {
  rtev_free(watcher);
}

int _rtev_watcher_init(rtev_ctx_t *ctx, rtev_watcher_t *watcher,
  rtev_watcher_type_t type) {
  RTEV_ASSERT(ctx != NULL, "ctx is null");
  RTEV_ASSERT(watcher != NULL, "watcher is null");
  watcher->ctx= ctx;
  watcher->state = RTEV_STATE_PENDING;
  watcher->data = NULL;
  watcher->close_cb = rtev_watcher_on_close;
  QUEUE_INIT(&watcher->node);
  watcher->type = type;
  watcher->next_watcher = NULL;
  ++ctx->watcher_count;
  return 0;
}

int _rtev_watcher_start(rtev_watcher_t *watcher) {
  RTEV_ASSERT(watcher->ctx, "watcher is not inited");
  RTEV_ASSERT(watcher->state == RTEV_STATE_PENDING, "watcher is no pending");
  rtev_watcher_t **pending_watcher = &watcher->ctx->pending_watchers;
  while (*pending_watcher != NULL) {
    pending_watcher = &(*pending_watcher)->next_watcher;
  }
  *pending_watcher = watcher;
  ++watcher->ctx->watcher_count;

  return 0;
}

int _rtev_watcher_stop(rtev_watcher_t *watcher) {
  watcher->state = RTEV_STATE_STOPPED;
  return 0;
}

int _rtev_watcher_close(rtev_watcher_t *watcher) {
  RTEV_ASSERT(watcher->state == RTEV_STATE_RUNNING, "watcher is not running");
  watcher->state = RTEV_STATE_CLOSING;
  rtev_watcher_t **last_watcher = &watcher->ctx->closing_watchers;
  while (*last_watcher != NULL) {
    last_watcher = &(*last_watcher)->next_watcher;
  }
  *last_watcher = watcher;
  return 0;
}

void _rtev_add_pending_watchers(rtev_ctx_t *ctx) {
  rtev_watcher_t *watcher = ctx->pending_watchers;
  while (watcher) {
    RTEV_ASSERT(watcher->state = RTEV_STATE_PENDING, "watcher is not pending");
    watcher->state = RTEV_STATE_RUNNING;
    if (watcher->type == RTEV_TYPE_TIMER) {
      QUEUE *q;
      rtev_timer_t *pt = (rtev_timer_t *) watcher;
      rtev_timer_t *t;
      if (QUEUE_EMPTY(&ctx->timer_queue)) {
        QUEUE_INSERT_HEAD(&ctx->timer_queue, &watcher->node);
      } else {
        QUEUE_FOREACH(q, &ctx->timer_queue) {
          t = QUEUE_DATA(q, rtev_timer_t, node);
          if (t->timeout > pt->timeout) {
            QUEUE_INSERT_HEAD(&t->node, &watcher->node);
            break;
          }
        }
      }
    } else if (watcher->type == RTEV_TYPE_ASYNC) {
      QUEUE_INSERT_TAIL(&ctx->async_queue, &watcher->node);
    } else {
      RTEV_ASSERT(0, "unknown watcher type");
    }
    watcher = watcher->next_watcher;
  }
  ctx->pending_watchers = NULL;
}

void _rtev_close_watchers(rtev_ctx_t *ctx) {
  rtev_watcher_t *watcher;
  while (ctx->closing_watchers) {
    watcher = ctx->closing_watchers;
    RTEV_ASSERT(watcher->state == RTEV_STATE_CLOSING, "watcher is not closing");
    ctx->closing_watchers = watcher->next_watcher;
    QUEUE_REMOVE(&watcher->node);
    QUEUE_INIT(&watcher->node);
    watcher->state = RTEV_STATE_CLOSED;
    --ctx->watcher_count;
    if (watcher->close_cb) {
      watcher->close_cb(watcher);
    }
  }
  ctx->closing_watchers = NULL;
}
