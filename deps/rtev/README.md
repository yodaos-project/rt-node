## Introduction

`rtev` is a tiny event loop library which support the following features:
- Timer with a precision of milliseconds
- Message queue for multi thread sync
- Thread pool for thread operations

## Pthread Dependencies

- pthread_create
- pthread_detach
- pthread_mutex_lock
- pthread_mutex_unlock
- pthread_cond_signal
- pthread_cond_timedwait

Since `rtev` is designed for RTOS, I/O operations are not considered, but I/O through thread pool is a good choice.

## Example

The following example shows how to use timer to print 3 times:

```c
#include "rtev.h"

int i = 0;
void on_timer(rtev_timer_t *timer) {
  printf("on timer\n");
  if (++i >= 3) {
    rtev_timer_close(timer);
  }
}

void on_close(rtev_watcher_t *watcher) {
  printf("timer closed\n");
}

int main(int argc, char **argv) {
  rtev_ctx_t ctx;
  rtev_timer_t timer;
  rtev_timer_start(&ctx, &timer, 1000, 1000, on_timer, on_close);
  rtev_ctx_loop(&ctx, RTEV_RUN_DEFAULT);
  return 0;
}

```

## Build & Install

```shell
$ cmake -B./build -H. -DRTEV_SAMPLE=ON
$ make -C./build
$ ./build/sample
```

## APIs

- context
  - int rtev_ctx_init(rtev_ctx_t *ctx);
  - int rtev_ctx_loop(rtev_ctx_t *ctx, rtev_run_type_t type);
- memory allocator
  - int rtev_set_allocator(void* (*malloc_fn)(size_t), void (*free_fn)(void *));
  - void* rtev_malloc(size_t size);
  - void rtev_free(void *ptr);
- timer
  - int rtev_timer_start(rtev_ctx_t *ctx, rtev_timer_t *timer, uint64_t timeout, uint64_t repeat, rtev_timer_cb cb, rtev_close_cb close_cb);
  - int rtev_timer_close(rtev_timer_t *timer);
- message loop
  - int rtev_async_start(rtev_ctx_t *ctx, rtev_async_t *async, rtev_async_cb cb, rtev_close_cb close_cb);
  - int rtev_async_send(rtev_async_t *async);
  - int rtev_async_close(rtev_async_t *async);
- thread pool
  - void rtev_threadpool_post(rtev_threadpool_fn fn, void *data);
- tick
  - int rtev_tick_start(rtev_ctx_t *ctx, rtev_tick_t *tick, rtev_tick_cb cb, rtev_close_cb close_cb);
  - int rtev_tick_close(rtev_tick_t *tick);
