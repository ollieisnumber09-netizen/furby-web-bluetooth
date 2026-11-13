// Draws the GamePad screen and the TV screen to PNGs-as-PPM, and checks the slot, sprint, jump and twirl.
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
static void ppm(const char* fn) { FILE* f = fopen(fn, "wb"); fprintf(f, "P6 %d %d 255\n", FB_W, FB_H); for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, f); } fclose(f); }
static Input I0;
static void ticks(Input in, int n) { for (int i = 0; i < n; i++) { Input k = in; game_update(&k); } }
static float ground_y(void) { return py; }
int main(void) {
  game_init(); g_pad_screen = 1;
  start_level(2); intro_t = 0; lives = 5; bananas = 23; ticks(I0, 30); update_camera(1);
  // pick up a sword while wearing the helmet: helmet goes to the slot
  power = 3; { Ent* e = &ents[nents++]; memset(e, 0, sizeof *e); e->type = E_POW; e->alive = 1; e->k = 2; e->t = 30; e->x = px; e->y = py; }
  ticks(I0, 2); printf("after pickup: power %d (sword=2), slot %d (helmet=3)\n", power, slot);
  game_draw_pad(fb); ppm("out/pad.ppm"); game_draw(fb); ppm("out/tv.ppm");
  { Input p = I0; p.pop = 1; ticks(p, 1); } printf("after pop: slot %d, falling power-up %d\n", slot, ents[nents - 1].type == E_POW && ents[nents - 1].k == 3);
  ticks(I0, 200); printf("after it lands on Dino: power %d, slot %d\n", power, slot);
  // walking and sprinting speed on flat ground
  start_level(0); intro_t = 0; ticks(I0, 20);
  { Input r = I0; r.right = 1; ticks(r, 60); printf("walk speed %.2f\n", pvx); r.run = 1; for (int t = 0; t <= 90; t += 30) { printf("  sprint after %d ticks: %.2f\n", t, pvx); ticks(r, 30); } }
  // jump height: tap vs hold
  for (int hold = 0; hold < 2; hold++) {
    start_level(0); intro_t = 0; ticks(I0, 30); float y0 = py, top = py;
    for (int t = 0; t < 70; t++) { Input j = I0; j.jump = hold ? 1 : t < 2; game_update(&j); if (py < top) top = py; }
    printf("jump %s: %.0f px high\n", hold ? "held" : "tapped", y0 - top);
  }
  // twirl: jump, then press R at the top
  start_level(0); intro_t = 0; ticks(I0, 30); { float y0 = py; Input j = I0; j.jump = 1; ticks(j, 22); float y1 = py; Input r = I0; r.run = 1; ticks(r, 1); ticks(I0, 31); printf("twirl: fell %.1f px in the half second after pressing R (height %.0f)\n", py - y1, y0 - y1);
    ticks(r, 1); printf("second twirl in the same jump: %d (should be 0)\n", twirl_t > 0); }
  return 0;
}
