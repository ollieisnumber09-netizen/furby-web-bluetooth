// Times the slow parts: loading a level, drawing a frame, a second of sound.
#include "../src/game.c"
#include <time.h>
void plat_sfx(int id) { synth_trigger(id); }
static uint32_t fb[FB_W * FB_H];
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
int main(void) {
  double t0 = now(); game_init(); printf("init %.1f ms\n", now() - t0);
  int lv[4] = { 0, 3, 6, 9 };
  for (int k = 0; k < 4; k++) {
    t0 = now(); start_level(lv[k]); update_camera(1); game_draw(fb); double tl = now() - t0;
    Input in = { 0 }; in.right = 1;
    double tu = 0, td = 0;
    for (int f = 0; f < 300; f++) { in.jump = (f % 50) < 18; lives = 9; double a = now(); game_update(&in); double b = now(); game_draw(fb); td += now() - b; tu += b - a; }
    printf("level %d: load+first frame %.1f ms, update %.3f ms, draw %.3f ms per frame\n", lv[k] + 1, tl, tu / 300, td / 300);
  }
  state = ST_MAP; map_t = 99; t0 = now(); for (int f = 0; f < 100; f++) { frame++; game_draw(fb); } printf("map draw %.3f ms\n", (now() - t0) / 100);
  state = ST_TITLE; t0 = now(); for (int f = 0; f < 100; f++) { frame++; state_t++; game_draw(fb); } printf("title draw %.3f ms\n", (now() - t0) / 100);
  static int16_t snd[48000 * 2];
  state = ST_PLAY; t0 = now(); for (int i = 0; i < 5; i++) synth_render(snd, 48000, 48000); printf("sound %.1f ms per second of music\n", (now() - t0) / 5);
  return 0;
}
