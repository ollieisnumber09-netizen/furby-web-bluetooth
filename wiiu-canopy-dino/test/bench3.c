#include "../src/game.c"
#include <time.h>
void plat_sfx(int id) { synth_trigger(id); }
static uint32_t fb[FB_W * FB_H];
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
#define TIME(name, code) { double a = now(); for (int i = 0; i < 500; i++) { code; } printf("%-22s %.3f ms\n", name, (now() - a) / 500); }
int main(void) {
  game_init(); start_level(0); update_camera(1); FB = fb;
  DinoPose p; dino_pose(&p);
  TIME("dino ss2", dino3d(&DSCR, &p, 0.75f, 1));
  RT_SS = 1; TIME("dino ss1", dino3d(&DSCR, &p, 0.75f, 1)); RT_SS = 2;
  TIME("begin+end only", (rt_begin(72, 72, 36, 64, 1), rt_end(&DSCR)));
  TIME("hblit dino", hblit(&DSCR, 100, 100, 0));
  TIME("hud", draw_hud());
  TIME("world", draw_world());
  return 0;
}
