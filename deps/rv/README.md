## Introduction

`rv` is a tiny event loop library which support the following features:
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

Since `rv` is designed for RTOS, I/O operations are not considered, but I/O through thread pool is a good choice.

## Example

The following example shows how to use timer to print 3 times:

```c
#include "rv.h"

int i = 0;
void on_timer(rv_timer_t *timer) {
  printf("on timer\n");
  if (++i >= 3) {
    rv_timer_close(timer);
  }
}

void on_close(rv_watcher_t *watcher) {
  printf("timer closed\n");
}

int main(int argc, char **argv) {
  rv_ctx_t ctx;
  rv_timer_t timer;
  rv_timer_start(&ctx, &timer, 1000, 1000, on_timer, on_close);
  rv_ctx_loop(&ctx, RV_RUN_DEFAULT);
  return 0;
}

```

## Build & Install

```shell
$ cmake -B./build -H. -DRV_SAMPLE=ON
$ make -C./build
$ ./build/sample
```

## APIs

- context
  - int rv_ctx_init(rv_ctx_t *ctx);
  - int rv_ctx_loop(rv_ctx_t *ctx, rv_run_type_t type);
- memory allocator
  - int rv_set_allocator(void* (*malloc_fn)(size_t), void (*free_fn)(void *));
  - void* rv_malloc(size_t size);
  - void rv_free(void *ptr);
- timer
  - int rv_timer_start(rv_ctx_t *ctx, rv_timer_t *timer, uint64_t timeout, uint64_t repeat, rv_timer_cb cb, rv_close_cb close_cb);
  - int rv_timer_close(rv_timer_t *timer);
- message loop
  - int rv_async_start(rv_ctx_t *ctx, rv_async_t *async, rv_async_cb cb, rv_close_cb close_cb);
  - int rv_async_send(rv_async_t *async);
  - int rv_async_close(rv_async_t *async);
- thread pool
  - void rv_threadpool_post(rv_threadpool_fn fn, void *data);
- tick
  - int rv_tick_start(rv_ctx_t *ctx, rv_tick_t *tick, rv_tick_cb cb, rv_close_cb close_cb);
  - int rv_tick_close(rv_tick_t *tick);
