// Renders each tune to a file so it can be listened to: out/music.pcm (s16 stereo 48k).
#include "../src/game.c"
void plat_sfx(int id) { synth_trigger(id); }
int main(int argc, char** argv) {
  game_init();
  static int16_t buf[48000 * 2];
  FILE* f = fopen("out/music.pcm", "wb");
  // title, map, then a level for each tune, then the boss
  int lv[10] = { -1, -2, 0, 12, 4, 2, 6, 8, 9, 5 };
  int secs = argc > 1 ? atoi(argv[1]) : 16;
  for (int k = 0; k < 10; k++) {
    g_boss_music = 0;
    if (lv[k] == -1) state = ST_TITLE; else if (lv[k] == -2) state = ST_MAP; else { state = ST_PLAY; load_level(lv[k]); g_boss_music = k == 9; }
    int peak = 0;
    for (int sec = 0; sec < secs; sec++) { synth_render(buf, 48000, 48000); if (sec == secs - 1) for (int i = 0; i < 48000; i++) { buf[i * 2] = buf[i * 2 + 1] = (int16_t)(buf[i * 2] * (1 - i / 48000.0f)); } fwrite(buf, 2, 96000, f); for (int i = 0; i < 96000; i++) if (abs(buf[i]) > peak) peak = abs(buf[i]); }
    memset(buf, 0, sizeof buf); fwrite(buf, 2, 48000, f);
    printf("song %d (%d) peak %d\n", k, song_now, peak);
  }
  fclose(f);
  return 0;
}
