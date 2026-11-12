// Builds every level and prints what is in it; writes a tile map of each to out/lv_all.ppm (2 px a tile)
// and a few screens of chosen levels (out/lv_N_K.ppm).
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
static void shot(const char* nm) { game_draw(fb); FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
int main(int argc, char** argv) {
  game_init();
  int W = MAXW * 2, H = NLEVELS * 42;
  static unsigned char img[MAXW * 2 * 56 * 42 * 3];
  memset(img, 40, sizeof img);
  for (int n = 0; n < NLEVELS; n++) {
    start_level(n);
    int cnt[32] = { 0 }, keys = 0, cps = 0, doors = 0, water = 0;
    for (int i = 0; i < nents; i++) cnt[ents[i].type]++;
    for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) {
      char c = G[y][x]; keys += c == 'K'; cps += c == 'c'; doors += c == 'D'; water += c == 'w';
      unsigned char r = 120, g = 190, b = 240;
      if (L->theme == 9) { r = 30; g = 90; b = 160; }
      if (c == '#') { r = 150; g = 100; b = 60; } else if (c == 'X') { r = 120; g = 120; b = 130; } else if (c == '=') { r = 220; g = 170; b = 90; }
      else if (c == '^') { r = 255; g = 255; b = 255; } else if (c == 'o') { r = 250; g = 220; b = 40; } else if (c == '?') { r = 250; g = 140; b = 0; }
      else if (c == 'K') { r = 255; g = 0; b = 255; } else if (c == 'D') { r = 255; g = 0; b = 0; } else if (c == 'w') { r = 60; g = 140; b = 230; }
      else if (c == 'I') { r = 200; g = 240; b = 255; } else if (c == 'k') { r = 90; g = 60; b = 40; } else if (c == 's') { r = 255; g = 60; b = 60; } else if (c == 'c') { r = 0; g = 255; b = 0; }
      if (FAKE[y][x]) { r = 255; g = 120; b = 200; }
      for (int k = 0; k < 4; k++) { int X = x * 2 + (k & 1), Y = n * 42 + y * 2 + (k >> 1); unsigned char* p = &img[(Y * W + X) * 3]; p[0] = r; p[1] = g; p[2] = b; }
    }
    for (int i = 0; i < nents; i++) { int X = (int)(ents[i].x / T) * 2, Y = n * 42 + (int)(ents[i].y / T) * 2; if (X >= 0 && X < W - 1 && Y >= 0 && Y < H) { unsigned char* p = &img[(Y * W + X) * 3]; p[0] = 255; p[1] = 0; p[2] = 0; p[3] = 255; } }
    printf("%-26s theme %2d  w %3d  keys %d door %d cps %d  foes:", L->name, L->theme, LW, keys, doors, cps);
    for (int t = 0; t < 32; t++) if (cnt[t] && t != E_PLAT && t != E_ICE) printf(" %d:%d", t, cnt[t]);
    printf("  plats %d  boss %d\n", cnt[E_PLAT], boss_i >= 0);
  }
  FILE* fp = fopen("out/lv_all.ppm", "wb"); fprintf(fp, "P6 %d %d 255\n", W, H); fwrite(img, 1, (size_t)W * H * 3, fp); fclose(fp);
  for (int a = 1; a + 1 < argc; a += 2) {   // pairs: level number (from 1) and x tile
    int n = atoi(argv[a]) - 1; float xt = (float)atof(argv[a + 1]);
    start_level(n); intro_t = 0;
    int cx = (int)xt, cy = -1;
    for (; cx < LW && cy < 0; cx++) for (int y = 3; y < LH; y++) if (solid_c(G[y][cx]) && G[y - 1][cx] == ' ') { cy = y; break; }
    cx--; px = cx * T + 3; py = cy * T - PH;
    update_camera(1);
    for (int i = 0; i < 30; i++) { Input in = { 0 }; game_update(&in); }
    char nm[64]; snprintf(nm, 64, "out/lv_%d_%d.ppm", n + 1, (int)xt); shot(nm);
  }
}
