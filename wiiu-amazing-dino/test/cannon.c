// Cannons and stages: pictures of a water level's shore, a cannon ride, and a back stage: out/can_N.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
static int shots;
static void save(void) { char nm[64]; snprintf(nm, 64, "out/can_%d.ppm", shots++); game_draw(fb); FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
static void ride(int n, int ci) {
  Input in = { 0 };
  start_level(n); intro_t = 0; lives = 9;
  Cannon* c = &CAN[ci];
  px = (c->tx - 4) * T; py = c->ty * T + T - PH - 2; pvy = 0;
  for (int i = 0; i < ncan; i++) printf("cannon %d at %d,%d lands at %d,%d\n", i, CAN[i].tx, CAN[i].ty, CAN[i].dx, CAN[i].dy);
  for (int f = 0; f < 30; f++) game_update(&in);
  update_camera(1); save();
  in.right = 1; for (int f = 0; f < 200 && !can_t; f++) { if (px > c->tx * T) { in.right = 0; in.down = f & 1; } game_update(&in); }
  printf("ride started %d, stage %d\n", can_t, cur_stg);
  in.right = in.down = 0;
  for (int f = 0; f < 140; f++) { game_update(&in); if (f == 10 || f == 40 || f == 60 || f == 85 || f == 139) save(); }
  printf("after ride: stage %d at %.0f,%.0f, can_t %d, dead %d\n", cur_stg, px / T, py / T, can_t, dead_t);
}
int main(void) {
  game_init();
  ride(3, 0);    // Lagoon Dive: shore to water
  ride(2, 0);    // Misty Falls: front to back
  ride(2, 1);    // and back again
  return 0;
}
