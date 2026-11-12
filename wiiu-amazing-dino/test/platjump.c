// Jumping off a moving platform: Dino should rise well clear of it, on every level.
#include "../src/game.c"
void plat_sfx(int id) {}
int main(void) {
  game_init();
  Input no = { 0 }, jump = { 0 }; jump.jump = 1;
  int n = 0, bad = 0; float worst = 999;
  for (int lv = 0; lv < NLEVELS; lv++) {
    start_level(lv); int np = 0; for (int i = 0; i < nents; i++) if (ents[i].type == E_PLAT && ents[i].k < 2) np++;
    for (int j = 0; j < np; j++) {
      start_level(lv); intro_t = 0; lives = 9;
      int c = 0; Ent* e = NULL; for (int i = 0; i < nents; i++) if (ents[i].type == E_PLAT && ents[i].k < 2 && c++ == j) e = &ents[i];
      px = e->x + 18; py = e->y - PH; pvx = pvy = 0; inv_t = 999;
      for (int f = 0; f < 60; f++) game_update(&no);
      if (ride_i < 0) continue;   // fell off or blocked, not a riding test
      float best = 0;
      for (int f = 0; f < 30; f++) { game_update(f < 16 ? &jump : &no); float h = e->y - (py + PH); if (h > best) best = h; }
      n++; if (best < worst) worst = best; if (best < 30) { bad++; printf("level %d platform %d: only %.0f px\n", lv + 1, j, best); }
    }
  }
  printf("%d platform rides tested, %d bad jumps, lowest jump %.0f px\n", n, bad, worst);
}
