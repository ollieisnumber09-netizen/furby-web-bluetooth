// Health, safety time, seeds, birds, wet coins, bumping and respawn positions
#include "../src/game.c"
void plat_sfx(int id) {}
static Input I0;
static void ticks(Input in, int n) { for (int i = 0; i < n; i++) { Input k = in; game_update(&k); } }
static Ent* find(int type) { for (int i = 0; i < nents; i++) if (ents[i].type == type && ents[i].alive) return &ents[i]; return NULL; }
int main(void) {
  game_init(); lives = 3; start_level(0); intro_t = 0; ticks(I0, 5);
  printf("safe after respawn: inv_t %d\n", inv_t);
  inv_t = 0; hurt(); printf("hit 1: hp %d dead %d\n", hp, dead_t > 0); inv_t = 0; hurt(); printf("hit 2: hp %d dead %d\n", hp, dead_t > 0); inv_t = 0; hurt(); printf("hit 3: hp %d dead %d\n", hp, dead_t > 0);
  for (int i = 0; i < 90 && dead_t; i++) game_update(&I0); printf("after respawn: hp %d inv_t %d lives %d dead_t %d state %d\n", hp, inv_t, lives, dead_t, state);
  // seed stomp
  for (int lv = 0; lv < NLEVELS; lv++) { start_level(lv); if (find(E_PLANT)) break; } intro_t = 0; inv_t = 0;
  { py -= 60; add_shot(px + 2, py + PH + 3, 0, 0, 0); Ent* s = &ents[nents - 1]; pvy = 2; float b = 0; for (int i = 0; i < 6 && s->alive; i++) { game_update(&I0); } b = pvy; printf("seed stomp: bounced %d hurt %d seed gone %d\n", b < 0, hp < 3 || dead_t, !s->alive); }
  // wet coins
  int wetc = 0, drawn_dry = 0; for (int lv = 0; lv < NLEVELS; lv++) { start_level(lv); for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) if (WET[y][x]) { wetc++; if (!wet(x, y)) drawn_dry++; } }
  printf("coins or keys in water: %d, missing water: %d\n", wetc, drawn_dry);
  // bird waits 1.2 s
  for (int lv = 0; lv < NLEVELS; lv++) { start_level(lv); if (find(E_BIRD)) break; } intro_t = 0; inv_t = 999;
  { Ent* b = find(E_BIRD); b->t = 100; px = b->x + 30; py = b->y + 60; int seen = -1, dive = -1; for (int t = 0; t < 200 && dive < 0; t++) { game_update(&I0); px = b->x + 30; py = b->by + 60; pvy = 0; if (b->k == 3 && seen < 0) seen = t; if (b->k == 1 && dive < 0) dive = t; } printf("bird: spotted at %d, dived at %d (%.1f s later)\n", seen, dive, (dive - seen) / 60.0f); }
  // two walkers meet
  start_level(0); intro_t = 0; inv_t = 9999; px = 0; py = 0;
  { nents = 0; int c0 = (int)(start_x / T) + 1; Ent* a = add_ent(E_BEETLE, c0, 0), *b = add_ent(E_BEETLE, c0 + 5, 0); int gy = (int)((start_y + PH) / T);
    for (int x = c0; x <= c0 + 6; x++) if (!solid_c(tile(x, gy))) printf("(uneven ground at %d)\n", x);
    a->y = a->by = gy * T - 12 + 2; b->y = b->by = a->y; a->vx = 0.5f; b->vx = -0.5f; float minx = 999;
    for (int t = 0; t < 200; t++) { game_update(&I0); float d = fabsf(b->x - a->x); if (d < minx) minx = d; }
    printf("beetles meeting: closest %.1f px apart (12 = touching), a heading %s\n", minx, a->vx > 0 ? "right" : "left"); }
  // respawn puts creatures back
  for (int lv = 0; lv < NLEVELS; lv++) { start_level(lv); if (find(E_CRAB)) break; } intro_t = 0;
  { Ent* c = find(E_CRAB); float ox = c->x; c->x += 40; inv_t = 0; hp = 1; hurt(); ticks(I0, 90); printf("crab back at start after respawn: %d\n", fabsf(c->x - ox) < 3); }
  return 0;
}
