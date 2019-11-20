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

int main(int argc, char **argv) {
  printf("main thread start\n");
  rv_ctx_t ctx;
  rv_timer_t timer;
  rv_ctx_init(&ctx);
  rv_timer_start(&ctx, &timer, 2000, 1000, on_timer, on_close);
  rv_ctx_loop(&ctx, RV_RUN_DEFAULT);
  printf("main thread end\n");
  return 0;
}
