// Saves pictures of the newest levels and maps for checking: out/shotN.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
static void save(int k) { char nm[64]; snprintf(nm, 64, "out/shot%d.ppm", k); FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
int main(void) {
  game_init();
  Input in = { 0 };
  for (int k = 0; k < 6; k++) {
    start_level(12 + k); lives = 9;
    px = (k % 2 ? 40 : 150) * T; py = 4 * T; update_camera(1);
    for (int f = 0; f < 50; f++) game_update(&in);
    update_camera(1); game_draw(fb); save(k);
  }
  for (int i = 1; i <= NLEVELS; i++) cleared[i] = i < 15;
  for (int wd = 2; wd < 5; wd++) { state = ST_MAP; world = wd; map_node = map_from = 1; map_t = 99; frame = 40; game_draw(fb); save(6 + wd - 2); }
  state = ST_TITLE; state_t = 10; game_draw(fb); save(9);
  return 0;
}
