#include <atomic-ops.h>
#include "rtev.h"

void on_work(rtev_worker_t *async) {
  printf("%d work, %u\n", (int) (intptr_t) async->data, (uint32_t) pthread_self());
  sleep(2);
  printf("%d worked, %u\n", (int) (intptr_t) async->data, (uint32_t) pthread_self());
}

void on_done(rtev_worker_t *watcher) {
  printf("%d done, %u\n", (int) (intptr_t) watcher->data, (uint32_t) pthread_self());
}

void on_close(rtev_watcher_t *watcher) {
  printf("%d closed, %u\n", (int) (intptr_t) watcher->data, (uint32_t) pthread_self());
  rtev_free(watcher);
}

void* on_thread(void *data) {
  printf("thread start, %u\n", (uint32_t) pthread_self());
  rtev_async_t *async = (rtev_async_t *) data;
  sleep(3);
  rtev_async_send(async);
  printf("thread end %u\n", (uint32_t) pthread_self());
  return NULL;
}

int main(int argc, char **argv) {
  printf("main thread start %u\n", (uint32_t) pthread_self());
  rtev_ctx_t ctx;
  rtev_ctx_init(&ctx);
  for (int i = 0; i < 2; ++i) {
    rtev_worker_t *worker = (rtev_worker_t *) rtev_malloc(sizeof(rtev_worker_t));
    worker->data = (void *) (intptr_t) i;
    rtev_worker_start(&ctx, worker, on_work, on_done, on_close);
  }
  rtev_ctx_loop(&ctx, RTEV_RUN_DEFAULT);
  printf("main thread end, %u\n", (uint32_t) pthread_self());
  return 0;
}
