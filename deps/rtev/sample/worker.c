#include <atomic-ops.h>
#include "rtev.h"

int count = 0;
pthread_mutex_t lock;

void on_work(rtev_worker_t *worker) {
  pthread_mutex_lock(&lock);
  ++count;
  pthread_mutex_unlock(&lock);
  printf("%d work, %u\n", (int) (intptr_t) worker->data, (uint32_t) pthread_self());
  sleep(2);
  printf("%d worked, %u\n", (int) (intptr_t) worker->data, (uint32_t) pthread_self());
}

void on_done(rtev_worker_t *worker) {
  printf("%d done, %u\n", (int) (intptr_t) worker->data, (uint32_t) pthread_self());
  rtev_worker_close(worker);
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
  pthread_mutex_init(&lock, NULL);
  rtev_ctx_t ctx;
  rtev_ctx_init(&ctx);
  int worker_count = 10;
  for (int i = 0; i < worker_count; ++i) {
    rtev_worker_t *worker = (rtev_worker_t *) rtev_malloc(sizeof(rtev_worker_t));
    worker->data = (void *) (intptr_t) i;
    rtev_worker_start(&ctx, worker, on_work, on_done, on_close);
  }
  rtev_ctx_loop(&ctx, RTEV_RUN_DEFAULT);
  printf("main thread end, %u\n", (uint32_t) pthread_self());
  RTEV_ASSERT(worker_count == count, "");
  return 0;
}
