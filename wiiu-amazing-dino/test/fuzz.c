// Plays every level for a while with a simple autopilot, drawing every frame, to catch crashes.
#include "../src/game.c"
void plat_sfx(int id) { synth_trigger(id); }
static uint32_t fb[FB_W * FB_H];
int main(int argc, char** argv) {
  game_init();
  int frames = argc > 1 ? atoi(argv[1]) : 1500;
  static int16_t snd[800 * 2];
  for (int n = 0; n < NLEVELS; n++) {
    start_level(n); intro_t = 0; lives = 99;
    float far = 0; int deaths = 0, wasdead = 0;
    for (int f = 0; f < frames; f++) {
      Input in = { 0 };
      in.right = (f / 200) % 5 != 4; in.left = !in.right;
      in.jump = (f % 37) < 12 || in_water && (f % 9) < 2;
      in.attack = (f % 50) == 0;
      if (state != ST_PLAY) { in.jump = f % 2; }
      game_update(&in);
      if (f % 3 == 0) game_draw(fb);
      synth_render(snd, 800, 48000);
      if (dead_t && !wasdead) deaths++;
      wasdead = dead_t != 0;
      if (px > far) far = px;
      if (state != ST_PLAY) { start_level(n); intro_t = 0; lives = 99; }
    }
    printf("%-26s got to %3.0f of %3d, deaths %d, power %d\n", L->name, far / T, LW, deaths, power);
  }
}
