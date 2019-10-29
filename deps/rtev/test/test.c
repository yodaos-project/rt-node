#include "rtev.h"

int main(int argc, char **argv) {
  rtev_ctx_t *ctx = malloc(sizeof(rtev_ctx_t));
  rtev_timer_t *timer = malloc(sizeof(rtev_timer_t));
  rtev_timer_start(ctx, timer, 1000, 2000, NULL);
  rtev_loop_init(ctx);
  rtev_loop_run(ctx, RTEV_RUN_DEFAULT);
  free(timer);
  free(ctx);
  return 0;
}