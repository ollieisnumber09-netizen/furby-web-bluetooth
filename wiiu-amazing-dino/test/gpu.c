// Checks the Wii U's split drawing: the game's see-through picture laid over the background and
// level pictures (the way the graphics chip will) must match the game drawing everything itself.
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t A[FB_W * FB_H], B[FB_W * FB_H], C[FB_W * FB_H];
static void sblend(uint32_t* d, uint32_t c) {   // straight alpha
  int a = c >> 24; if (!a) return;
  *d = lerpc(*d | 0xFF000000, c | 0xFF000000, a + (a >> 7)) | 0xFF000000;
}
static void compose(const GameLayers* g, const uint32_t* ov, uint32_t* out) {
  for (int i = 0; i < FB_W * FB_H; i++) out[i] = 0xFF000000;
  if (g->bg) {
    for (int y = 0; y < FB_H; y++) {
      int sy = y - g->far_yoff; if (sy < 0) sy = 0; if (sy >= g->bgh) sy = g->bgh - 1;
      for (int x = 0; x < FB_W; x++) out[y * FB_W + x] = g->far[sy * g->bgw + (x + g->far_scroll) % g->bgw];
    }
    if (g->sun) for (int y = 0; y < g->sun_h; y++) for (int x = 0; x < g->sun_w; x++) sblend(&out[(g->sun_y + y) * FB_W + g->sun_x + x], g->sun[y * g->sun_w + x]);
    for (int y = 0; y < FB_H; y++) {
      int sy = y - g->near_yoff; if (sy < 0) continue; if (sy >= g->bgh) sy = g->bgh - 1;
      for (int x = 0; x < FB_W; x++) sblend(&out[y * FB_W + x], g->near[sy * g->bgw + (x + g->near_scroll) % g->bgw]);
    }
  }
  if (g->level) for (int y = 0; y < FB_H; y++) for (int x = 0; x < FB_W; x++) {
    int sx = x + g->cx, sy = y + g->cy;
    if (sx >= 0 && sy >= 0 && sx < g->lvb_w && sy < g->lvb_h) sblend(&out[y * FB_W + x], g->lvb[sy * g->lvb_w + sx]);
  }
  for (int i = 0; i < FB_W * FB_H; i++) {   // premultiplied: out = ov + out * (1 - ov alpha)
    uint32_t o = ov[i]; int ia = 256 - ((o >> 24) + ((o >> 24) >> 7));
    uint32_t d = out[i];
    uint32_t rb = ((((d & 0xFF00FF) * ia) >> 8) & 0xFF00FF) + (o & 0xFF00FF), g2 = ((((d & 0xFF00) * ia) >> 8) & 0xFF00) + (o & 0xFF00);
    out[i] = 0xFF000000 | (rb & 0xFF00FF) | (g2 & 0xFF00);
  }
}
static void save(const char* nm, const uint32_t* p) { FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { p[i] >> 16, p[i] >> 8, p[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
int main(void) {
  game_init();
  Input in = { 0 };
  static const int SPOT[][3] = { { 6, 150, 14 }, { 4, 30, 12 }, { 3, 200, 14 }, { 22, 120, 14 } };
  int bad = 0;
  for (int s = 0; s < 4; s++) {
    start_level(SPOT[s][0] - 1); lives = 9; intro_t = 0;
    px = SPOT[s][1] * T; py = SPOT[s][2] * T; set_stage(stage_of(px));
    for (int f = 0; f < 40; f++) game_update(&in);
    if (s == 3) { state = ST_PAUSE; }
    g_gpu = 0; game_draw(A);
    g_gpu = 1; game_draw(B);
    GameLayers g; game_layers(&g, 0);
    compose(&g, B, C);
    long sum = 0; int worst = 0, nbig = 0;
    for (int i = 0; i < FB_W * FB_H; i++) for (int k = 0; k < 24; k += 8) { int d = abs((int)((A[i] >> k) & 255) - (int)((C[i] >> k) & 255)); sum += d; if (d > worst) worst = d; if (d > 12) nbig++; }
    printf("%-22s layers bg %d level %d  average difference %.3f  worst %d  pixels off by more than 12: %d\n", L->name, g.bg, g.level, sum / (FB_W * FB_H * 3.0), worst, nbig);
    if (nbig > 200) { bad++; char nm[64]; snprintf(nm, 64, "out/gpuA%d.ppm", s); save(nm, A); snprintf(nm, 64, "out/gpuC%d.ppm", s); save(nm, C); }
    g_gpu = 0;
  }
  printf("%d places that do not match\n", bad);
}
