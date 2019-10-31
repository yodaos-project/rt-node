#include "rtev.h"

int i = 0;
void on_timer(rtev_timer_t *timer) {
  printf("on timer\n");
  if (++i >= 3) {
    rtev_timer_close(timer);
  }
}

void on_close(rtev_watcher_t *watcher) {
  printf("watcher closed\n");
}

void on_thread_fn(void *data) {
  printf("threadpool fn start\n");
  sleep(1);
  rtev_async_send((rtev_async_t *) data);
  printf("threadpool fn finish\n");
}

void on_async_cb(rtev_async_t *async) {
  printf("async cb in main thread\n");
  rtev_async_close(async);
}

void on_tick_cb(rtev_tick_t *tick) {
  int *times = (int*) &tick->data;
  ++*times;
  printf("event loop tick %d times\n", *times);
}

int main(int argc, char **argv) {
  printf("main thread start\n");
  rtev_ctx_t ctx;
  rtev_timer_t timer;
  rtev_async_t async;
  rtev_tick_t tick;
  tick.data = 0;
  rtev_ctx_init(&ctx);
  rtev_timer_start(&ctx, &timer, 2000, 1000, on_timer, on_close);
  rtev_async_start(&ctx, &async, on_async_cb, on_close);
  rtev_tick_start(&ctx, &tick, on_tick_cb, on_close);
  rtev_threadpool_post(on_thread_fn, &async);
  rtev_ctx_run(&ctx, RTEV_RUN_DEFAULT);
  printf("main thread end\n");
  return 0;
}
