#include "rv.h"

static void _rv_remove_watcher_from_link(rv_watcher_t *watcher,
                                           rv_watcher_t **head) {
  while (*head) {
    if (*head == watcher) {
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

int _rv_watcher_init(rv_ctx_t *ctx, rv_watcher_t *watcher,
                       rv_watcher_type_t type, rv_close_cb close_cb) {
  if (watcher->state == RV_STATE_CLOSING) {
    _rv_remove_watcher_from_link(watcher, &watcher->ctx->closing_watchers);
  }
  watcher->ctx= ctx;
  watcher->state = RV_STATE_PENDING;
  watcher->close_cb = close_cb;
  QUEUE_INIT(&watcher->node);
  watcher->type = type;
  watcher->next_watcher = NULL;
  return 0;
}

int _rv_watcher_pending(rv_watcher_t *watcher) {
  RV_ASSERT(watcher->ctx, "watcher is not inited");
  RV_ASSERT(watcher->state == RV_STATE_PENDING, "watcher is not pending");
  RV_ASSERT(!watcher->next_watcher, "watcher pending link error");
  rv_watcher_t **pending_watcher = &watcher->ctx->pending_watchers;
  while (*pending_watcher) {
    RV_ASSERT(*pending_watcher != watcher, "duplicate pending watcher");
    pending_watcher = &(*pending_watcher)->next_watcher;
  }
  *pending_watcher = watcher;
  ++watcher->ctx->watcher_count;

  return 0;
}

int _rv_watcher_close(rv_watcher_t *watcher) {
  if (watcher->state == RV_STATE_PENDING) {
    _rv_remove_watcher_from_link(watcher, &watcher->ctx->pending_watchers);
  } else if (watcher->state != RV_STATE_RUNNING) {
    RV_ASSERT(0, "watcher state is invalid");
  }
  RV_ASSERT(!watcher->next_watcher, "watcher close link error");
  watcher->state = RV_STATE_CLOSING;
  rv_watcher_t **last_watcher = &watcher->ctx->closing_watchers;
  while (*last_watcher) {
    RV_ASSERT(*last_watcher != watcher, "duplicate close watcher");
    last_watcher = &(*last_watcher)->next_watcher;
  }
  *last_watcher = watcher;
  return 0;
}

void _rv_add_pending_watchers(rv_ctx_t *ctx) {
  rv_watcher_t *watcher;
  while (ctx->pending_watchers) {
    watcher = ctx->pending_watchers;
    ctx->pending_watchers = watcher->next_watcher;
    watcher->next_watcher = NULL;
    RV_ASSERT(watcher->state = RV_STATE_PENDING, "watcher is not pending");
    watcher->state = RV_STATE_RUNNING;
    if (watcher->type == RV_TYPE_TIMER) {
      QUEUE *q;
      rv_timer_t *pt = (rv_timer_t *) watcher;
      rv_timer_t *t;
      if (QUEUE_EMPTY(&ctx->timer_queue)) {
        QUEUE_INSERT_HEAD(&ctx->timer_queue, &watcher->node);
      } else {
        QUEUE_FOREACH(q, &ctx->timer_queue) {
          t = QUEUE_DATA(q, rv_timer_t, node);
          if (t->timeout > pt->timeout) {
            QUEUE_INSERT_HEAD(&t->node, &watcher->node);
            break;
          }
        }
      }
    } else if (watcher->type == RV_TYPE_ASYNC) {
      QUEUE_INSERT_TAIL(&ctx->async_queue, &watcher->node);
    } else if (watcher->type == RV_TYPE_TICK) {
      QUEUE_INSERT_TAIL(&ctx->tick_queue, &watcher->node);
    } else if (watcher->type == RV_TYPE_WORKER) {
      pthread_mutex_lock(&ctx->worker_lock);
      ++ctx->worker_count;
      QUEUE_INIT(&watcher->node);
      QUEUE_INSERT_TAIL(&ctx->worker_queue, &watcher->node);
      pthread_cond_signal(&ctx->worker_cond);
      pthread_mutex_unlock(&ctx->worker_lock);
    } else {
      RV_ASSERT(0, "unknown watcher type");
    }
  }
}

void _rv_close_watchers(rv_ctx_t *ctx) {
  rv_watcher_t *watcher;
  while (ctx->closing_watchers) {
    watcher = ctx->closing_watchers;
    ctx->closing_watchers = watcher->next_watcher;
    watcher->next_watcher = NULL;
    RV_ASSERT(watcher->state == RV_STATE_CLOSING, "watcher is not closing");
    QUEUE_REMOVE(&watcher->node);
    QUEUE_INIT(&watcher->node);
    watcher->state = RV_STATE_CLOSED;
    RV_ASSERT(ctx->watcher_count > 0, "invalid watcher count");
    --ctx->watcher_count;
    if (watcher->close_cb) {
      watcher->close_cb(watcher);
    }
  }
}

int rv_watcher_close(rv_watcher_t *watcher) {
  switch (watcher->type) {
  case RV_TYPE_TIMER:
    return rv_timer_close((rv_timer_t *) watcher);
  case RV_TYPE_ASYNC:
    return rv_async_close((rv_async_t *) watcher);
  case RV_TYPE_TICK:
    return rv_tick_close((rv_tick_t *) watcher);
  case RV_TYPE_WORKER:
    return rv_worker_close((rv_worker_t *) watcher);
  default:
    RV_ASSERT(0, "unknown watcher type");
  }
  return EINVAL;
}
