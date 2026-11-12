// Checks every level can be finished: from the start, which places can Dino get to (running,
// jumping, wall-jumping, springs, swimming, riding platforms)? Are all three key pieces among them,
// and then the flag? Uses a copy of Dino's movement on the real level.
#include "../src/game.c"
void plat_sfx(int id) {}
static unsigned char seen[MAXH][MAXW], wseen[MAXH][MAXW], got[MAXH][MAXW];
static int qx[MAXH * MAXW * 2], qy[MAXH * MAXW * 2], qn, wqx[MAXH * MAXW], wqy[MAXH * MAXW], wqn;
static int k_open;   // cracked blocks count as open (Dino has the helmet)
static int solid_r(char c) { return solid_c(c) && !(k_open && c == 'k'); }
static int bsolid(float x, float y, float w, float h) {
  int x0 = (int)floorf(x / T), x1 = (int)floorf((x + w - 0.01f) / T), y0 = (int)floorf(y / T), y1 = (int)floorf((y + h - 0.01f) / T);
  for (int ty = y0; ty <= y1; ty++) for (int tx = x0; tx <= x1; tx++) if (solid_r(tile(tx, ty))) return 1;
  return 0;
}
static int is_water(int tx, int ty) { char c = tile(tx, ty); return L->theme == 9 ? !solid_r(c) && ty >= 0 && ty < LH && c != '^' : c == 'w' || (c == 'K' && tile(tx, ty - 1) == 'w') || (c == 'o' && tile(tx, ty - 1) == 'w'); }
static void push(int x, int y) { if (x < 0 || x >= LW || y < 0 || y >= LH || seen[y][x]) return; seen[y][x] = 1; qx[qn] = x; qy[qn] = y; qn++; }
static void wpush(int x, int y) { if (x < 0 || x >= LW || y < 0 || y >= LH || wseen[y][x] || !is_water(x, y)) return; wseen[y][x] = 1; wqx[wqn] = x; wqy[wqn] = y; wqn++; }
// one jump (or walk off a ledge): where does Dino come to rest?
static void sim(float x, float y, float vx, float vy, int d, int hold, int steer, int wj) {
  int ground = vy == 0, lock = 0, since = 0, spring = 0;
  if (ground && hold > 0) { vy = -5.3f; ground = 0; }
  for (int t = 0; t < 260; t++) {
    int dir = d;
    if (steer == 1 && vy > 0) dir = 0;
    if (steer == 2 && vy > 0) dir = -d;
    if (lock > 0) { lock--; dir = 0; }
    float acc = ground ? 0.32f : 0.22f;
    if (dir) { vx += dir * acc; if (vx > 2.1f) vx = 2.1f; if (vx < -2.1f) vx = -2.1f; }
    else if (!lock) { float f = ground ? 0.3f : 0.08f; if (vx > f) vx -= f; else if (vx < -f) vx += f; else vx = 0; }
    int wd = 0;
    if (!ground) { if (bsolid(x + PW, y + 2, 1, PH - 4)) wd = 1; else if (bsolid(x - 1, y + 2, 1, PH - 4)) wd = -1; }
    since++;
    if (wj && wd && vy > -0.5f && since > 8) { vy = -5.0f; vx = -wd * 2.3f; lock = 9; since = 0; d = -wd; }
    if (vy >= 0) spring = 0;
    int holding = since < hold || (wj && since < 14);
    float g = ((holding || spring) && vy < 0) ? 0.28f : 0.55f;
    vy += g;
    float mf = (wd && dir == wd && vy > 0) ? 1.1f : 5.5f;
    if (vy > mf) vy = mf;
    float nx = x + vx;
    if (bsolid(nx, y, PW, PH)) { if (vx > 0) nx = floorf((nx + PW) / T) * T - PW; else if (vx < 0) nx = (floorf(nx / T) + 1) * T; vx = 0; }
    x = nx;
    float ny = y + vy; ground = 0;
    if (vy > 0) {
      float b0 = y + PH, b1 = ny + PH;
      int x0 = (int)floorf(x / T), x1 = (int)floorf((x + PW - 0.01f) / T), ty = (int)floorf((b1 - 0.01f) / T), hit = 0;
      for (int tx = x0; tx <= x1; tx++) { char c = tile(tx, ty); if (solid_r(c) || (c == '=' && b0 <= ty * T + 0.01f)) hit = 1; }
      if (hit) { ny = ty * T - PH; vy = 0; ground = 1; }
    } else if (vy < 0 && bsolid(x, ny, PW, PH)) { int ty = (int)floorf(ny / T); ny = (ty + 1) * T; vy = 0; }
    y = ny;
    int x0 = (int)floorf(x / T), x1 = (int)floorf((x + PW - 0.01f) / T), y0 = (int)floorf(y / T), y1 = (int)floorf((y + PH - 0.01f) / T);
    for (int ty = y0; ty <= y1; ty++) for (int tx = x0; tx <= x1; tx++) {
      if (tx < 0 || tx >= LW || ty < 0 || ty >= LH) continue;
      char c = tile(tx, ty);
      got[ty][tx] = 1;
      if (c == '^' && y + PH > ty * T + 9) return;
      if (c == 's' && vy >= 0 && y + PH > ty * T + 8) { vy = -8.6f; spring = 1; ground = 0; }
    }
    if (y > LH * T + 24) return;
    int cx = (int)floorf((x + PW / 2) / T), cy = (int)floorf((y + PH / 2) / T);
    if (vy > 0 && is_water(cx, cy) && L->theme != 9) { wpush(cx, cy); return; }
    if (ground && t > 0) { push((int)floorf((x + PW / 2) / T), (int)floorf((y + PH - 1) / T)); return; }
  }
}
static void from_cell(int cx, int cy) {
  float y = cy * T + T - PH;
  static const int H[5] = { 0, 5, 10, 16, 40 };
  for (int d = -1; d <= 1; d++) for (int s = 0; s < 2; s++) for (int h = 0; h < 5; h++) for (int st = 0; st < 3; st++) {
    if (d == 0 && (s || st)) continue;
    if (h == 0 && st) continue;
    float x = d > 0 ? cx * T + T - PW - 0.2f : d < 0 ? cx * T + 0.2f : cx * T + 3;
    if (bsolid(x, y, PW, PH)) x = cx * T + 3;
    sim(x, y, s ? d * 2.1f : 0, 0, d, h, st, 0);
    if (h == 4) sim(x, y, s ? d * 2.1f : 0, 0, d, h, st, 1);
  }
}
static int analyse(int door_open, int* keys_out) {
  memset(seen, 0, sizeof seen); memset(wseen, 0, sizeof wseen); memset(got, 0, sizeof got);
  char saved[MAXH][MAXW]; memcpy(saved, G, sizeof G);
  if (door_open) for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) if (G[y][x] == 'D') G[y][x] = 'd';
  for (int i = 0; i < nents; i++) {   // platforms: every place one goes counts as a plank
    Ent* e = &ents[i];
    if (e->type != E_PLAT) continue;
    int c0 = (int)(e->x0 / T), r = (int)(e->by / T);
    if (e->k == 1) { for (int rr = (int)((e->by - e->vx) / T); rr <= r; rr++) for (int c = c0; c < c0 + 3; c++) if (G[rr][c] == ' ') G[rr][c] = '='; }
    else { int c1 = (int)((e->x0 + (e->k == 0 ? e->vx : 0)) / T) + 2; for (int c = c0; c <= c1; c++) if (G[r][c] == ' ') G[r][c] = '='; }
  }
  qn = 0; wqn = 0;
  int sx = (int)(start_x / T), sy = (int)((start_y + PH - 1) / T);
  if (L->theme == 9) wpush(sx, sy); else push(sx, sy);
  int qi = 0, wi = 0;
  while (qi < qn || wi < wqn) {
    while (qi < qn) { int x = qx[qi], y = qy[qi]; qi++; got[y][x] = 1; from_cell(x, y); }
    while (wi < wqn) {
      int x = wqx[wi], y = wqy[wi]; wi++; got[y][x] = 1;
      wpush(x + 1, y); wpush(x - 1, y); wpush(x, y + 1); wpush(x, y - 1);
      if (L->theme != 9 && !is_water(x, y - 1) && !solid_r(tile(x, y - 1)))   // at the top of a pool: leap out
        for (int d = -1; d <= 1; d++) { float px0 = x * T + 3, py0 = y * T + 1; if (!bsolid(px0, py0, PW, PH)) { int keep = wqn; sim(px0, py0, d * 1.5f, -5.2f, d, 0, 0, 0); (void)keep; } }
      // standing on the bottom of a pool next to a ledge: walk out
      if (L->theme != 9) { for (int d = -1; d <= 1; d += 2) if (!solid_r(tile(x + d, y)) && !is_water(x + d, y)) push(x + d, y); }
    }
  }
  int keys = 0, goal = 0, arena = 0;
  for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) {
    if (saved[y][x] == 'K' && got[y][x]) keys++;
    if (got[y][x] && x >= goal_x - 1) goal = 1;
    if (got[y][x] && boss_i >= 0 && x * T > arena_x0 + 32) arena = 1;
  }
  memcpy(G, saved, sizeof G);
  *keys_out = keys;
  return boss_i >= 0 ? arena : goal;
}
int main(int argc, char** argv) {
  game_init();
  int bad = 0, from = argc > 1 ? atoi(argv[1]) - 1 : 0, to = argc > 2 ? atoi(argv[2]) : NLEVELS;
  for (int n = from; n < to; n++) {
    start_level(n);
    int nk = 0; for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) nk += G[y][x] == 'K';
    k_open = 0;
    int keys, ok, keys2 = 0;
    analyse(0, &keys);
    ok = analyse(1, &keys2);
    const char* note = "";
    if (keys < nk || !ok) { k_open = 1; int k3; analyse(0, &k3); int ok3 = analyse(1, &k3); if (ok3 && k3 >= nk) note = " (needs the helmet)"; keys = k3; ok = ok3; }
    int fine = keys >= nk && ok;
    if (!fine) bad++;
    // the furthest place reached, to show where it gets stuck
    int far = 0; for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) if (got[y][x] && x > far) far = x;
    printf("%-26s keys %d/%d  %s%s  (got to column %d of %d)\n", L->name, keys, nk, fine ? "OK" : "STUCK", note, far, LW);
  }
  printf("%d levels with problems\n", bad);
}
