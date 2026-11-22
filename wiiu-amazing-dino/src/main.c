// Amazing Dino: Wii U (and desktop SDL2) front end. Opens the window (on the Wii U SDL shows it
// on both the TV and the GamePad), reads the controllers, runs the game at 60 ticks a second
// and plays its sound.
#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "game.h"

#ifdef __WIIU__
#include <vpad/input.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#endif

static SDL_AudioDeviceID g_audio;
static int g_rate = 48000;

void plat_sfx(int id) {
  if (g_audio) SDL_LockAudioDevice(g_audio);
  synth_trigger(id);
  if (g_audio) SDL_UnlockAudioDevice(g_audio);
}
static void audio_cb(void* u, Uint8* stream, int len) {
  (void)u;
  synth_render((int16_t*)stream, len / 4, g_rate);
}

#ifdef __WIIU__
// GamePad, plus up to four Pro Controllers / Classic Controllers / Wii Remotes.
static void wiiu_read(Input* in) {
  VPADStatus v; VPADReadError err;
  if (VPADRead(VPAD_CHAN_0, &v, 1, &err) == 1 && err == VPAD_READ_SUCCESS) {
    uint32_t h = v.hold;
    if (h & (VPAD_BUTTON_LEFT | VPAD_STICK_L_EMULATION_LEFT)) in->left = 1;
    if (h & (VPAD_BUTTON_RIGHT | VPAD_STICK_L_EMULATION_RIGHT)) in->right = 1;
    if (h & (VPAD_BUTTON_UP | VPAD_STICK_L_EMULATION_UP)) in->up = 1;
    if (h & (VPAD_BUTTON_DOWN | VPAD_STICK_L_EMULATION_DOWN)) in->down = 1;
    if (h & (VPAD_BUTTON_A | VPAD_BUTTON_B)) in->jump = 1;
    if (h & VPAD_BUTTON_PLUS) in->start = 1;
    if (h & (VPAD_BUTTON_X | VPAD_BUTTON_Y)) in->attack = 1;
    if (h & (VPAD_BUTTON_ZL | VPAD_BUTTON_ZR)) in->run = 1;
    if (h & (VPAD_BUTTON_L | VPAD_BUTTON_R)) in->twirl = 1;
    if (h & VPAD_BUTTON_MINUS) in->pop = 1;
    if (v.tpNormal.touched) {   // a tap on the power-up slot
      VPADTouchData t;
      VPADGetTPCalibratedPointEx(VPAD_CHAN_0, VPAD_TP_854X480, &t, &v.tpNormal);
      if (game_pad_slot_hit(t.x * FB_W / 854, t.y * FB_H / 480)) in->pop = 1;
    }
    if (v.leftStick.x < -0.35f) in->left = 1;
    if (v.leftStick.x > 0.35f) in->right = 1;
    if (v.leftStick.y < -0.5f) in->down = 1;
  }
  for (int ch = 0; ch < 4; ch++) {
    WPADExtensionType ext;
    if (WPADProbe((WPADChan)ch, &ext) != 0) continue;
    KPADStatus k; KPADError kerr;
    memset(&k, 0, sizeof k);
    if (KPADReadEx((KPADChan)ch, &k, 1, &kerr) < 1) continue;
    float sx = 0, sy = 0;
    if (k.extensionType == WPAD_EXT_PRO_CONTROLLER) {
      uint32_t b = k.pro.hold;
      if (b & WPAD_PRO_BUTTON_LEFT) in->left = 1;
      if (b & WPAD_PRO_BUTTON_RIGHT) in->right = 1;
      if (b & WPAD_PRO_BUTTON_UP) in->up = 1;
      if (b & WPAD_PRO_BUTTON_DOWN) in->down = 1;
      if (b & (WPAD_PRO_BUTTON_A | WPAD_PRO_BUTTON_B)) in->jump = 1;
      if (b & WPAD_PRO_BUTTON_PLUS) in->start = 1;
      if (b & (WPAD_PRO_BUTTON_X | WPAD_PRO_BUTTON_Y)) in->attack = 1;
      if (b & (WPAD_PRO_TRIGGER_ZL | WPAD_PRO_TRIGGER_ZR)) in->run = 1;
      if (b & (WPAD_PRO_TRIGGER_L | WPAD_PRO_TRIGGER_R)) in->twirl = 1;
      if (b & WPAD_PRO_BUTTON_MINUS) in->pop = 1;
      sx = k.pro.leftStick.x; sy = k.pro.leftStick.y;
    } else if (k.extensionType == WPAD_EXT_CLASSIC || k.extensionType == WPAD_EXT_MPLUS_CLASSIC) {
      uint32_t b = k.classic.hold;
      if (b & WPAD_CLASSIC_BUTTON_LEFT) in->left = 1;
      if (b & WPAD_CLASSIC_BUTTON_RIGHT) in->right = 1;
      if (b & WPAD_CLASSIC_BUTTON_UP) in->up = 1;
      if (b & WPAD_CLASSIC_BUTTON_DOWN) in->down = 1;
      if (b & (WPAD_CLASSIC_BUTTON_A | WPAD_CLASSIC_BUTTON_B)) in->jump = 1;
      if (b & WPAD_CLASSIC_BUTTON_PLUS) in->start = 1;
      if (b & (WPAD_CLASSIC_BUTTON_X | WPAD_CLASSIC_BUTTON_Y)) in->attack = 1;
      if (b & (WPAD_CLASSIC_BUTTON_ZL | WPAD_CLASSIC_BUTTON_ZR)) in->run = 1;
      if (b & (WPAD_CLASSIC_BUTTON_L | WPAD_CLASSIC_BUTTON_R)) in->twirl = 1;
      if (b & WPAD_CLASSIC_BUTTON_MINUS) in->pop = 1;
      sx = k.classic.leftStick.x; sy = k.classic.leftStick.y;
    } else if (k.extensionType == WPAD_EXT_NUNCHUK || k.extensionType == WPAD_EXT_MPLUS_NUNCHUK) {
      if (k.hold & WPAD_BUTTON_A) in->jump = 1;
      if (k.hold & WPAD_BUTTON_B) in->attack = 1;
      if (k.hold & WPAD_BUTTON_PLUS) in->start = 1;
      if (k.hold & WPAD_BUTTON_MINUS) in->pop = 1;
      if (k.nunchuk.hold & WPAD_NUNCHUK_BUTTON_Z) in->run = 1;
      if (k.nunchuk.hold & WPAD_NUNCHUK_BUTTON_C) in->twirl = 1;
      sx = k.nunchuk.stick.x; sy = k.nunchuk.stick.y;
    } else {
      // Wii Remote on its own, held sideways (D-pad on the left)
      uint32_t b = k.hold;
      if (b & WPAD_BUTTON_UP) in->left = 1;
      if (b & WPAD_BUTTON_DOWN) in->right = 1;
      if (b & WPAD_BUTTON_RIGHT) in->up = 1;
      if (b & WPAD_BUTTON_LEFT) in->down = 1;
      if (b & WPAD_BUTTON_2) in->jump = 1;
      if (b & WPAD_BUTTON_1) in->attack = 1;
      if (b & WPAD_BUTTON_PLUS) in->start = 1;
      if (b & WPAD_BUTTON_MINUS) in->pop = 1;
      if (b & WPAD_BUTTON_B) in->run = 1;   // the trigger on the back
      if (b & WPAD_BUTTON_A) in->twirl = 1;
    }
    if (sx < -0.35f) in->left = 1;
    if (sx > 0.35f) in->right = 1;
    if (sy < -0.5f) in->down = 1;
  }
}
#endif

