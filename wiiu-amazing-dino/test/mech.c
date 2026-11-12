// Checks the newer mechanics one at a time, and saves pictures of the newest levels (out/m_N.ppm).
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H]; static int nshot;
static void save(void) { char nm[64]; snprintf(nm, 64, "out/m_%d.ppm", nshot++); game_draw(fb); FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
static void go(int n) { start_level(n); intro_t = 0; lives = 9; }
static void put_at(float tx, float ty) { px = tx * T; py = ty * T - PH; pvx = pvy = 0; update_camera(1); }
static void run(Input in, int n) { for (int i = 0; i < n; i++) { game_update(&in); in.attack = in.attack && i < 1; } }
int main(void) {
  game_init();
  Input no = { 0 }, right = { 0 }, atk = { 0 }; right.right = 1; atk.attack = 1;
  // 1. riding platforms (level 19 has a side-to-side one and an up-and-down one)
  go(18);
  for (int i = 0, k = 0; i < nents; i++) if (ents[i].type == E_PLAT) {
    Ent* e = &ents[i]; px = e->x + 18; py = e->y - PH; pvx = pvy = 0;
    float lo = 1e9, hi = -1e9; int off = 0;
    for (int f = 0; f < 500; f++) { game_update(&no); if (!(px + PW > e->x && px < e->x + 48 && fabsf(py + PH - e->y) < 3)) off++; if (e->k ? e->y < lo : e->x < lo) lo = e->k ? e->y : e->x; if ((e->k ? e->y : e->x) > hi) hi = e->k ? e->y : e->x; }
    printf("platform %d (%s): travelled %.0f..%.0f px, frames off it: %d, state %d\n", k++, e->k ? "up/down" : "sideways", lo, hi, off, state);
    if (k == 1) { update_camera(1); save(); }
  }
  // 2. pretend wall: walk from the step into the hill
  go(18); put_at(38.5f, 9); run(right, 70);
  printf("pretend wall: Dino at tile %.1f,%.1f (inside the room is x 42..47, y 8..10), coins %d\n", px / T, py / T, bananas); update_camera(1); save();
  // 3. helmet smashes cracked blocks, and without it they stay
  go(18); put_at(90.6f, 11); face = 1; power = 0; run(atk, 40); printf("no helmet: block is '%c'\n", G[10][92]);
  go(18); put_at(90.6f, 11); face = 1; power = 3; update_camera(1); save(); run(atk, 40); printf("helmet: block is '%c'\n", G[10][92]);
  // 4. spiky ball: stomp hurts, headbutt bounces, star beats it
  go(18); Ent* sp = NULL; for (int i = 0; i < nents; i++) if (ents[i].type == E_SPINY) { sp = &ents[i]; break; }
  put_at(sp->x / T - 1.2f, 11); face = 1; power = 0; run(atk, 30); printf("headbutt on spiky: dying %d, Dino alive %d\n", sp->dying, !dead_t);
  go(18); for (int i = 0; i < nents; i++) if (ents[i].type == E_SPINY) { sp = &ents[i]; break; }
  put_at(sp->x / T - 3, 11); face = 1; power = 4; run(atk, 3); update_camera(1); save(); run(no, 50); printf("star on spiky: dying %d alive %d\n", sp->dying, sp->alive);
  // 5. frog hops, flower spits, bat wakes
  go(18); put_at(11, 11); run(no, 200); { int shots = 0; for (int i = 0; i < nents; i++) if (ents[i].type == E_SHOT) shots++; Ent* fr = NULL; for (int i = 0; i < nents; i++) if (ents[i].type == E_FROG) { fr = &ents[i]; break; } printf("frog moved from %.0f to %.0f; lives %d\n", fr->x0, fr->x, lives); }
  go(18); put_at(20.5f, 9); run(no, 160); { int shots = 0; for (int i = 0; i < nents; i++) if (ents[i].type == E_SHOT) shots++; printf("flower: seeds made %d, power %d dead %d\n", shots, power, dead_t); } update_camera(1); save();
  go(22); put_at(8, 13); run(no, 60); { Ent* b = NULL; for (int i = 0; i < nents; i++) if (ents[i].type == E_BAT) { b = &ents[i]; break; } printf("bat awake %d at %.0f,%.0f (Dino %.0f,%.0f)\n", b->k, b->x, b->y, px, py); } update_camera(1); save();
  // pictures
  go(19); put_at(28, 12); run(no, 30); update_camera(1); save();
  go(21); put_at(26, 6); run(no, 30); update_camera(1); save();
  go(22); put_at(41, 13); run(no, 5); update_camera(1); save();
  go(20); put_at(18, 9); run(no, 5); update_camera(1); save();
  for (int b = 0; b < 2; b++) { go(b ? 23 : 20); Ent* e = &ents[boss_i]; px = arena_x0 + 40; py = e->by - 10; run(no, 150); update_camera(1); save(); printf("boss %s: awake %d hp %d\n", BOSS_NAME[e->k], boss_awake, boss_hp); }
  for (int i = 1; i <= NLEVELS; i++) cleared[i] = i < 21;
  for (int wd = 5; wd < 7; wd++) { state = ST_MAP; world = wd; map_node = map_from = 1; map_t = 99; frame = 40; save(); }
  printf("shots %d\n", nshot);
  return 0;
}
