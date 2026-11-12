// Renders each tune as the game plays it, so it can be listened to: out/music.pcm (s16 stereo 48k).
#include "../src/game.c"
void plat_sfx(int id) { synth_trigger(id); }
int main(int argc, char** argv) {
  game_init();
  static int16_t buf[48000 * 2];
  FILE* f = fopen("out/music.pcm", "wb");
  // title, map, then a level for each tune, then the boss
  int lv[12] = { -1, -2, 0, 12, 4, 2, 6, 8, 9, 18, 22, 5 };
  int secs = argc > 1 ? atoi(argv[1]) : 6;
  for (int k = 0; k < 12; k++) {
    g_boss_music = 0;
    if (lv[k] == -1) state = ST_TITLE; else if (lv[k] == -2) state = ST_MAP; else { state = ST_PLAY; load_level(lv[k]); g_boss_music = k == 11; }
    int peak = 0;
    for (int sec = 0; sec < secs; sec++) { synth_render(buf, 48000, 48000); fwrite(buf, 2, 96000, f); for (int i = 0; i < 96000; i++) if (abs(buf[i]) > peak) peak = abs(buf[i]); }
    printf("song %d (%d) peak %d\n", k, song_now >= 0 ? song_now : MS.song, peak);
  }
  // loop check: play the title round its join and look for a jump
  state = ST_TITLE; int worst = 0, prev = 0;
  for (int i = 0; i < 48000 * 26; i++) { synth_render(buf, 1, 48000); int d = abs(buf[0] - prev); if (i > 48000 * 22 && d > worst) worst = d; prev = buf[0]; }
  printf("biggest step between samples around the loop join: %d\n", worst);
  fclose(f);
  return 0;
}
