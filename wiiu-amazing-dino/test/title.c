#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
int main(void) { game_init(); Input in = { 0 }; for (int i = 0; i < 70; i++) game_update(&in); game_draw(fb);
  FILE* f = fopen("out/title.ppm", "wb"); fprintf(f, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, f); } fclose(f); }