// ---- drawing on the graphics chip: in a level the game leaves the background and the level picture
// out of its own picture (see game_layers), and they are drawn here from textures instead. Only the
// game's see-through picture on top (creatures, Dino, coins, the HUD) is sent across each frame.
#define LV_CHUNK 1024
typedef struct {
  SDL_Renderer* r;
  SDL_Texture *far, *near, *sun, *lv[48];
  const uint32_t* sun_px;
  int bg_ver, lvb_ver, nlv, lv_w[48], lv_h, premul;
  SDL_BlendMode pm;
} Layers;
static void layers_init(Layers* L, SDL_Renderer* r, SDL_Texture* over) {
  memset(L, 0, sizeof *L); L->r = r; L->bg_ver = L->lvb_ver = -1;
  // the game's picture is premultiplied where it is see-through: lay it on with that blend if the chip can
  L->pm = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
                                     SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
  L->premul = over && SDL_SetTextureBlendMode(over, L->pm) == 0;
  if (over) SDL_SetTextureBlendMode(over, SDL_BLENDMODE_NONE);
}
static SDL_Texture* make_tex(SDL_Renderer* r, int w, int h, const uint32_t* px, int pitch, SDL_BlendMode b) {
  SDL_Texture* t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
  if (!t) return NULL;
  SDL_SetTextureScaleMode(t, SDL_ScaleModeLinear);
  SDL_SetTextureBlendMode(t, b);
  SDL_UpdateTexture(t, NULL, px, pitch * 4);
  return t;
}
static void layers_sync(Layers* L, const GameLayers* g) {
  if (g->bg && (g->bg_ver != L->bg_ver || !L->far)) {
    if (L->far) SDL_DestroyTexture(L->far);
    if (L->near) SDL_DestroyTexture(L->near);
    L->far = make_tex(L->r, g->bgw, g->bgh, g->far, g->bgw, SDL_BLENDMODE_NONE);
    L->near = make_tex(L->r, g->bgw, g->bgh, g->near, g->bgw, SDL_BLENDMODE_BLEND);
    L->bg_ver = g->bg_ver;
  }
  if (g->bg && g->sun != L->sun_px) {
    if (L->sun) SDL_DestroyTexture(L->sun);
    L->sun = g->sun ? make_tex(L->r, g->sun_w, g->sun_h, g->sun, g->sun_w, SDL_BLENDMODE_BLEND) : NULL;
    L->sun_px = g->sun;
  }
  if (g->level && g->lvb_ver != L->lvb_ver) {
    for (int i = 0; i < L->nlv; i++) if (L->lv[i]) SDL_DestroyTexture(L->lv[i]);
    L->nlv = 0;
    for (int x = 0; x < g->lvb_w && L->nlv < 48; x += LV_CHUNK) {
      int w = g->lvb_w - x < LV_CHUNK ? g->lvb_w - x : LV_CHUNK;
      L->lv_w[L->nlv] = w;
      L->lv[L->nlv++] = make_tex(L->r, w, g->lvb_h, g->lvb + x, g->lvb_w, SDL_BLENDMODE_BLEND);
    }
    L->lv_h = g->lvb_h; L->lvb_ver = g->lvb_ver;
  }
}
// copy rows sy..sy+sh of a strip that repeats sideways, starting at column scroll, to screen rows dy..dy+dh
static void strip(SDL_Renderer* r, SDL_Texture* t, int bgw, int scroll, int sy, int sh, int dy, int dh) {
  if (!t || sh <= 0 || dh <= 0) return;
  int n1 = bgw - scroll; if (n1 > FB_W) n1 = FB_W;
  SDL_Rect s1 = { scroll, sy, n1, sh }, d1 = { 0, dy, n1, dh };
  SDL_RenderCopy(r, t, &s1, &d1);
  if (n1 < FB_W) { SDL_Rect s2 = { 0, sy, FB_W - n1, sh }, d2 = { n1, dy, FB_W - n1, dh }; SDL_RenderCopy(r, t, &s2, &d2); }
}
static void layers_draw(Layers* L, const GameLayers* g) {
  if (g->bg) {
    int y0 = g->far_yoff > 0 ? g->far_yoff : 0, y1 = g->far_yoff + g->bgh < FB_H ? g->far_yoff + g->bgh : FB_H;
    if (g->far_yoff > 0) strip(L->r, L->far, g->bgw, g->far_scroll, 0, 1, 0, g->far_yoff);   // the top row, stretched up
    strip(L->r, L->far, g->bgw, g->far_scroll, y0 - g->far_yoff, y1 - y0, y0, y1 - y0);
    if (y1 < FB_H) strip(L->r, L->far, g->bgw, g->far_scroll, g->bgh - 1, 1, y1, FB_H - y1);   // the bottom row, stretched down
    if (L->sun) { SDL_Rect d = { g->sun_x, g->sun_y, g->sun_w, g->sun_h }; SDL_RenderCopy(L->r, L->sun, NULL, &d); }
    y0 = g->near_yoff > 0 ? g->near_yoff : 0; y1 = g->near_yoff + g->bgh < FB_H ? g->near_yoff + g->bgh : FB_H;
    strip(L->r, L->near, g->bgw, g->near_scroll, y0 - g->near_yoff, y1 - y0, y0, y1 - y0);
    if (y1 < FB_H) strip(L->r, L->near, g->bgw, g->near_scroll, g->bgh - 1, 1, y1, FB_H - y1);
  }
  if (g->level) for (int i = 0; i < L->nlv; i++) {
    int x = i * LV_CHUNK - g->cx;
    if (!L->lv[i] || x >= FB_W || x + L->lv_w[i] <= 0) continue;
    SDL_Rect d = { x, -g->cy, L->lv_w[i], L->lv_h };
    SDL_RenderCopy(L->r, L->lv[i], NULL, &d);
  }
}
// send the game's picture and lay it on top. If the chip has no premultiplied blend, undo the premultiplying first.
static void overlay(Layers* L, SDL_Texture* tex, uint32_t* fb, int layered) {
  static uint32_t tmp[FB_W * FB_H];
  static int inv[256];
  const uint32_t* src = fb;
  if (layered && !L->premul) {
    if (!inv[1]) for (int a = 1; a < 256; a++) inv[a] = (255 << 16) / a;
    for (int i = 0; i < FB_W * FB_H; i++) {
      uint32_t c = fb[i], a = c >> 24;
      if (a == 0 || a == 255) { tmp[i] = c; continue; }
      uint32_t k = inv[a], rr = ((c >> 16) & 255) * k >> 16, gg = ((c >> 8) & 255) * k >> 16, bb = (c & 255) * k >> 16;
      tmp[i] = (a << 24) | ((rr > 255 ? 255 : rr) << 16) | ((gg > 255 ? 255 : gg) << 8) | (bb > 255 ? 255 : bb);
    }
    src = tmp;
  }
  SDL_UpdateTexture(tex, NULL, src, FB_W * 4);
  SDL_SetTextureBlendMode(tex, !layered ? SDL_BLENDMODE_NONE : L->premul ? L->pm : SDL_BLENDMODE_BLEND);
  SDL_RenderCopy(L->r, tex, NULL, NULL);
}

