// Filmstrip of Dino speeding up to a sprint and of the twirl: out/run.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H], strip[8 * 120 * 3 * 120];
static void grab(int col, int row) { for (int y = 0; y < 120; y++) for (int x = 0; x < 120; x++) { int sx = dino_sx - 60 + x, sy = dino_sy - 90 + y; strip[(row * 120 + y) * 960 + col * 120 + x] = (sx >= 0 && sx < FB_W && sy >= 0 && sy < FB_H) ? fb[sy * FB_W + sx] : 0; } }
int main(void) {
  game_init(); start_level(0); intro_t = 0; Input in = { 0 }; for (int i = 0; i < 30; i++) game_update(&in);
  in.right = 1; for (int i = 0; i < 30; i++) game_update(&in);
  in.run = 1;
  for (int c = 0; c < 8; c++) { for (int i = 0; i < 6; i++) game_update(&in); game_draw(fb); grab(c, 0); }   // speeding up
  for (int i = 0; i < 40; i++) game_update(&in);
  for (int c = 0; c < 8; c++) { game_update(&in); game_draw(fb); grab(c, 1); }   // top speed, one tick apart
  in.run = 0; in.right = 0; for (int i = 0; i < 40; i++) game_update(&in);
  in.jump = 1; for (int i = 0; i < 20; i++) game_update(&in); in.jump = 0; in.twirl = 1;
  for (int c = 0; c < 8; c++) { game_update(&in); game_update(&in); in.twirl = 0; game_draw(fb); grab(c, 2); }   // twirl
  FILE* f = fopen("out/run.ppm", "wb"); fprintf(f, "P6 960 360 255\n"); for (int i = 0; i < 960 * 360; i++) { unsigned char c[3] = { strip[i] >> 16, strip[i] >> 8, strip[i] }; fwrite(c, 1, 3, f); } fclose(f);
  return 0;
}
