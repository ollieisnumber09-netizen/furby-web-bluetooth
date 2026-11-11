#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
int main(void) {
  game_init();
  int lv[3] = { 5, 8, 10 };
  for (int k = 0; k < 3; k++) {
    start_level(lv[k]); lives = 9;
    Ent* b = &ents[boss_i];
    px = arena_x0 + 40; py = b->by - 40; update_camera(1);
    Input in = { 0 };
    int f, shot = 0;
    for (f = 0; f < 4000 && state == ST_PLAY; f++) {
      memset(&in, 0, sizeof in);
      if (!boss_dead) {
        // a simple fighter: walk towards the boss and jump so as to land on it
        float dx = b->x + 15 - (px + 5);
        in.right = dx > 4; in.left = dx < -4;
        if (on_ground && fabsf(dx) < 60 && fabsf(dx) > 20 && !boss_inv && (b->k != 2 || b->y > b->by + 40)) in.jump = 1;
        if (!on_ground && pvy < 0) in.jump = 1;
      } else in.right = 1;
      game_update(&in);
      if (boss_awake && !shot && f > 100) { shot = 1; game_draw(fb); char nm[64]; snprintf(nm, 64, "out/boss%d.ppm", k); FILE* fp = fopen(nm, "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp); }
    }
    printf("boss %d: frames %d state %d dead %d hp %d/%d lives %d\n", k, f, state, boss_dead, boss_hp, boss_max, lives);
  }
  return 0;
}
