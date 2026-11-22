// Where the time goes in one frame, in busy places: prof
#include "../src/game.c"
#include <time.h>
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec * 1e-9; }
int main(void) {
  game_init(); FB = fb;
  static const int SPOT[][3] = { { 6, 150, 14 }, { 20, 200, 14 }, { 2, 120, 16 }, { 4, 200, 12 }, { 30, 260, 14 } };
  Input in = { 0 };
  for (int s = 0; s < 5; s++) {
    start_level(SPOT[s][0] - 1); intro_t = 0; lives = 9;
    px = SPOT[s][1] * T; py = SPOT[s][2] * T; set_stage(stage_of(px));
    for (int f = 0; f < 60; f++) game_update(&in);
    update_camera(1);
    int N = 200; double tg = 0, t0, tb = 0, tl = 0, tw = 0, tall = 0, tu = 0;
    for (int i = 0; i < N; i++) {
      t0 = now(); hd_background(); tb += now() - t0;
      t0 = now(); blit_level((int)floorf(camx * 2), (int)floorf(camy * 2)); tl += now() - t0;
      t0 = now(); game_draw(fb); tall += now() - t0;
      g_gpu = 1; t0 = now(); game_draw(fb); tg += now() - t0; g_gpu = 0;
      t0 = now(); game_update(&in); tu += now() - t0;
    }
    int opaque = 0, part = 0; for (int i = 0; i < FB_W * FB_H; i++) { (void)i; }
    printf("%-22s background %.3f  level picture %.3f  whole frame %.3f  update %.3f  with the chip %.3f ms\n", L->name, tb / N * 1e3, tl / N * 1e3, tall / N * 1e3, tu / N * 1e3, tg / N * 1e3);
  }
}
