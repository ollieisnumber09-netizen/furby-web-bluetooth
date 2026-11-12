// Scripted checks of the newest things: swimming, pools, keys and the door, crumbly platforms,
// and each new creature doing its thing.
#include "../src/game.c"
void plat_sfx(int id) {}
static Input I0;
static void ticks(Input in, int n) { for (int i = 0; i < n; i++) { Input k = in; game_update(&k); } }
static void go(int n) { start_level(n); intro_t = 0; lives = 9; }
static Ent* find(int type) { for (int i = 0; i < nents; i++) if (ents[i].type == type && ents[i].alive) return &ents[i]; return NULL; }
static void stand_near(Ent* e, float dx) { px = e->x + dx; py = e->y + (e->type == E_BIRD || e->type == E_FISH || e->type == E_JELLY ? 60 : 10) - PH + 6; pvx = pvy = 0; update_camera(1); }
int main(void) {
  game_init();
  // swimming: a water level; tap jump and Dino goes up, leave it and he sinks slowly
  go(3); px = start_x + 40; py = start_y - 60; pvy = 0;
  float y0 = py; Input j = I0; j.jump = 1;
  for (int i = 0; i < 40; i++) { Input k = I0; k.jump = i % 10 < 2; game_update(&k); }
  printf("swim: in water %d, rose %.0f px with 4 strokes\n", in_water, y0 - py);
  y0 = py; ticks(I0, 60); printf("swim: sank %.0f px in a second (falling would be far more)\n", py - y0);
  // a pool in a land level: fall in, then leap out onto the far side
  go(0); int pool = -1; for (int x = 0; x < LW && pool < 0; x++) if (G[15][x] == 'w') pool = x;
  px = (pool + 1) * T; py = 15 * T; pvy = 0;
  ticks(I0, 10); printf("pool at column %d: in water %d\n", pool, in_water);
  int out = 0;
  for (int i = 0; i < 400 && !out; i++) { Input k = I0; k.right = 1; k.jump = i % 12 < 2; game_update(&k); if (!in_water && on_ground) out = 1; }
  printf("pool: climbed out %d at column %d\n", out, (int)(px / T));
  // keys and the door
  go(0); int nk = 0, kx[3], ky[3], dx = -1, dy = 0;
  for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) { if (G[y][x] == 'K' && nk < 3) { kx[nk] = x; ky[nk] = y; nk++; } if (G[y][x] == 'D' && dx < 0) { dx = x; dy = y; } }
  px = (dx - 2) * T; py = (dy + 2) * T - PH; pvy = 0; { Input r = I0; r.right = 1; ticks(r, 60); }
  printf("door at %d: Dino stopped at %.1f, message %d, door tile '%c'\n", dx, px / T, door_msg_t > 0, G[dy][dx]);
  for (int k = 0; k < 3; k++) { px = kx[k] * T; py = ky[k] * T; pvx = pvy = 0; ticks(I0, 2); }
  printf("keys: holding %d (7 = all three)\n", keyp);
  px = (dx - 2) * T; py = (dy + 2) * T - PH; pvy = 0; { Input r = I0; r.right = 1; ticks(r, 90); }
  printf("door: tile now '%c', Dino at %.1f\n", G[dy][dx], px / T);
  // crumbly platform
  go(1); Ent* fp = NULL; for (int i = 0; i < nents; i++) if (ents[i].type == E_PLAT && ents[i].k == 2) { fp = &ents[i]; break; }
  if (fp) { px = fp->x + 18; py = fp->y - PH; pvy = 0; float by = fp->y; ticks(I0, 25); float a = fp->y; ticks(I0, 40); printf("crumbly platform: moved %.0f after 25 ticks, %.0f after 65\n", a - by, fp->y - by); }
  // creatures
  struct { int lv, type; const char* name; } C[] = { { 8, E_CRAB, "crab" }, { 3, E_FISH, "fish" }, { 10, E_JELLY, "jellyfish" }, { 25, E_ROLL, "armadillo" },
    { 16, E_PENGUIN, "penguin" }, { 24, E_MOLE, "mole" }, { 11, E_BIRD, "bird" }, { 17, E_SNOWMAN, "snowman" } };
  for (int c = 0; c < 8; c++) {
    go(C[c].lv); Ent* e = find(C[c].type);
    if (!e) { printf("%s: none in level %d\n", C[c].name, C[c].lv + 1); continue; }
    float ex = e->x, ey = e->y; int maxk = 0, shots = 0;
    stand_near(e, C[c].type == E_BIRD ? 10 : -60);
    lives = 9;
    for (int i = 0; i < 300; i++) { game_update(&I0); if (e->k > maxk) maxk = e->k; for (int s = 0; s < nents; s++) if (ents[s].type == E_SHOT && ents[s].alive && ents[s].k >= 2) shots++; if (dead_t || inv_t) break; }
    printf("%-10s moved %.0f,%.0f  furthest mode %d  thrown things seen %d  Dino hurt %d  alive %d\n", C[c].name, e->x - ex, e->y - ey, maxk, shots > 0, dead_t || inv_t || power, e->alive);
  }
  // stomp rules: jellyfish stings, armadillo rolled up bounces you, crab squashes
  go(10); { Ent* e = find(E_JELLY); px = e->x; py = e->y - PH - 4; pvy = 2; power = 0; inv_t = 0; ticks(I0, 3); printf("stomp jellyfish: Dino hurt %d, jelly dying %d\n", dead_t > 0, e->dying > 0); }
  go(8); { Ent* e = find(E_CRAB); px = e->x; py = e->y - PH - 2; pvy = 2; ticks(I0, 3); printf("stomp crab: crab dying %d, Dino hurt %d\n", e->dying > 0, dead_t > 0); }
  go(25); { Ent* e = find(E_ROLL); e->k = 2; px = e->x; py = e->y - PH - 2; pvy = 2; ticks(I0, 2); printf("stomp rolling armadillo: bounced %d, armadillo dying %d, Dino hurt %d\n", pvy < 0, e->dying > 0, dead_t > 0); }
}
