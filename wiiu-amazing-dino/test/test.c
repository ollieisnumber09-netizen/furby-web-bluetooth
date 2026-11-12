// Desktop test harness: runs the game with scripted input and saves screenshots (PPM).
#include "../src/game.c"
#include <stdlib.h>

void plat_sfx(int id) { synth_trigger(id); }
static uint32_t fb[FB_W * FB_H];

static void save(const char* name, uint32_t* p, int w, int h) {
  FILE* f = fopen(name, "wb");
  fprintf(f, "P6 %d %d 255\n", w, h);
  for (int i = 0; i < w * h; i++) { unsigned char c[3] = { p[i] >> 16, p[i] >> 8, p[i] }; fwrite(c, 1, 3, f); }
  fclose(f);
}

// whole-level map, drawn in screen-sized chunks
static void overview(int n, const char* name) {
  g_no_hud = 1;
  load_level(n); respawn(); state = ST_PLAY; state_t = 999;
  int W = LW * T * 2, H = LH * T * 2;
  uint32_t* big = calloc(W * H, 4);
  for (int y = 0; y < H; y += FB_H) for (int x = 0; x < W; x += FB_W) {
    int cx = x > W - FB_W ? W - FB_W : x, cy = y > H - FB_H ? H - FB_H : y;
    if (cy < 0) cy = 0;
    camx = cx / 2.0f; camy = cy / 2.0f;
    game_draw(fb);
    for (int j = 0; j < FB_H; j++) for (int i = 0; i < FB_W; i++) {
      int X = cx + i, Y = cy + j;
      if (X < W && Y < H) big[Y * W + X] = fb[j * FB_W + i];
    }
  }
  save(name, big, W, H);
  free(big);
}

int main(int argc, char** argv) {
  const char* out = argc > 1 ? argv[1] : ".";
  char path[512];
  game_init();
  Input in = { 0 };
  for (int i = 0; i < 40; i++) { game_update(&in); }
  game_draw(fb); snprintf(path, sizeof path, "%s/title.ppm", out); save(path, fb, FB_W, FB_H);
  in.jump = 1; game_update(&in); in.jump = 0; game_update(&in);
  for (int i = 0; i < 30; i++) game_update(&in);
  game_draw(fb); snprintf(path, sizeof path, "%s/start.ppm", out); save(path, fb, FB_W, FB_H);
  // run right, jumping now and then
  int shots = 0;
  for (int f = 0; f < 1500 && state == ST_PLAY; f++) {
    in.right = 1;
    in.jump = (f % 50) < 18;
    game_update(&in);
    if (f % 150 == 75 && shots < 8) {
      game_draw(fb); snprintf(path, sizeof path, "%s/play%d.ppm", out, shots++); save(path, fb, FB_W, FB_H);
    }
  }
  printf("after autoplay: state=%d level=%d x=%.0f y=%.0f lives=%d bananas=%d\n", state, lvl, px, py, lives, total_bananas);
  // synth sanity
  static int16_t snd[48000 * 2];
  synth_trigger(SFX_JUMP); synth_render(snd, 48000, 48000);
  int peak = 0; for (int i = 0; i < 96000; i++) if (abs(snd[i]) > peak) peak = abs(snd[i]);
  printf("audio peak %d\n", peak);
  for (int n = 0; n < NLEVELS; n++) { snprintf(path, sizeof path, "%s/level%d.ppm", out, n + 1); overview(n, path); }
  // world map, with a few levels cleared
  cleared[1] = cleared[2] = 1; total_bananas = 57; lives = 3;
  state = ST_MAP; map_node = 3; map_from = 2; map_t = 99; game_draw(fb); snprintf(path, sizeof path, "%s/map.ppm", out); save(path, fb, FB_W, FB_H);
  for (int i = 1; i <= 8; i++) cleared[i] = 1;
  world = 0; map_node = map_from = 7; game_draw(fb); snprintf(path, sizeof path, "%s/map1.ppm", out); save(path, fb, FB_W, FB_H);
  world = 1; map_node = map_from = 2; game_draw(fb); snprintf(path, sizeof path, "%s/map2.ppm", out); save(path, fb, FB_W, FB_H);
  world = 2; map_node = map_from = 1; game_draw(fb); snprintf(path, sizeof path, "%s/map3.ppm", out); save(path, fb, FB_W, FB_H); world = 0;
  for (int n = 0; n < NLEVELS; n++) { char nm[64]; snprintf(nm, sizeof nm, "%s/shot%d.ppm", out, n + 1); g_no_hud = 0; load_level(n); respawn(); state = ST_PLAY; state_t = 999; update_camera(1); game_draw(fb); save(nm, fb, FB_W, FB_H); }
  // pause and win screens
  state = ST_PAUSE; game_draw(fb); snprintf(path, sizeof path, "%s/pause.ppm", out); save(path, fb, FB_W, FB_H);
  state = ST_WIN; state_t = 100; game_draw(fb); snprintf(path, sizeof path, "%s/win.ppm", out); save(path, fb, FB_W, FB_H);
  return 0;
}