int main(int argc, char** argv) {
  (void)argc; (void)argv;
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
  Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO;
#ifndef __WIIU__
  flags |= SDL_INIT_GAMECONTROLLER;
#endif
  if (SDL_Init(flags) != 0) return 1;
#ifdef __WIIU__
  KPADInit();
  WPADEnableURCC(1);
  WPADEnableWiiRemote(1);
#endif
#ifdef __WIIU__
  // the game on the TV; the GamePad gets its own screen (background, coins, lives, level and the power-up slot)
  SDL_Window* win = SDL_CreateWindow("Amazing Dino", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_WIIU_TV_ONLY);
  SDL_Window* pwin = SDL_CreateWindow("Amazing Dino pad", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 854, 480, SDL_WINDOW_WIIU_GAMEPAD_ONLY);
#else
#ifdef DINO_SELFTEST
  SDL_Window* win = SDL_CreateWindow("Amazing Dino", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, FB_W, FB_H, 0);
#else
  SDL_Window* win = SDL_CreateWindow("Amazing Dino", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
#endif
  SDL_Window* pwin = NULL;
#endif
  SDL_Renderer* r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_RenderSetLogicalSize(r, FB_W, FB_H);   // everything is placed in the game's 640x360 pixels and stretched
  SDL_Texture* tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
  SDL_SetTextureScaleMode(tex, SDL_ScaleModeLinear);   // smooth when stretched to the TV, not blocky
  // two of them, used in turn, so sending this frame never waits for the chip to finish with the last one
  SDL_Texture* tex2 = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
  if (tex2) SDL_SetTextureScaleMode(tex2, SDL_ScaleModeLinear); else tex2 = tex;
  int flip = 0;
  SDL_Renderer* pr = pwin ? SDL_CreateRenderer(pwin, -1, SDL_RENDERER_ACCELERATED) : NULL;
  SDL_Texture* ptex = pr ? SDL_CreateTexture(pr, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H) : NULL;
  if (ptex) SDL_SetTextureScaleMode(ptex, SDL_ScaleModeLinear);
  static uint32_t pfb[FB_W * FB_H];
  int pad_n = 0;
  if (!ptex) {   // no second screen after all: show the game on both, as before
    if (pwin) SDL_DestroyWindow(pwin);
    pwin = NULL;
  } else { g_pad_screen = 1; SDL_RenderSetLogicalSize(pr, FB_W, FB_H); }
  static Layers TVL, PADL;
  layers_init(&TVL, r, tex);
  if (pr) layers_init(&PADL, pr, ptex);
  g_gpu = getenv("DINO_CPU") ? 0 : 1;

  game_init();

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = 48000; want.format = AUDIO_S16SYS; want.channels = 2; want.samples = 1024; want.callback = audio_cb;
  g_audio = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
  if (g_audio) { g_rate = have.freq; SDL_PauseAudioDevice(g_audio, 0); }

#ifndef __WIIU__
  SDL_GameController* pad = NULL;
  for (int i = 0; i < SDL_NumJoysticks() && !pad; i++) if (SDL_IsGameController(i)) pad = SDL_GameControllerOpen(i);
#endif

  static uint32_t fb[FB_W * FB_H];
  Uint64 freq = SDL_GetPerformanceFrequency(), last = SDL_GetPerformanceCounter();
  double acc = 0, tick = 1.0 / 60.0;
  int running = 1;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = 0;
#ifndef __WIIU__
      if (e.type == SDL_CONTROLLERDEVICEADDED && !pad) pad = SDL_GameControllerOpen(e.cdevice.which);
#endif
    }
    Input in;
    memset(&in, 0, sizeof in);
#ifdef __WIIU__
    wiiu_read(&in);
#else
    const Uint8* ks = SDL_GetKeyboardState(NULL);
    in.left = ks[SDL_SCANCODE_LEFT] || ks[SDL_SCANCODE_A];
    in.right = ks[SDL_SCANCODE_RIGHT] || ks[SDL_SCANCODE_D];
    in.up = ks[SDL_SCANCODE_UP] || ks[SDL_SCANCODE_W];
    in.down = ks[SDL_SCANCODE_DOWN] || ks[SDL_SCANCODE_S];
    in.jump = ks[SDL_SCANCODE_SPACE] || ks[SDL_SCANCODE_Z];
    in.start = ks[SDL_SCANCODE_RETURN] || ks[SDL_SCANCODE_ESCAPE];
    in.attack = ks[SDL_SCANCODE_X] || ks[SDL_SCANCODE_LSHIFT];
    in.run = ks[SDL_SCANCODE_C] || ks[SDL_SCANCODE_RSHIFT];
    in.twirl = ks[SDL_SCANCODE_Q] || ks[SDL_SCANCODE_E];
    in.pop = ks[SDL_SCANCODE_V] || ks[SDL_SCANCODE_TAB];
    if (pad) {
      in.left |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT) || SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) < -12000;
      in.right |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) > 12000;
      in.down |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
      in.jump |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A) || SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B);
      in.start |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START);
      in.attack |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_X) || SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_Y);
      in.run |= SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 12000 || SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 12000;
      in.twirl |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) || SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
      in.pop |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_BACK);
    }
