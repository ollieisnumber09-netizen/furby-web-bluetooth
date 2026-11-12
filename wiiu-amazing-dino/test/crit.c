// A sheet of the newer creatures, for checking how they look: out/crit.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
int main(void) {
  hd_init(); m3d_build();
  static uint32_t fb[FB_W * FB_H]; FB = fb;
  for (int i = 0; i < FB_W * FB_H; i++) fb[i] = 0xFF6FB07E;
  static Bmp b; float S = 2.2f;
  for (int k = 0; k < 5; k++) { float yaw = -1.2f + k * 0.6f;
    frog3d(&b, k == 4 ? 1.25f : 1, yaw, 0, S, k == 2); hblit(&b, 5 + k * 125, 0, 0);
    spiny3d(&b, k, 1, yaw, 0, S); hblit(&b, 5 + k * 125, 85, 0);
    plant3d(&b, k / 4.0f, 1, yaw, 0, S); hblit(&b, 5 + k * 125, 170, 0);
    bat3d(&b, -1 + k * 0.5f, yaw, 0, S, k == 2); hblit(&b, 5 + k * 125, 255, 0); }
  FILE* fp = fopen("out/crit.ppm", "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp);
  return 0;
}
