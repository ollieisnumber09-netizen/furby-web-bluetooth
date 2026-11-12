// Pictures of the level intro and the game over screen: out/s2_N.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H]; static int nshot;
static void save(void) { char nm[64]; snprintf(nm, 64, "out/s2_%d.ppm", nshot++); game_draw(fb); FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
int main(void) {
  game_init();
  Input in = { 0 };
  lives = 3; bananas = 7; start_level(0); update_camera(1);
  for (int f = 1; f <= 150; f++) { game_update(&in); if (f == 40 || f == 105 || f == 125 || f == 149) save(); }
  printf("intro done: %d\n", intro_t);
  state = ST_OVER; state_t = 0;
  for (int f = 1; f <= 230; f++) { in.right = f == 200; game_update(&in); if (f == 20 || f == 70 || f == 92 || f == 112 || f == 150 || f == 210) save(); }
  in.right = 0; in.jump = 1; game_update(&in); printf("after pick: state %d\n", state);
  return 0;
}
