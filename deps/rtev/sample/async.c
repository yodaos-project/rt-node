#include "rtev.h"

void _rtev_worker_on_async(rtev_async_t *async) {
  printf("on async, %u\n", (uint32_t) pthread_self());
  rtev_async_close(async);
}

void on_close(rtev_watcher_t *watcher) {
  printf("watcher closed, %u\n", (uint32_t) pthread_self());
}

void* on_thread(void *data) {
  printf("thread start, %u\n", (uint32_t) pthread_self());
  rtev_async_t *async = (rtev_async_t *) data;
  sleep(1);
  rtev_async_send(async);
  printf("thread end %u\n", (uint32_t) pthread_self());
  return NULL;
}

int main(int argc, char **argv) {
  printf("main thread start %u\n", (uint32_t) pthread_self());
  rtev_ctx_t ctx;
  rtev_async_t async;
  rtev_ctx_init(&ctx);
  rtev_async_start(&ctx, &async, _rtev_worker_on_async, on_close);
  pthread_t thread;
  pthread_create(&thread, NULL, on_thread, &async);
  pthread_detach(thread);
  rtev_ctx_loop(&ctx, RTEV_RUN_DEFAULT);
  printf("main thread end, %u\n", (uint32_t) pthread_self());
  return 0;
}
