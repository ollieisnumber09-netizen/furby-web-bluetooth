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
// bits: 1 left, 2 right, 4 up, 8 down, 16 jump, 32 start, 64 attack, 128 run, 256 drop the kept power-up, 512 twirl
EMSCRIPTEN_KEEPALIVE void web_tick(int bits) {
  Input in;
  in.left = bits & 1; in.right = (bits >> 1) & 1; in.up = (bits >> 2) & 1; in.down = (bits >> 3) & 1;
  in.jump = (bits >> 4) & 1; in.start = (bits >> 5) & 1; in.attack = (bits >> 6) & 1; in.run = (bits >> 7) & 1; in.pop = (bits >> 8) & 1; in.twirl = (bits >> 9) & 1;
  game_update(&in);
}
// In a level the background and the level picture are left out of fb and the page draws them from
// canvases (the browser does that on the graphics chip); fb then only holds what goes on top.
static GameLayers LY;
static uint32_t rgba_buf[1024 * 1024];
static int lay_info[24];
static void to_rgba(uint32_t* d, const uint32_t* s, int n, int unpremul) {   // 0xAARRGGBB to the canvas's bytes R,G,B,A
  for (int i = 0; i < n; i++) {
    uint32_t c = s[i], a = c >> 24, r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
    if (unpremul && a && a < 255) { r = r * 255 / a; g = g * 255 / a; b = b * 255 / a; if (r > 255) r = 255; if (g > 255) g = 255; if (b > 255) b = 255; }
    d[i] = (a << 24) | (b << 16) | (g << 8) | r;
  }
}
EMSCRIPTEN_KEEPALIVE uint32_t* web_draw(int lowq) {
  g_lowq = lowq; g_gpu = 1;
  game_draw(fb);
  game_layers(&LY, 0);
  to_rgba(fb, fb, FB_W * FB_H, LY.bg || LY.level);
  return fb;
}
// what to draw under fb: [bg, level, bg_ver, lvb_ver, bgw, bgh, far_scroll, far_yoff, near_scroll, near_yoff,
//  has_sun, sun_x, sun_y, sun_w, sun_h, lvb_w, lvb_h, cx, cy]
EMSCRIPTEN_KEEPALIVE int* web_layers(void) {
  int v[19] = { LY.bg, LY.level, LY.bg_ver, LY.lvb_ver, LY.bgw, LY.bgh, LY.far_scroll, LY.far_yoff, LY.near_scroll, LY.near_yoff,
                LY.sun != 0, LY.sun_x, LY.sun_y, LY.sun_w, LY.sun_h, LY.lvb_w, LY.lvb_h, LY.cx, LY.cy };
  memcpy(lay_info, v, sizeof v);
  return lay_info;
}
// the pixels of one layer picture, as canvas bytes: 0 far strip, 1 near strip, 2 sun, 3 + k the level picture's k-th 1024-wide slice
EMSCRIPTEN_KEEPALIVE uint32_t* web_layer_px(int which) {
  if (which == 0 || which == 1) to_rgba(rgba_buf, which ? LY.near : LY.far, LY.bgw * LY.bgh, 0);
  else if (which == 2) to_rgba(rgba_buf, LY.sun, LY.sun_w * LY.sun_h, 0);
  else {
    int x0 = (which - 3) * 1024, w = LY.lvb_w - x0 < 1024 ? LY.lvb_w - x0 : 1024;
    for (int y = 0; y < LY.lvb_h; y++) to_rgba(rgba_buf + y * w, LY.lvb + y * LY.lvb_w + x0, w, 0);
  }
  return rgba_buf;
}
EMSCRIPTEN_KEEPALIVE int16_t* web_audio(int frames, int rate) {
  if (frames > 8192) frames = 8192;
  synth_render(abuf, frames, rate);
  return abuf;
}
