#include "../src/game.c"
#include <time.h>
void plat_sfx(int id) { synth_trigger(id); }
static uint32_t fb[FB_W * FB_H];
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
#define TIME(name, code) { double a = now(); for (int i = 0; i < 200; i++) { code; } printf("%-14s %.3f ms\n", name, (now() - a) / 200); }
int main(void) {
  game_init(); start_level(6); update_camera(1); FB = fb;
  camx = 600; px = 700;
  TIME("background", hd_background());
  TIME("level blit", blit_level((int)(camx * 2), (int)(camy * 2)));
  DinoPose p; dino_pose(&p);
  TIME("dino 3d", dino3d(&DSCR, &p, 0.75f, 1));
  TIME("dino small", dino3d(&DSCR, &p, 0.75f, 0.72f));
  TIME("beetle 3d", beetle3d(&ESCR, 1, 1, 1.15f, 0, 0, 5, 1, 0));
  TIME("bee 3d", bee3d(&ESCR, 1.0f, 0.1f, 1.15f, 0, 1, 0));
  TIME("boss 3d", beetle3d(&ESCR, 1, 1, 1.15f, 2, 0, 5, 2.4f, 1));
  TIME("hud", draw_hud());
  TIME("world", draw_world());
  double a = now(); hd_render_level(); printf("render level %.1f ms\n", now() - a);
  a = now(); bg_theme = -1; hd_render_bg(); printf("render bg %.1f ms\n", now() - a);
  a = now(); m3d_build(); printf("m3d build %.1f ms\n", now() - a);
  return 0;
}
