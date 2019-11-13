#include "rtev.h"

static void _rtev_worker_on_async(rtev_async_t *async) {
  QUEUE wq;
  pthread_mutex_lock(&async->ctx->worker_done_lock);
  QUEUE_MOVE(&async->ctx->worker_done_queue,&wq);
  pthread_mutex_lock(&async->ctx->worker_lock);
  if (async->ctx->worker_count == 0) {
    rtev_async_close(async->ctx->worker_async);
  }
  pthread_mutex_unlock(&async->ctx->worker_lock);
  pthread_mutex_unlock(&async->ctx->worker_done_lock);
  QUEUE *q;
  rtev_worker_t *worker;
  while (!QUEUE_EMPTY(&wq)) {
    q = QUEUE_HEAD(&wq);
    QUEUE_REMOVE(q);
    QUEUE_INIT(q);
    worker = QUEUE_DATA(q, rtev_worker_t, node);
    if (worker->state != RTEV_STATE_RUNNING) {
      continue;
    }
    if (worker->done_cb) {
      worker->done_cb(worker);
    }
    rtev_worker_close(worker);
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
    pthread_mutex_lock(&ctx->worker_done_lock);
    QUEUE_INSERT_TAIL(&ctx->worker_done_queue, &worker->node);
    pthread_mutex_unlock(&ctx->worker_done_lock);
    pthread_mutex_lock(&ctx->worker_lock);
    RTEV_ASSERT(ctx->worker_count > 0, "invalid worker count");
    --ctx->worker_count;
    rtev_async_send(ctx->worker_async);
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
  pthread_mutex_lock(&ctx->worker_lock);
  pthread_cond_signal(&ctx->worker_cond);
  pthread_mutex_unlock(&ctx->worker_lock);
}

int rtev_worker_start(rtev_ctx_t *ctx, rtev_worker_t *worker, rtev_worker_cb cb,
                      rtev_worker_cb  done_cb, rtev_close_cb close_cb) {
  int r;
  r = rtev_async_start(ctx, ctx->worker_async, _rtev_worker_on_async, NULL);
  if (r != 0 && r != EEXIST) {
    return r;
  }
  rtev_watcher_t *w = (rtev_watcher_t *) worker;
  r = _rtev_watcher_init(ctx, w, RTEV_TYPE_WORKER, close_cb);
  if (r != 0) {
    return r;
  }
  worker->cb = cb;
  worker->done_cb = done_cb;
  worker->pending = 0;
  return _rtev_watcher_pending(w);
}

int rtev_worker_close(rtev_worker_t *worker) {
  return _rtev_watcher_close((rtev_watcher_t *) worker);
}
