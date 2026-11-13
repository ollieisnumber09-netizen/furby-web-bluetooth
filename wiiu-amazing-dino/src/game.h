// Amazing Dino: an original little platformer. The game itself is plain C with no
// platform code: it draws into a 640x360 pixel buffer and makes its own sound, so the same
// code runs on the Wii U and in the desktop test harness.
#pragma once
#include <stdint.h>

#define SCR_W 320
#define SCR_H 180
// the picture the game draws is twice that: 640x360, smooth shaded art
#define FB_W 640
#define FB_H 360

typedef struct {
  int left, right, up, down;
  int jump;   // held
  int start;  // held
  int attack; // held
  int run;    // held: any back trigger. Hold to sprint; a fresh press in the air twirls
  int pop;    // held: drop the power-up waiting in the slot (GamePad touch or -)
} Input;

void game_init(void);
void game_update(const Input* in);       // one 60 Hz tick
void game_draw(uint32_t* fb);            // fb is FB_W*FB_H, 0xAARRGGBB
// the second screen (the Wii U GamePad): the level's background, coins, lives, level name and the power-up slot
void game_draw_pad(uint32_t* fb);
int game_pad_slot_hit(int x, int y);     // x, y in FB_W x FB_H pixels: is that on the power-up slot?

// sound: the platform calls synth_render from its audio callback. The game calls plat_sfx,
// which the platform implements (taking its audio lock, then calling synth_trigger).
void synth_render(int16_t* out, int frames, int rate);   // interleaved stereo
void synth_trigger(int id);
void plat_sfx(int id);

// The platform sets this to 1 if frames are taking too long; the game then draws a little more simply.
extern int g_lowq;
// The platform sets this to 1 when the GamePad shows game_draw_pad, so the TV leaves the slot off its HUD.
extern int g_pad_screen;
