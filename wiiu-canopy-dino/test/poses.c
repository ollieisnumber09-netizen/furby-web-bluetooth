// Draws a sheet of Dino and creature poses, big, to check the model and the animation.
#include "../src/game.c"
void plat_sfx(int id) { synth_trigger(id); }
int main(void) {
  hd_init();
  int S = 3, cw = DN * S, n = 12, rows = 4;
  int W = cw * n, H = cw * rows;
  uint32_t* big = malloc(W * H * 4);
  for (int i = 0; i < W * H; i++) big[i] = 0xFF8FC4EC;
  Bmp b = {0};
  for (int r = 0; r < rows; r++) for (int k = 0; k < n; k++) {
    DinoPose p; dino_idle_pose(&p, 50);
    if (r == 0) {   // run cycle
      float ph = k * 6.2832f / n;
      for (int i = 0; i < 2; i++) { float a = ph + i * 3.14159f; p.lx[i] = sinf(a) * 7.5f + (i ? -2.8f : 2.8f); p.ly[i] = maxf(0, cosf(a)) * 5.5f; }
      p.bob = 1.6f * fabsf(cosf(ph)); p.tilt = 0.15f; p.sq = 1 + 0.035f * sinf(ph * 2); p.spk = sinf(ph * 2 + 1) * 0.12f - 0.05f;
    } else if (r == 1) {   // jump: squash, stretch, rise, fall, land
      float t = k / (float)(n - 1);
      p.air = k > 1 && k < 10;
      p.sq = k == 0 ? 0.85f : k == 1 ? 0.8f : k < 6 ? 1.14f : k < 10 ? 1.05f : (k == 10 ? 0.8f : 0.95f);
      if (k >= 2 && k < 6) { p.tilt = -0.07f; p.lx[0] = -2.5f; p.lx[1] = 1.5f; p.ly[0] = 2.5f; p.ly[1] = 4.2f; p.mouth = 1; p.spk = -0.4f + t * 0.4f; }
      if (k >= 6 && k < 10) { p.tilt = 0.1f; p.lx[0] = -2.5f + sinf(k * 1.3f) * 1.6f; p.lx[1] = 2.5f - sinf(k * 1.3f) * 1.6f; p.mouth = 0.5f; p.spk = 0.1f + (k - 6) * 0.14f; }
    } else if (r == 2) {   // blink, headbutt, hurt, happy, wings
      if (k < 4) p.eye = 1 - k / 3.0f;
      else if (k < 7) { p.tilt = 0.27f * (k - 3); p.sq = 1.1f; p.mood = 3; p.air = 1; p.lx[0] = -4; p.lx[1] = -2; p.ly[0] = 0.5f; p.ly[1] = 1.5f; p.spk = -0.35f; }
      else if (k < 9) { p.mood = 2; p.tilt = -0.6f * (k - 6); p.air = 1; }
      else if (k < 10) { p.mood = 4; p.mouth = 1; p.bob = 3; }
      else { p.wing = k == 10 ? 0.6f : -0.6f; p.air = 1; p.ly[0] = p.ly[1] = 1; }
    } else {
      if (k < 6) beetle_render(&b, k * 1.05f, k == 5 ? 0.3f : 1, k == 4 ? 0.4f : 1, k == 3, k == 5, 100);
      else bee_render(&b, k * 2 + (k == 11 ? 1 : 0), (k - 8) * 0.1f, k == 11);
      for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) for (int yy = 0; yy < 3; yy++) for (int xx = 0; xx < 3; xx++) {
        uint32_t c = b.px[y * b.w + x]; if ((c >> 24) < 128) continue;
        big[(r * cw + 30 + y * 3 + yy) * W + k * cw + 20 + x * 3 + xx] = c;
      }
      continue;
    }
    dino_render(&b, &p, 0, S);
    for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
      uint32_t c = b.px[y * b.w + x], a = c >> 24; if (!a) continue;
      uint32_t* d = &big[(r * cw + y) * W + k * cw + x];
      *d = mixc(*d, c | 0xFF000000, a / 255.0f);
    }
  }
  FILE* f = fopen("out/poses.ppm", "wb"); fprintf(f, "P6 %d %d 255\n", W, H);
  for (int i = 0; i < W * H; i++) { unsigned char c[3] = { big[i] >> 16, big[i] >> 8, big[i] }; fwrite(c, 1, 3, f); }
  fclose(f);
  return 0;
}
