// Amazing Dino: web front end. The page calls these from JavaScript: it draws the picture on a
// canvas, plays the sound through Web Audio and reads the touch buttons and keyboard.
#include <emscripten.h>
#include <string.h>
#include "../src/game.h"

extern const unsigned char* music_bin;
static uint32_t fb[FB_W * FB_H];
static int16_t abuf[8192 * 2];

void plat_sfx(int id) { synth_trigger(id); }

EMSCRIPTEN_KEEPALIVE void web_init(const unsigned char* music) { music_bin = music; game_init(); }
// bits: 1 left, 2 right, 4 up, 8 down, 16 jump, 32 start, 64 attack
EMSCRIPTEN_KEEPALIVE void web_tick(int bits) {
  Input in;
  in.left = bits & 1; in.right = (bits >> 1) & 1; in.up = (bits >> 2) & 1; in.down = (bits >> 3) & 1;
  in.jump = (bits >> 4) & 1; in.start = (bits >> 5) & 1; in.attack = (bits >> 6) & 1;
  game_update(&in);
}
EMSCRIPTEN_KEEPALIVE uint32_t* web_draw(int lowq) {
  g_lowq = lowq;
  game_draw(fb);
  for (int i = 0; i < FB_W * FB_H; i++) {   // 0xAARRGGBB to the canvas's byte order R,G,B,A
    uint32_t c = fb[i];
    fb[i] = 0xFF000000u | ((c & 0xFF) << 16) | (c & 0xFF00) | ((c >> 16) & 0xFF);
  }
  return fb;
}
EMSCRIPTEN_KEEPALIVE int16_t* web_audio(int frames, int rate) {
  if (frames > 8192) frames = 8192;
  synth_render(abuf, frames, rate);
  return abuf;
}
