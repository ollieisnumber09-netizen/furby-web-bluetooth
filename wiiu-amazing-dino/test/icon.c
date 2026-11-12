// Makes the Wii U menu icon and the loading pictures from the game's own art.
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
static void save(const char* nm, uint32_t* p, int w, int h) { FILE* f = fopen(nm, "wb"); fprintf(f, "P6 %d %d 255\n", w, h); for (int i = 0; i < w * h; i++) { unsigned char c[3] = { p[i] >> 16, p[i] >> 8, p[i] }; fwrite(c, 1, 3, f); } fclose(f); }
int main(void) {
  game_init();
  state = ST_TITLE; state_t = 31; frame = 20;   // 31: "press A" is hidden, Dino mid-hop
  g_no_hud = 1; game_draw(fb);
  save("out/splash.ppm", fb, FB_W, FB_H);
  // icon: Dino close up, 4x size then shrunk
  int W = 512; static uint32_t ic[512 * 512];
  for (int y = 0; y < W; y++) for (int x = 0; x < W; x++) {
    uint32_t c = mixc(0xFF5DB4F0, 0xFFCFEFFF, y / 400.0f);
    float dd = sd_ellipse(x + 0.5f, y + 0.5f, 256, 760, 520, 360);
    if (dd < 0) c = dd > -18 ? mixc(0xFFC8F27A, 0xFF6CC943, -dd / 18) : mixc(0xFF6CC943, 0xFF3B9A36, clampf((-dd - 18) / 90, 0, 1));
    ic[y * W + x] = c;
  }
  DinoPose p; dino_idle_pose(&p, 50); p.mouth = 1; p.sq = 1.03f; p.spk = 0.1f;
  static Bmp b; dino3d(&b, &p, 0.45f, 7.6f);
  int ox = 256 - (int)(36 * 7.6f) + 10, oy = 430 - (int)(64 * 7.6f);
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    uint32_t c = b.px[y * b.w + x], a = c >> 24; int X = ox + x, Y = oy + y;
    if (!a || X < 0 || Y < 0 || X >= W || Y >= W) continue;
    ic[Y * W + X] = mixc(ic[Y * W + X], c | 0xFF000000, a / 255.0f);
  }
  save("out/icon.ppm", ic, W, W);
  return 0;
}
