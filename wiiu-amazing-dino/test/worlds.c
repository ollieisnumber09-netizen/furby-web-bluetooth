// The world select and the trip between worlds: out/wsel_N.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
static int shots;
static void save(void) { char nm[64]; snprintf(nm, 64, "out/wsel_%d.ppm", shots++); game_draw(fb); FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
int main(void) {
  game_init();
  Input in = { 0 };
  for (int i = 1; i <= 24; i++) cleared[i] = 1;
  state = ST_TITLE; state_t = 30; in.jump = 1; game_update(&in); in.jump = 0; game_update(&in);
  printf("after title: state %d (worlds %d)\n", state, ST_WORLDS);
  for (int f = 0; f < 40; f++) game_update(&in);
  save();
  in.right = 1; game_update(&in); in.right = 0; for (int f = 0; f < 6; f++) game_update(&in); save();
  for (int k = 0; k < 3; k++) { in.right = 1; game_update(&in); in.right = 0; game_update(&in); }
  for (int f = 0; f < 40; f++) game_update(&in);
  save();   // a locked world
  for (int k = 0; k < 2; k++) { in.left = 1; game_update(&in); in.left = 0; game_update(&in); }
  for (int f = 0; f < 40; f++) game_update(&in);
  in.jump = 1; game_update(&in); in.jump = 0;
  printf("picked world %d: state %d (travel %d)\n", wsel, state, ST_TRAVEL);
  for (int f = 0; f < 230; f++) { game_update(&in); if (f == 30 || f == 100 || f == 170) save(); }
  printf("arrived: state %d world %d\n", state, world);
  return 0;
}
