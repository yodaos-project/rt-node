#include "rv.h"

void _rv_worker_on_async(rv_async_t *async) {
  printf("on async, %u\n", (uint32_t) pthread_self());
  rv_async_close(async);
}

void on_close(rv_watcher_t *watcher) {
  printf("watcher closed, %u\n", (uint32_t) pthread_self());
}

void* on_thread(void *data) {
  printf("thread start, %u\n", (uint32_t) pthread_self());
  rv_async_t *async = (rv_async_t *) data;
  sleep(1);
  rv_async_send(async);
  printf("thread end %u\n", (uint32_t) pthread_self());
  return NULL;
}

int main(int argc, char **argv) {
  printf("main thread start %u\n", (uint32_t) pthread_self());
  rv_ctx_t ctx;
  rv_async_t async;
  rv_ctx_init(&ctx);
  rv_async_start(&ctx, &async, _rv_worker_on_async, on_close);
  pthread_t thread;
  pthread_create(&thread, NULL, on_thread, &async);
  pthread_detach(thread);
  rv_ctx_loop(&ctx, RV_RUN_DEFAULT);
  printf("main thread end, %u\n", (uint32_t) pthread_self());
  return 0;
}