#endif
#ifdef DINO_SELFTEST   // a desktop check of the drawing: walk into level 1, save a picture, stop
    static int st_f; st_f++;
    memset(&in, 0, sizeof in);
    in.jump = st_f == 40 || st_f == 80 || st_f == 170;
    in.right = (st_f > 130 && st_f < 134) || (st_f > 340 && st_f < 420);
    acc = tick;
#endif
    Uint64 now = SDL_GetPerformanceCounter();
    acc += (double)(now - last) / freq; last = now;
#ifdef DINO_SELFTEST
    acc = tick;
#endif
    if (acc > 0.1) acc = 0.1;
    int steps = 0;
    while (acc >= tick && steps < 4) {
      if (g_audio) SDL_LockAudioDevice(g_audio);   // the music reads game state
      game_update(&in);
      if (g_audio) SDL_UnlockAudioDevice(g_audio);
      acc -= tick; steps++;
    }
    Uint64 d0 = SDL_GetPerformanceCounter();
    game_draw(fb);
    // if drawing keeps taking longer than a 60th of a second, ask the game to draw more simply
    // (one long frame, such as loading a level, is not counted, and it switches back once things speed up again,
    // so Dino isn't left drawn the rough way for the rest of the game)
    static double slow_avg; static int slow_n;
    double dt = (double)(SDL_GetPerformanceCounter() - d0) / freq;
    if (dt > 0.03) dt = 0.03;
    slow_avg = slow_avg * 0.97 + dt * 0.03;
    if (++slow_n > 180) { if (slow_avg > 0.014) g_lowq = 1; else if (slow_avg < 0.010) g_lowq = 0; }
    {
      GameLayers g; game_layers(&g, 0);
      layers_sync(&TVL, &g);
      SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
      SDL_RenderClear(r);
      layers_draw(&TVL, &g);
      overlay(&TVL, (flip ^= 1) ? tex2 : tex, fb, g.bg || g.level);
#ifdef DINO_SELFTEST
      if (st_f == 470) {
        static uint32_t shot[FB_W * FB_H];
        SDL_Rect all = { 0, 0, FB_W, FB_H };
        int ok = SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_ARGB8888, shot, FB_W * 4);
        (void)all;
        FILE* fp = fopen(getenv("DINO_SHOT") ? getenv("DINO_SHOT") : "shot.ppm", "wb");
        fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H);
        for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { shot[i] >> 16, shot[i] >> 8, shot[i] }; fwrite(c, 1, 3, fp); }
        fclose(fp);
        printf("read %d, layers bg %d level %d, premultiplied blend %d\n", ok, g.bg, g.level, TVL.premul);
        running = 0;
      }
#endif
      SDL_RenderPresent(r);
    }
    // the GamePad screen changes slowly, so it is only drawn every fourth frame, leaving the time for the TV
    if (ptex && ++pad_n % (g_lowq ? 8 : 4) == 0) {
      game_draw_pad(pfb);
      GameLayers g; game_layers(&g, 1);
      layers_sync(&PADL, &g);
      SDL_SetRenderDrawColor(pr, 0, 0, 0, 255);
      SDL_RenderClear(pr);
      layers_draw(&PADL, &g);
      overlay(&PADL, ptex, pfb, g.bg);
      SDL_RenderPresent(pr);
    }
  }
  if (g_audio) SDL_CloseAudioDevice(g_audio);
  SDL_Quit();
  return 0;
}
