#include "rv.h"

static void _rv_worker_async_on_done(rv_async_t *async) {
  rv_worker_t *worker = (rv_worker_t *) async->data;
  worker->done_cb(worker);
}

static void _rv_worker_async_on_close(rv_watcher_t *watcher) {
  rv_free(watcher);
}

static void _rv_worker_close(rv_watcher_t *watcher) {
  rv_worker_t *worker = (rv_worker_t *) watcher;
  rv_async_close(worker->async);
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
    worker = QUEUE_DATA(head, rv_worker_t, node);   \
  }

static void* _rv_worker_run(void *data) {
  rv_ctx_t *ctx = (rv_ctx_t *) data;
  rv_worker_t *worker;
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
    rv_async_send(worker->async);
    pthread_mutex_lock(&ctx->worker_lock);
    RV_ASSERT(ctx->worker_count > 0, "invalid worker count");
    --ctx->worker_count;
  }
  return NULL;
}

#undef NEW_TASK

void _rv_threadpool_init(rv_ctx_t *ctx) {
  for (int i = 0; i < RV_THREADPOOL_SIZE; ++i) {
    pthread_t worker_thread;
    pthread_create(&worker_thread, NULL, _rv_worker_run, ctx);
    pthread_detach(worker_thread);
  }
}

void _rv_threadpool_stop(rv_ctx_t *ctx) {
  RV_ASSERT(!ctx->is_running, "ctx is still running");
  pthread_mutex_lock(&ctx->worker_lock);
  pthread_cond_signal(&ctx->worker_cond);
  pthread_mutex_unlock(&ctx->worker_lock);
}

int rv_worker_start(rv_ctx_t *ctx, rv_worker_t *worker, rv_worker_cb cb,
                      rv_worker_cb  done_cb, rv_close_cb close_cb) {
  rv_watcher_t *w = (rv_watcher_t *) worker;
  int r = _rv_watcher_init(ctx, w, RV_TYPE_WORKER, _rv_worker_close);
  if (r != 0) {
    return r;
  }
  worker->user_close_cb = close_cb;
  worker->cb = cb;
  worker->done_cb = done_cb;
  worker->async = (rv_async_t *) rv_malloc(sizeof(rv_async_t));
  worker->async->data = worker;
  rv_async_start(ctx, worker->async, _rv_worker_async_on_done,
    _rv_worker_async_on_close);
  return _rv_watcher_pending(w);
}

int rv_worker_close(rv_worker_t *worker) {
  return _rv_watcher_close((rv_watcher_t *) worker);
}
