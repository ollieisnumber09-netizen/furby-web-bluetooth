// Power-up hits, same power-up healing, leaving a level, the flag
#include "../src/game.c"
void plat_sfx(int id) {}
static Input I0;
static void ticks(Input in, int n) { for (int i = 0; i < n; i++) { Input k = in; game_update(&k); } }
int main(void) {
  game_init(); lives = 5; bananas = 10; total_bananas = 10; power = 0;
  start_level(0); intro_t = 0; ticks(I0, 5);
  power = 3; inv_t = 0; hurt(); printf("hit with helmet: power %d hp %d\n", power, hp);
  inv_t = 0; hurt(); printf("hit without: hp %d\n", hp);
  power = 2; { Ent* e = &ents[nents++]; memset(e, 0, sizeof *e); e->type = E_POW; e->alive = 1; e->k = 2; e->t = 30; e->x = px; e->y = py; } ticks(I0, 2);
  printf("same power-up again: power %d hp %d\n", power, hp);
  lives = 8; bananas = 33; total_bananas = 33; power = 4;
  { Input st = I0; st.start = 1; ticks(st, 1); ticks(I0, 1); Input dn = I0; dn.down = 1; ticks(dn, 1); }
  printf("left without the flag: state map %d, lives %d (was 5), coins %d (was 10), power %d (was 0)\n", state == ST_MAP, lives, bananas, power);
  // flag: walk past it underground is impossible, so check the rule directly: put Dino past the pole but far below
  start_level(0); intro_t = 0; ticks(I0, 5);
  px = goal_x * T + 10; py = goal_y * T + 3 * T; pvy = 0; on_ground = 1; inv_t = 999; game_update(&I0); printf("below the flag: cleared %d\n", state == ST_CLEAR);
  start_level(0); intro_t = 0; ticks(I0, 5);
  px = goal_x * T + 4; py = goal_y * T + T - PH; game_update(&I0); printf("at the flag: cleared %d\n", state == ST_CLEAR);
  return 0;
}
