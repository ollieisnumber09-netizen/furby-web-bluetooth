// Dino with each power-up, the 3D items, cold and hot: out/gear.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t sh[900 * 260];
static void putb(Bmp* b, int X0, int Y0) { for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) { uint32_t c = b->px[y * b->w + x]; int a = c >> 24; if (!a) continue; int X = X0 + x, Y = Y0 + y; if (X < 0 || Y < 0 || X >= 900 || Y >= 260) continue; sh[Y * 900 + X] = lerpc(sh[Y * 900 + X], c | 0xFF000000, a); } }
int main(void) {
  game_init(); start_level(0); make_extra_art();
  for (int i = 0; i < 900 * 260; i++) sh[i] = 0xFF7EC8F0;
  Bmp b = { 0 };
  float sw[6] = { 0.3f, 0.3f, -0.3f, 1.8f, 0.3f, 0.3f }; int gear[6] = { 3, 2, 2, 2, 4, 0 };
  for (int i = 0; i < 6; i++) { DinoPose p; dino_idle_pose(&p, 50); p.gear = gear[i]; p.sword = sw[i]; p.hot = i == 5; dino3d(&b, &p, 0.75f, 2); putb(&b, 10 + i * 145, 0); }
  for (int k = 1; k <= 4; k++) for (int f = 0; f < 4; f++) { item3d(&b, k, f * 0.5f); putb(&b, 10 + ((k - 1) * 4 + f) * 52, 170); }
  FILE* fp = fopen("out/gear.ppm", "wb"); fprintf(fp, "P6 900 260 255\n"); for (int i = 0; i < 900 * 260; i++) { unsigned char c[3] = { sh[i] >> 16, sh[i] >> 8, sh[i] }; fwrite(c, 1, 3, fp); } fclose(fp);
}
