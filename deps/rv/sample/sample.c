#include "rv.h"

int i = 0;
void on_timer(rv_timer_t *timer) {
  printf("on timer\n");
  if (++i >= 3) {
    rv_timer_close(timer);
  }
}

void on_close(rv_watcher_t *watcher) {
  printf("watcher closed\n");
}

void on_thread_fn(void *data) {
  printf("threadpool fn start\n");
  sleep(1);
  rv_async_send((rv_async_t *) data);
  printf("threadpool fn finish\n");
}

void on_async_cb(rv_async_t *async) {
  printf("async cb in main thread\n");
  rv_async_close(async);
}

void on_tick_cb(rv_tick_t *tick) {
  int *times = (int*) &tick->data;
  ++*times;
  printf("event loop tick %d times\n", *times);
  if (*times >= 8) {
    rv_tick_close(tick);
  }
}

void on_worker(rv_worker_t *worker) {
  printf("on worker thread\n");
}

void on_worker_done(rv_worker_t *worker) {
  printf("worker done in main thread\n");
  rv_worker_close(worker);
}

void* on_thread(void *data) {
  rv_async_t *async = (rv_async_t *) data;
  printf("on thread\n");
  rv_async_send(async);
  return NULL;
}

int main(int argc, char **argv) {
  printf("main thread start\n");
  rv_ctx_t ctx;
  rv_timer_t timer;
  rv_async_t async;
  rv_tick_t tick;
  rv_worker_t worker;
  tick.data = 0;
  rv_ctx_init(&ctx);
  rv_timer_start(&ctx, &timer, 2000, 1000, on_timer, on_close);
  rv_async_start(&ctx, &async, on_async_cb, on_close);
  rv_tick_start(&ctx, &tick, on_tick_cb, on_close);
  rv_worker_start(&ctx, &worker, on_worker, on_worker_done, on_close);
  pthread_t thread;
  pthread_create(&thread, NULL, on_thread, &async);
  pthread_detach(thread);
  rv_ctx_loop(&ctx, RV_RUN_DEFAULT);
  printf("main thread end\n");
  return 0;
}
