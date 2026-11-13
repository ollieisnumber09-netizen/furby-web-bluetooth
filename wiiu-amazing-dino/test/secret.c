// A hidden room: covered by ground from outside, open once Dino walks in. Writes out/secret.png (via ppm)
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H], img[FB_W * FB_H * 2];
int main(void) {
  game_init();
  for (int lv = 0; lv < NLEVELS; lv++) { start_level(lv); if (nrooms > 1) break; }
  printf("level %d has %d hidden rooms\n", lvl + 1, nrooms);
  intro_t = 0; int r = 1;
  // stand just outside the entrance
  px = (ROOM[r].x0 - 3) * T; py = ROOM[r].y1 * T + T - PH; pvx = pvy = 0; Input in = { 0 }; for (int i = 0; i < 5; i++) game_update(&in);
  update_camera(1); game_draw(fb); memcpy(img, fb, sizeof fb);
  printf("outside: show %.2f\n", ROOM[r].show);
  in.right = 1; for (int i = 0; i < 60; i++) game_update(&in);
  printf("walked in: show %.2f, Dino at column %.0f (room %d..%d)\n", ROOM[r].show, px / T, ROOM[r].x0, ROOM[r].x1);
  in.right = 0; for (int i = 0; i < 10; i++) game_update(&in);
  inv_t = 0;
  update_camera(1); game_draw(fb); memcpy(img + FB_W * FB_H, fb, sizeof fb);
  FILE* f = fopen("out/secret.ppm", "wb"); fprintf(f, "P6 %d %d 255\n", FB_W, FB_H * 2); for (int i = 0; i < FB_W * FB_H * 2; i++) { unsigned char c[3] = { img[i] >> 16, img[i] >> 8, img[i] }; fwrite(c, 1, 3, f); } fclose(f);
}
