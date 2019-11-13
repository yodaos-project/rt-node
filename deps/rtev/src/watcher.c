#include "rtev.h"

static void _rtev_remove_watcher_from_link(rtev_watcher_t *watcher,
                                           rtev_watcher_t **head) {
  while (*head) {
    if ((*head) == watcher) {
      *head = NULL;
      break;
    }
    if ((*head)->next_watcher == watcher) {
      (*head)->next_watcher = watcher->next_watcher;
      break;
    }
    head = &(*head)->next_watcher;
  }
  watcher->next_watcher = NULL;
}

int _rtev_watcher_init(rtev_ctx_t *ctx, rtev_watcher_t *watcher,
  rtev_watcher_type_t type, rtev_close_cb close_cb) {
  if (watcher->state == RTEV_STATE_PENDING
    || watcher->state == RTEV_STATE_RUNNING) {
    return EEXIST;
  }
  if (watcher->state == RTEV_STATE_CLOSING) {
    _rtev_remove_watcher_from_link(watcher, &watcher->ctx->closing_watchers);
  }
  watcher->ctx= ctx;
  watcher->state = RTEV_STATE_PENDING;
  watcher->close_cb = close_cb;
  QUEUE_INIT(&watcher->node);
  watcher->type = type;
  watcher->next_watcher = NULL;
  return 0;
}

int _rtev_watcher_pending(rtev_watcher_t *watcher) {
  RTEV_ASSERT(watcher->ctx, "watcher is not inited");
  RTEV_ASSERT(watcher->state == RTEV_STATE_PENDING, "watcher is not pending");
  rtev_watcher_t **pending_watcher = &watcher->ctx->pending_watchers;
  while (*pending_watcher) {
    RTEV_ASSERT(*pending_watcher != watcher, "duplicate pending watcher");
    pending_watcher = &(*pending_watcher)->next_watcher;
  }
  *pending_watcher = watcher;
  ++watcher->ctx->watcher_count;

  return 0;
}

int _rtev_watcher_close(rtev_watcher_t *watcher) {
  RTEV_ASSERT(watcher->state == RTEV_STATE_PENDING ||
    watcher->state == RTEV_STATE_RUNNING, "watcher is not valid");
  if (watcher->state == RTEV_STATE_PENDING) {
    _rtev_remove_watcher_from_link(watcher, &watcher->ctx->pending_watchers);
  }
  watcher->state = RTEV_STATE_CLOSING;
  rtev_watcher_t **last_watcher = &watcher->ctx->closing_watchers;
  while (*last_watcher) {
    RTEV_ASSERT(*last_watcher != watcher, "duplicate close watcher");
    last_watcher = &(*last_watcher)->next_watcher;
  }
  *last_watcher = watcher;
  return 0;
}

void _rtev_add_pending_watchers(rtev_ctx_t *ctx) {
  rtev_watcher_t *watcher;
  while (ctx->pending_watchers) {
    watcher = ctx->pending_watchers;
    ctx->pending_watchers = watcher->next_watcher;
    watcher->next_watcher = NULL;
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
    } else if (watcher->type == RTEV_TYPE_TICK) {
      QUEUE_INSERT_TAIL(&ctx->tick_queue, &watcher->node);
    } else if (watcher->type == RTEV_TYPE_WORKER) {
      pthread_mutex_lock(&ctx->worker_lock);
      ++ctx->worker_count;
      QUEUE_INIT(&watcher->node);
      QUEUE_INSERT_TAIL(&ctx->worker_queue, &watcher->node);
      pthread_cond_signal(&ctx->worker_cond);
      pthread_mutex_unlock(&ctx->worker_lock);
    } else {
      RTEV_ASSERT(0, "unknown watcher type");
    }
  }
}

void _rtev_close_watchers(rtev_ctx_t *ctx) {
  rtev_watcher_t *watcher;
  while (ctx->closing_watchers) {
    watcher = ctx->closing_watchers;
    ctx->closing_watchers = watcher->next_watcher;
    watcher->next_watcher = NULL;
    RTEV_ASSERT(watcher->state == RTEV_STATE_CLOSING, "watcher is not closing");
    QUEUE_REMOVE(&watcher->node);
    QUEUE_INIT(&watcher->node);
    watcher->state = RTEV_STATE_CLOSED;
    RTEV_ASSERT(ctx->watcher_count > 0, "invalid watcher count");
    --ctx->watcher_count;
    if (watcher->close_cb) {
      watcher->close_cb(watcher);
    }
  }
}

int rtev_watcher_close(rtev_watcher_t *watcher) {
  switch (watcher->type) {
  case RTEV_TYPE_TIMER:
    return rtev_timer_close((rtev_timer_t *) watcher);
  case RTEV_TYPE_ASYNC:
    return rtev_async_close((rtev_async_t *) watcher);
  case RTEV_TYPE_TICK:
    return rtev_tick_close((rtev_tick_t *) watcher);
  default:
    RTEV_ASSERT(0, "unknown watcher type");
  }
  return EINVAL;
}
