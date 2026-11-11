// Renders each tune to a file so it can be listened to.
#include "../src/game.c"
void plat_sfx(int id) { synth_trigger(id); }
int main(void) {
  game_init();
  static int16_t buf[48000 * 2];
  FILE* f = fopen("out/music.pcm", "wb");
  int themes[4] = { 0, 4, 6, 9 };   // level indices: jungle, temple(3?), snow, night
  for (int k = 0; k < 6; k++) {
    if (k == 0) state = ST_TITLE; else if (k == 1) state = ST_MAP; else { state = ST_PLAY; load_level(k == 2 ? 0 : k == 3 ? 4 : 6); g_boss_music = k == 5; }
    int peak = 0;
    for (int sec = 0; sec < 14; sec++) { synth_render(buf, 48000, 48000); fwrite(buf, 2, 96000, f); for (int i = 0; i < 96000; i++) if (abs(buf[i]) > peak) peak = abs(buf[i]); }
    printf("song %d peak %d\n", k, peak);
  }
  fclose(f); (void)themes;
  return 0;
}
