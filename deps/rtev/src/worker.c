#include "rtev.h"

static void _rtev_worker_async_on_done(rtev_async_t *async) {
  rtev_worker_t *worker = (rtev_worker_t *) async->data;
  worker->done_cb(worker);
}

static void _rtev_worker_async_on_close(rtev_watcher_t *watcher) {
  rtev_free(watcher);
}

static void _rtev_worker_close(rtev_watcher_t *watcher) {
  rtev_worker_t *worker = (rtev_worker_t *) watcher;
  rtev_async_close(worker->async);
  if (worker->user_close_cb) {
    worker->user_close_cb(watcher);
  }
}

#define NEW_TASK                                      \
  worker = NULL;                                      \
  if (!QUEUE_EMPTY(&ctx->worker_queue)) {             \
    QUEUE *head = QUEUE_HEAD(&ctx->worker_queue);     \
    QUEUE_REMOVE(head);                               \
    QUEUE_INIT(head);                                 \
    worker = QUEUE_DATA(head, rtev_worker_t, node);   \
  }

static void* _rtev_worker_run(void *data) {
  rtev_ctx_t *ctx = (rtev_ctx_t *) data;
  rtev_worker_t *worker;
  pthread_mutex_lock(&ctx->worker_lock);
  while (true) {
    NEW_TASK;
    if (worker == NULL) {
      pthread_cond_wait(&ctx->worker_cond, &ctx->worker_lock);
      if (ctx->is_running == false) {
        pthread_cond_signal(&ctx->worker_cond);
        pthread_mutex_unlock(&ctx->worker_lock);
        break;
      }
      NEW_TASK;
    }
    pthread_mutex_unlock(&ctx->worker_lock);
    if (worker == NULL) {
      continue;
    }
    worker->cb(worker);
    rtev_async_send(worker->async);
    pthread_mutex_lock(&ctx->worker_lock);
    RTEV_ASSERT(ctx->worker_count > 0, "invalid worker count");
    --ctx->worker_count;
  }
  return NULL;
}

#undef NEW_TASK

void _rtev_threadpool_init(rtev_ctx_t *ctx) {
  for (int i = 0; i < RTEV_THREADPOOL_SIZE; ++i) {
    pthread_t worker_thread;
    pthread_create(&worker_thread, NULL, _rtev_worker_run, ctx);
    pthread_detach(worker_thread);
  }
}

void _rtev_threadpool_stop(rtev_ctx_t *ctx) {
  RTEV_ASSERT(!ctx->is_running, "ctx is still running");
  pthread_mutex_lock(&ctx->worker_lock);
  pthread_cond_signal(&ctx->worker_cond);
  pthread_mutex_unlock(&ctx->worker_lock);
}

int rtev_worker_start(rtev_ctx_t *ctx, rtev_worker_t *worker, rtev_worker_cb cb,
                      rtev_worker_cb  done_cb, rtev_close_cb close_cb) {
  rtev_watcher_t *w = (rtev_watcher_t *) worker;
  int r = _rtev_watcher_init(ctx, w, RTEV_TYPE_WORKER, _rtev_worker_close);
  if (r != 0) {
    return r;
  }
  worker->user_close_cb = close_cb;
  worker->cb = cb;
  worker->done_cb = done_cb;
  worker->async = (rtev_async_t *) rtev_malloc(sizeof(rtev_async_t));
  worker->async->data = worker;
  rtev_async_start(ctx, worker->async, _rtev_worker_async_on_done,
    _rtev_worker_async_on_close);
  return _rtev_watcher_pending(w);
}

int rtev_worker_close(rtev_worker_t *worker) {
  return _rtev_watcher_close((rtev_watcher_t *) worker);
}
