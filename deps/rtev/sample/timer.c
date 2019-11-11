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

int main(int argc, char **argv) {
  printf("main thread start\n");
  rtev_ctx_t ctx;
  rtev_timer_t timer;
  rtev_ctx_init(&ctx);
  rtev_timer_start(&ctx, &timer, 2000, 1000, on_timer, on_close);
  rtev_ctx_loop(&ctx, RTEV_RUN_DEFAULT);
  printf("main thread end\n");
  return 0;
}
