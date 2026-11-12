// Amazing Dino: game logic, drawing and sound. See game.h.
#include "game.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define T 16                 // tile size in pixels
#define MAXW 340
#define MAXH 40

// ============================================================ sprites
// Each sprite is 16 rows of 16 characters. '.' is see-through; other letters are palette colours.
static int pal_snow;   // set while drawing in a snow level: beetles turn icy blue
static uint32_t pal(char c) {
  if (pal_snow) {
    if (c == 'g') return 0xFF8FC7E8;
    if (c == 'G') return 0xFF5C93BD;
    if (c == 'h') return 0xFFE3F5FF;
  }
  switch (c) {
    case 'k': return 0xFF2B1A12;  // outline
    case 'b': return 0xFF8B5A2B;  // fur
    case 'd': return 0xFF6B3F1D;  // dark fur
    case 't': return 0xFFF2C99A;  // face / belly
    case 'p': return 0xFFFF9EB5;  // pink
    case 'w': return 0xFFFFFFFF;
    case 'e': return 0xFF1A1A1A;
    case 'r': return 0xFFD8463C;
    case 'y': return 0xFFF7D038;
    case 'Y': return 0xFFC9A227;
    case 'g': return 0xFF3FA34D;
    case 'G': return 0xFF24723A;
    case 'h': return 0xFF9BE37A;
    case 'z': return 0xFF222222;
    case 'l': return 0xFFCFE8FF;
    case 's': return 0xFFB0B4C0;
    default: return 0;
  }
}

static const char* SPR_BEETLE[2][16] = { {
  "................", "................", "................", "................",
  "................", "................", "................",
  ".....kkkkkk.....",
  "...kkghhgggkk...",
  "..kgghhgggggGk..",
  ".kggggggggggGGk.",
  ".kGgggggggggGGk.",
  "kwekGGGGGGGGGGk.",
  "kkkkkkkkkkkkkkk.",
  "..k.k...k.k..k..",
  ".k...k.k...k.k..",
}, {
  "................", "................", "................", "................",
  "................", "................", "................",
  ".....kkkkkk.....",
  "...kkghhgggkk...",
  "..kgghhgggggGk..",
  ".kggggggggggGGk.",
  ".kGgggggggggGGk.",
  "kwekGGGGGGGGGGk.",
  "kkkkkkkkkkkkkkk.",
  "..k.k..k.k..k...",
  "...k.k...k.k.k..",
} };
static const char* SPR_BEE[2][16] = { {
  "................", "................", "................", "................",
  "................",
  "......ll.ll.....",
  ".....lwllwl.....",
  "...kkkkkkkkk....",
  "..kyyzyyzyyyk...",
  ".kweyzyyzyyyyk..",
  ".kkyyzyyzyyyyk..",
  "..kyyzyyzyyykk..",
  "...kkkkkkkkk.kk.",
  "................", "................", "................",
}, {
  "................", "................", "................", "................",
  "................",
  "................",
  "....llwllwll....",
  "...kkkkkkkkk....",
  "..kyyzyyzyyyk...",
  ".kweyzyyzyyyyk..",
  ".kkyyzyyzyyyyk..",
  "..kyyzyyzyyykk..",
  "...kkkkkkkkk.kk.",
  "................", "................", "................",
} };
static const char* SPR_WING[16] = {
  "................", "................", "................",
  "..........kk....",
  "........kkwwk...",
  "......kkwwwwk...",
  "....kkwwwwwwk...",
  "...kwwwwwwwwk...",
  "..kwwwwwwwwk....",
  "..kwwlwwlwk.....",
  "...kwlwwlk......",
  "....kwlwk.......",
  ".....kkk........",
  "................", "................", "................",
};
static const char* SPR_SWORD[16] = {
  "................",
  ".............kk.",
  "............kwsk",
  "...........kwsk.",
  "..........kwsk..",
  ".........kwsk...",
  "........kwsk....",
  ".......kwsk.....",
  "..kk..kwsk......",
  "..kykkwsk.......",
  "...kyysk........",
  "....kyyk........",
  "...kkkyyk.......",
  "..krrk.kyk......",
  "..krk...k.......",
  "...k............",
};
static const char* SPR_BANANA[16] = {
  "................", "................", "................",
  "..........kk....",
  ".........kYk....",
  "........kyyk....",
  ".......kyyyk....",
  "......kyyyk.....",
  "....kkyyyyk.....",
  "..kkyyyyyk......",
  ".kYyyyyykk......",
  "..kkkkkk........",
  "................", "................", "................", "................",
};

#include "dino_sprites.h"

static uint32_t* FB;
static void put(int x, int y, uint32_t c) {
  if ((unsigned)x < SCR_W && (unsigned)y < SCR_H) { uint32_t* d = FB + y * 2 * FB_W + x * 2; d[0] = d[1] = d[FB_W] = d[FB_W + 1] = c; }
}
static void fill(int x, int y, int w, int h, uint32_t c) {
  int x0 = x * 2, y0 = y * 2, x1 = (x + w) * 2, y1 = (y + h) * 2;
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > FB_W) x1 = FB_W;
  if (y1 > FB_H) y1 = FB_H;
  for (int j = y0; j < y1; j++) for (int i = x0; i < x1; i++) FB[j * FB_W + i] = c;
}
static uint32_t blend(uint32_t a, uint32_t b, float t) {
  int ar = (a >> 16) & 255, ag = (a >> 8) & 255, ab = a & 255;
  int br = (b >> 16) & 255, bg = (b >> 8) & 255, bb = b & 255;
  return 0xFF000000 | ((int)(ar + (br - ar) * t) << 16) | ((int)(ag + (bg - ag) * t) << 8) | (int)(ab + (bb - ab) * t);
}
static void darken(int x, int y, int w, int h, float t) {
  int x0 = x * 2, y0 = y * 2, x1 = (x + w) * 2, y1 = (y + h) * 2;
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > FB_W) x1 = FB_W;
  if (y1 > FB_H) y1 = FB_H;
  int k = (int)((1 - t) * 256);
  for (int j = y0; j < y1; j++) for (int i = x0; i < x1; i++) {
    uint32_t c = FB[j * FB_W + i];
    FB[j * FB_W + i] = 0xFF000000 | ((((c >> 16) & 255) * k >> 8) << 16) | ((((c >> 8) & 255) * k >> 8) << 8) | ((c & 255) * k >> 8);
  }
}
static void sprite(const char* const* s, int x, int y, int flip) {
  for (int j = 0; j < 16; j++) for (int i = 0; i < 16; i++) {
    char c = s[j][flip ? 15 - i : i];
    if (c != '.') put(x + i, y + j, pal(c));
  }
}
static void sprite_n(const char* const* s, int n, int x, int y, int flip) {
  for (int j = 0; j < n; j++) for (int i = 0; i < n; i++) {
    char c = s[j][flip ? n - 1 - i : i];
    if (c != '.') put(x + i, y + j, pal(c));
  }
}
static void sprite_flat(const char* const* s, int x, int y, int flip, uint32_t col) {
  for (int j = 0; j < 16; j++) for (int i = 0; i < 16; i++)
    if (s[j][flip ? 15 - i : i] != '.') put(x + i, y + j, col);
}
static uint32_t hash2(int x, int y) {
  uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u;
  h = (h ^ (h >> 13)) * 1274126177u;
  return h ^ (h >> 16);
}

#include "hdbase.h"

// ============================================================ font (3x5 cells, drawn as round strokes)
static const struct { char c; const char* r; } FONT[] = {
  {'A', ".X.X.XXXXX.XX.X"}, {'B', "XX.X.XXX.X.XXX."}, {'C', ".XXX..X..X...XX"}, {'D', "XX.X.XX.XX.XXX."},
  {'E', "XXXX..XX.X..XXX"}, {'F', "XXXX..XX.X..X.."}, {'G', ".XXX..X.XX.X.XX"}, {'H', "X.XX.XXXXX.XX.X"},
  {'I', "XXX.X..X..X.XXX"}, {'J', "..X..X..XX.X.X."}, {'K', "X.XX.XXX.X.XX.X"}, {'L', "X..X..X..X..XXX"},
  {'M', "X.XXXXXXXX.XX.X"}, {'N', "XX.X.XX.XX.XX.X"}, {'O', ".X.X.XX.XX.X.X."}, {'P', "XX.X.XXX.X..X.."},
  {'Q', ".X.X.XX.XXX..XX"}, {'R', "XX.X.XXX.X.XX.X"}, {'S', ".XXX...X...XXX."}, {'T', "XXX.X..X..X..X."},
  {'U', "X.XX.XX.XX.XXXX"}, {'V', "X.XX.XX.XX.X.X."}, {'W', "X.XX.XXXXXXXX.X"}, {'X', "X.XX.X.X.X.XX.X"},
  {'Y', "X.XX.X.X..X..X."}, {'Z', "XXX..X.X.X..XXX"},
  {'0', "XXXX.XX.XX.XXXX"}, {'1', ".X.XX..X..X.XXX"}, {'2', "XX...X.X.X..XXX"}, {'3', "XX...X.X...XXX."},
  {'4', "X.XX.XXXX..X..X"}, {'5', "XXXX..XX...XXX."}, {'6', ".XXX..XXXX.XXXX"}, {'7', "XXX..X.X..X..X."},
  {'8', "XXXX.XXXXX.XXXX"}, {'9', "XXXX.XXXX..XXX."},
  {':', "....X.....X...."}, {'!', ".X..X..X.....X."}, {'-', "......XXX......"}, {'.', ".............X."},
  {'x', "...X.X.X.X.X..."}, {'/', "..X..X.X.X..X.."}, {'\'', ".X..X.........."}, {'+', "....X.XXX.X...."},
  {',', "..........X.X.."}, {'?', "XX...X.X.....X."},
};
static int glyph_idx(char c) {
  if (c >= 'a' && c <= 'z' && c != 'x') c -= 32;
  for (unsigned i = 0; i < sizeof FONT / sizeof FONT[0]; i++) if (FONT[i].c == c) return (int)i;
  return -1;
}
static int text_w(const char* s, int sc) { return (int)strlen(s) * 4 * sc - sc; }
// each letter is made once per size (round strokes through its cells, with an outline) and kept
static Bmp GLY[2][11][64];
static int gly_pad(int sc, int ol) { return ol ? (int)ceilf(sc == 1 ? 1.0f : 0.8f + sc * 0.3f) + 1 : 1; }
static const Bmp* glyph_bmp(int gi, int sc, int ol) {
  Bmp* b = &GLY[ol][sc][gi];
  if (b->px) return b;
  float c = sc * 2.0f, r = sc == 1 ? 1.0f : c * 0.41f, o = ol ? (sc == 1 ? 1.0f : 0.8f + sc * 0.3f) : 0;
  int pad = gly_pad(sc, ol);
  *b = bmp_new((int)(3 * c) + pad * 2, (int)(5 * c) + pad * 2);
  const char* g = FONT[gi].r;
#define ON(i, j) ((i) >= 0 && (i) < 3 && (j) >= 0 && (j) < 5 && g[(j) * 3 + (i)] == 'X')
  for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) {
    float fx = x + 0.5f - pad, fy = y + 0.5f - pad, d = 99;
    for (int j = 0; j < 5; j++) for (int i = 0; i < 3; i++) {
      if (!ON(i, j)) continue;
      float cx = (i + 0.5f) * c, cy = (j + 0.5f) * c;
      d = minf(d, sd_circle(fx, fy, cx, cy, r));
      if (ON(i + 1, j)) d = minf(d, sd_seg(fx, fy, cx, cy, cx + c, cy) - r);
      if (ON(i, j + 1)) d = minf(d, sd_seg(fx, fy, cx, cy, cx, cy + c) - r);
      if (ON(i + 1, j + 1) && !ON(i + 1, j) && !ON(i, j + 1)) d = minf(d, sd_seg(fx, fy, cx, cy, cx + c, cy + c) - r);
      if (ON(i - 1, j + 1) && !ON(i - 1, j) && !ON(i, j + 1)) d = minf(d, sd_seg(fx, fy, cx, cy, cx - c, cy + c) - r);
    }
    float tot = cov(d - o), fl = cov(d);
    if (tot <= 0) continue;
    b->px[y * b->w + x] = ((uint32_t)(tot * 255) << 24) | ((uint32_t)(fl / tot * 255) << 16);
  }
#undef ON
  return b;
}
static void text_hd(const char* s, int x, int y, int sc, uint32_t col, int ol) {
  if (sc > 10) sc = 10;
  int pad = gly_pad(sc, ol);
  for (; *s; s++, x += 4 * sc) {
    int gi = glyph_idx(*s);
    if (gi < 0) continue;
    const Bmp* b = glyph_bmp(gi, sc, ol);
    int X = x * 2 - pad, Y = y * 2 - pad;
    for (int j = 0; j < b->h; j++) {
      if ((unsigned)(Y + j) >= FB_H) continue;
      for (int i = 0; i < b->w; i++) {
        uint32_t p = b->px[j * b->w + i];
        if (!(p >> 24) || (unsigned)(X + i) >= FB_W) continue;
        uint32_t* d = &FB[(Y + j) * FB_W + X + i];
        { uint32_t f = (p >> 16) & 255, a = p >> 24; *d = lerpc(*d, lerpc(0xFF1B1020, col, f + (f >> 7)), a + (a >> 7)); }
      }
    }
  }
}
static void text_raw(const char* s, int x, int y, int sc, uint32_t col) { text_hd(s, x, y, sc, col, 0); }
static void text(const char* s, int x, int y, int sc, uint32_t col) { text_hd(s, x, y, sc, col, 1); }
static void text_c(const char* s, int y, int sc, uint32_t col) { text(s, (SCR_W - text_w(s, sc)) / 2, y, sc, col); }

// ============================================================ levels
typedef struct { char op; short x, y, w, h; } Op;
typedef struct {
  const char* name;
  int w, h;
  uint32_t sky0, sky1, far, nearc;
  int theme;   // 0 jungle, 1 temple, 2 cave, 3 sky, 4 waterfalls, 5 snow, 6 ice cave, 7 snow at night, 8 desert
  const Op* ops;
  const char* signs[4];
} Level;
// more ops: m moving platform (w: how many tiles it travels, h: 1 = up and down)  k cracked blocks w*h (the helmet smashes them)
//           h / H a pretend wall of earth / stone w*h (walk through it)  _ dig out a room w*h
//           f frog  y spiky ball  p seed-spitting flower  a bat   B boxes: w = 3 helmet, 4 star

// ops: g ground (dirt to the bottom)  G ground (stone to the bottom)  x stone block w*h
//      = one-way plank  ^ spikes  o bananas  b beetle  z bee  s spring  c checkpoint
//      F goal flag  P start  i sign (w = which text)
static const Op L1[] = {
  {'B', 12, 7, 0}, {'B', 25, 7, 1}, {'B', 44, 5, 0}, {'B', 59, 7, 0}, {'B', 88, 7, 2}, {'B', 113, 7, 0},
  {'g', 0, 11, 30}, {'P', 3, 10}, {'i', 6, 10, 0}, {'o', 9, 8, 3}, {'b', 20, 10}, {'i', 16, 10, 1},
  {'o', 29, 8, 5},
  {'g', 33, 11, 18}, {'=', 36, 8, 4}, {'o', 36, 7, 4}, {'g', 42, 9, 4}, {'o', 42, 7, 4}, {'b', 48, 10},
  {'g', 51, 12, 3}, {'^', 51, 11, 3},
  {'g', 54, 11, 26}, {'c', 57, 10}, {'x', 62, 9, 2, 2}, {'b', 67, 10}, {'=', 69, 8, 5}, {'o', 69, 7, 5},
  {'s', 77, 10}, {'i', 74, 10, 2}, {'o', 80, 4, 4},
  {'g', 84, 11, 20}, {'b', 89, 10}, {'=', 92, 8, 4}, {'o', 92, 7, 4}, {'z', 97, 6},
  {'g', 104, 9, 6}, {'o', 105, 7, 4},
  {'g', 110, 11, 30}, {'b', 116, 10}, {'o', 120, 8, 6}, {'F', 132, 10},
  {0}
};
static const Op L2[] = {
  {'B', 4, 19, 0}, {'B', 9, 19, 2}, {'B', 33, 6, 0}, {'B', 83, 14, 1},
  {'g', 0, 23, 22}, {'P', 2, 22}, {'i', 6, 22, 0},
  {'x', 15, 12, 1, 9}, {'x', 20, 12, 2, 11},
  {'o', 18, 20, 1}, {'o', 17, 17, 1}, {'o', 18, 14, 1},
  {'=', 24, 12, 5}, {'b', 26, 11},
  {'=', 31, 10, 4}, {'o', 31, 9, 4},
  {'=', 37, 12, 6}, {'z', 40, 9},
  {'=', 45, 14, 3},
  {'x', 49, 13, 3, 1}, {'s', 50, 12}, {'o', 49, 5, 3},
  {'=', 54, 8, 6}, {'b', 57, 7},
  {'=', 62, 10, 5}, {'c', 64, 9},
  {'=', 69, 12, 5}, {'z', 71, 9},
  {'x', 76, 13, 4, 1}, {'o', 76, 12, 4},
  {'g', 80, 18, 30}, {'b', 85, 17}, {'^', 88, 17, 2},
  {'x', 92, 6, 1, 10}, {'x', 96, 8, 4, 10}, {'o', 94, 14, 1}, {'o', 94, 11, 1}, {'o', 94, 8, 1},
  {'i', 90, 17, 1},
  {'F', 98, 7},
  {0}
};
static const Op L3[] = {
  {'B', 6, 9, 2}, {'B', 35, 9, 0}, {'B', 62, 9, 1}, {'B', 108, 9, 0},
  {'G', 0, 13, 16}, {'P', 2, 12}, {'b', 10, 12},
  {'G', 18, 11, 5}, {'o', 18, 9, 5},
  {'G', 23, 13, 10}, {'^', 25, 12, 3}, {'z', 28, 9},
  {'G', 33, 13, 14}, {'x', 38, 10, 2, 3}, {'=', 41, 9, 4}, {'o', 41, 8, 4}, {'b', 44, 12},
  {'G', 50, 13, 3}, {'s', 51, 12}, {'o', 53, 5, 5},
  {'G', 58, 13, 20}, {'c', 60, 12}, {'x', 66, 0, 10, 10}, {'b', 71, 12}, {'o', 67, 12, 8},
  {'G', 80, 12, 3},
  {'G', 83, 13, 12}, {'^', 85, 12, 2}, {'^', 90, 12, 2}, {'=', 84, 9, 3}, {'=', 89, 8, 3}, {'o', 89, 7, 3}, {'z', 87, 6},
  {'G', 95, 13, 25}, {'x', 96, 2, 1, 8}, {'x', 100, 4, 2, 9}, {'o', 98, 9, 1}, {'o', 98, 6, 1},
  {'b', 106, 12}, {'b', 112, 12}, {'z', 109, 8}, {'=', 114, 9, 4}, {'o', 114, 8, 4},
  {'G', 123, 11, 4}, {'o', 123, 9, 4},
  {'G', 129, 13, 21}, {'b', 134, 12}, {'F', 142, 12},
  {0}
};
static const Op L4[] = {   // Misty Falls: climb up and up
  {'B', 5, 29, 1}, {'B', 34, 19, 0},
  {'g', 0, 33, 40}, {'P', 3, 32}, {'i', 6, 32, 0},
  {'=', 8, 31, 4}, {'=', 14, 29, 4}, {'o', 14, 28, 4}, {'=', 20, 27, 4},
  {'=', 26, 25, 4}, {'b', 27, 24}, {'=', 32, 23, 5}, {'o', 32, 22, 5},
  {'=', 26, 21, 4}, {'z', 23, 18}, {'=', 20, 19, 4}, {'c', 21, 18},
  {'=', 13, 17, 4}, {'x', 7, 16, 3, 1}, {'s', 8, 15}, {'o', 8, 12, 1}, {'o', 8, 9, 1},
  {'=', 2, 10, 5}, {'o', 2, 9, 5}, {'=', 9, 8, 4}, {'b', 10, 7}, {'=', 15, 6, 4},
  {'=', 21, 4, 4}, {'o', 21, 3, 4}, {'x', 27, 4, 13, 1}, {'z', 31, 1}, {'F', 36, 3},
  {0}
};
static const Op L5[] = {   // Beetle Cavern: lots of beetles under a low roof
  {'B', 5, 9, 2}, {'B', 36, 9, 0}, {'B', 88, 9, 1}, {'B', 115, 9, 0},
  {'x', 0, 0, 130, 2},
  {'G', 0, 13, 20}, {'P', 2, 12}, {'b', 8, 12}, {'b', 14, 12},
  {'x', 20, 2, 4, 6}, {'G', 20, 13, 10}, {'^', 24, 12, 2}, {'b', 27, 12},
  {'G', 33, 13, 6}, {'z', 35, 9}, {'o', 30, 10, 3},
  {'G', 39, 11, 4}, {'o', 39, 9, 4},
  {'G', 43, 13, 15}, {'=', 45, 10, 4}, {'o', 45, 9, 4}, {'b', 50, 12}, {'b', 54, 12}, {'x', 47, 2, 6, 5},
  {'c', 56, 12},
  {'G', 61, 13, 4}, {'s', 62, 12}, {'o', 65, 6, 5},
  {'G', 70, 13, 20}, {'^', 74, 12, 3}, {'^', 80, 12, 3}, {'=', 73, 9, 5}, {'=', 79, 9, 5}, {'b', 75, 8}, {'z', 84, 8}, {'o', 78, 8, 1},
  {'G', 90, 10, 4}, {'o', 90, 8, 4},
  {'G', 94, 13, 15}, {'b', 97, 12}, {'b', 101, 12}, {'b', 105, 12}, {'=', 97, 9, 10}, {'o', 98, 8, 8},
  {'G', 112, 13, 18}, {'b', 116, 12}, {'F', 124, 12},
  {0}
};
static const Op L6[] = {   // Sky Bridge: planks and floating islands high above the clouds
  {'B', 4, 6, 1}, {'B', 30, 6, 0}, {'B', 52, 6, 2}, {'B', 108, 6, 0}, {'B', 140, 6, 0},
  {'x', 0, 10, 8, 2}, {'P', 2, 9},
  {'=', 10, 10, 4}, {'=', 16, 9, 4}, {'z', 18, 6}, {'=', 22, 10, 3},
  {'x', 27, 10, 5, 2}, {'b', 29, 9}, {'o', 27, 8, 5},
  {'=', 34, 8, 3}, {'=', 39, 6, 3}, {'o', 39, 5, 3}, {'=', 44, 8, 3}, {'z', 42, 3},
  {'x', 49, 10, 6, 2}, {'c', 51, 9},
  {'x', 57, 11, 2, 1}, {'s', 58, 10}, {'=', 64, 4, 5}, {'o', 64, 3, 5},
  {'=', 71, 6, 4}, {'b', 72, 5}, {'=', 77, 8, 4}, {'z', 80, 5},
  {'x', 84, 10, 4, 2}, {'=', 90, 9, 3}, {'=', 95, 7, 3}, {'o', 95, 6, 3}, {'=', 100, 9, 3}, {'z', 98, 4},
  {'x', 105, 10, 8, 2}, {'b', 107, 9}, {'b', 110, 9},
  {'=', 115, 8, 4}, {'=', 121, 10, 4}, {'z', 123, 7}, {'=', 127, 8, 4}, {'o', 127, 7, 4},
  {'x', 133, 10, 17, 2}, {'b', 137, 9}, {'F', 144, 9},
  {0}
};
static const Op L7[] = {   // Snowy Trail: first ice
  {'g', 0, 11, 24}, {'P', 3, 10}, {'i', 6, 10, 0}, {'B', 10, 7, 0}, {'b', 16, 10},
  {'I', 24, 11, 10, 3}, {'o', 26, 9, 6},
  {'g', 34, 11, 10}, {'b', 38, 10}, {'B', 40, 7, 1},
  {'g', 47, 11, 8}, {'x', 50, 9, 2, 2}, {'o', 50, 7, 2},
  {'I', 55, 11, 8, 3}, {'g', 63, 11, 12}, {'^', 65, 10, 2}, {'c', 69, 10},
  {'=', 78, 9, 4}, {'o', 78, 8, 4},
  {'g', 84, 11, 14}, {'b', 88, 10}, {'b', 93, 10}, {'z', 90, 6}, {'B', 95, 7, 2},
  {'I', 98, 11, 12, 3}, {'o', 100, 9, 8},
  {'g', 110, 9, 5}, {'o', 110, 7, 5},
  {'g', 115, 11, 25}, {'b', 120, 10}, {'F', 132, 10},
  {0}
};
static const Op L8[] = {   // Frozen Lake: long slides and ice floes
  {'g', 0, 11, 12}, {'P', 2, 10}, {'B', 6, 7, 0},
  {'I', 12, 11, 20, 3}, {'b', 18, 10}, {'b', 26, 10}, {'o', 14, 9, 4},
  {'I', 35, 11, 5, 3}, {'I', 43, 11, 5, 3}, {'z', 41, 7}, {'o', 36, 9, 3}, {'o', 44, 9, 3},
  {'g', 51, 11, 10}, {'c', 54, 10}, {'B', 57, 7, 1}, {'s', 59, 10}, {'o', 62, 4, 4},
  {'I', 67, 11, 14, 3}, {'b', 71, 10}, {'x', 76, 9, 2, 2},
  {'=', 84, 9, 3}, {'=', 89, 7, 3}, {'o', 89, 6, 3}, {'=', 94, 9, 3}, {'z', 92, 4},
  {'I', 99, 11, 16, 3}, {'b', 104, 10}, {'b', 110, 10}, {'B', 107, 7, 2},
  {'g', 118, 9, 4}, {'o', 118, 7, 4},
  {'g', 125, 11, 25}, {'b', 130, 10}, {'F', 142, 10},
  {0}
};
static const Op L9[] = {   // Icicle Cave: icicles drop from the roof
  {'x', 0, 0, 130, 5},
  {'G', 0, 13, 18}, {'P', 2, 12}, {'i', 5, 12, 0}, {'v', 10, 5}, {'v', 13, 5}, {'b', 15, 12},
  {'G', 21, 13, 12}, {'v', 23, 5}, {'v', 26, 5}, {'v', 29, 5}, {'o', 22, 11, 8}, {'B', 31, 9, 0},
  {'I', 33, 13, 10, 3}, {'x', 36, 5, 3, 5}, {'b', 40, 12},
  {'G', 46, 11, 4}, {'o', 46, 9, 4},
  {'G', 53, 13, 16}, {'c', 55, 12}, {'v', 58, 5}, {'v', 61, 5}, {'v', 65, 5}, {'b', 60, 12}, {'b', 66, 12}, {'B', 63, 9, 1},
  {'s', 67, 12}, {'o', 70, 6, 4},
  {'G', 75, 13, 6}, {'v', 77, 5},
  {'G', 81, 13, 14}, {'^', 83, 12, 2}, {'^', 88, 12, 2}, {'=', 82, 9, 4}, {'=', 87, 9, 4}, {'z', 91, 8},
  {'I', 95, 13, 14, 3}, {'v', 98, 5}, {'v', 102, 5}, {'v', 106, 5}, {'b', 100, 12},
  {'G', 112, 13, 18}, {'b', 118, 12}, {'B', 115, 9, 2}, {'F', 124, 12},
  {0}
};
static const Op L10[] = {   // Northern Lights
  {'g', 0, 13, 14}, {'P', 2, 12}, {'B', 7, 9, 1}, {'b', 10, 12},
  {'g', 14, 11, 5}, {'o', 14, 9, 5},
  {'I', 22, 13, 10, 3}, {'z', 26, 9},
  {'g', 32, 13, 20}, {'B', 34, 9, 0}, {'x', 36, 4, 1, 7}, {'x', 40, 6, 2, 7}, {'o', 38, 10, 1}, {'o', 38, 7, 1},
  {'b', 45, 12}, {'b', 49, 12},
  {'=', 55, 10, 3}, {'=', 60, 8, 3}, {'o', 60, 7, 3}, {'=', 65, 10, 3}, {'z', 63, 5},
  {'g', 70, 13, 12}, {'c', 72, 12},
  {'I', 82, 13, 10, 3}, {'g', 92, 13, 10}, {'^', 92, 12, 2}, {'b', 97, 12}, {'B', 98, 9, 2},
  {'s', 100, 12}, {'o', 103, 5, 4},
  {'g', 108, 11, 5}, {'o', 108, 9, 5},
  {'g', 116, 13, 34}, {'x', 120, 10, 2, 3}, {'b', 126, 12}, {'b', 132, 12}, {'z', 129, 8}, {'F', 142, 12},
  {0}
};
static const Op L11[] = {   // Glacier Gap: ice floes high in the air
  {'I', 0, 10, 8, 2}, {'P', 2, 9}, {'B', 4, 6, 1},
  {'I', 11, 10, 4, 1}, {'I', 18, 9, 4, 1}, {'z', 20, 6}, {'I', 25, 10, 3, 1},
  {'x', 30, 10, 6, 2}, {'b', 32, 9}, {'o', 30, 8, 6},
  {'=', 38, 8, 3}, {'=', 43, 6, 3}, {'o', 43, 5, 3}, {'I', 48, 8, 4, 1}, {'z', 46, 3},
  {'x', 54, 10, 6, 2}, {'c', 56, 9}, {'B', 58, 6, 2},
  {'x', 62, 11, 2, 1}, {'s', 63, 10}, {'I', 69, 4, 5, 1}, {'o', 69, 3, 5},
  {'I', 76, 6, 4, 1}, {'b', 77, 5}, {'I', 82, 8, 4, 1}, {'z', 85, 5},
  {'x', 88, 10, 4, 2}, {'I', 94, 9, 3, 1}, {'I', 99, 7, 3, 1}, {'o', 99, 6, 3}, {'I', 104, 9, 3, 1}, {'z', 102, 4},
  {'x', 109, 10, 8, 2}, {'b', 111, 9}, {'b', 114, 9}, {'B', 112, 6, 0},
  {'I', 119, 8, 4, 1}, {'I', 125, 10, 4, 1}, {'z', 127, 7}, {'I', 131, 8, 4, 1}, {'o', 131, 7, 4},
  {'x', 137, 10, 13, 2}, {'b', 141, 9}, {'F', 145, 9},
  {0}
};
static const Op L12[] = {   // The Summit: the last climb
  {'g', 0, 37, 40}, {'P', 3, 36}, {'B', 6, 33, 1},
  {'=', 8, 35, 4}, {'I', 14, 33, 4, 1}, {'o', 14, 32, 4}, {'=', 20, 31, 4}, {'I', 26, 29, 4, 1}, {'b', 27, 28},
  {'=', 32, 27, 5}, {'o', 32, 26, 5}, {'I', 26, 25, 4, 1}, {'z', 23, 22}, {'=', 20, 23, 4}, {'c', 21, 22},
  {'=', 16, 22, 3}, {'x', 11, 22, 4, 1}, {'x', 10, 9, 1, 14}, {'x', 14, 12, 1, 8},
  {'o', 12, 19, 1}, {'o', 12, 16, 1}, {'o', 12, 13, 1},
  {'=', 17, 11, 4}, {'o', 17, 10, 4}, {'I', 23, 9, 4, 1}, {'=', 29, 7, 4}, {'b', 30, 6},
  {'I', 23, 5, 4, 1}, {'z', 27, 2}, {'x', 8, 3, 13, 1}, {'F', 11, 2},
  {0}
};
static const Op L13[] = {   // Vine Valley
  {'g', 0, 11, 22}, {'P', 3, 10}, {'B', 8, 7, 0}, {'o', 12, 8, 4}, {'b', 17, 10},
  {'=', 23, 9, 3}, {'=', 27, 7, 3}, {'o', 27, 6, 3}, {'=', 31, 9, 3},
  {'g', 35, 11, 16}, {'b', 39, 10}, {'b', 45, 10}, {'B', 42, 7, 1}, {'x', 47, 9, 2, 2}, {'o', 47, 7, 2},
  {'g', 51, 12, 3}, {'^', 51, 11, 3},
  {'g', 54, 11, 14}, {'c', 56, 10}, {'z', 61, 6}, {'o', 59, 8, 5},
  {'=', 69, 9, 4}, {'=', 74, 7, 4}, {'o', 74, 6, 4}, {'z', 76, 3}, {'=', 79, 9, 4},
  {'g', 84, 11, 12}, {'s', 86, 10}, {'=', 90, 6, 4}, {'o', 90, 5, 4}, {'b', 93, 10},
  {'g', 96, 9, 6}, {'o', 97, 7, 4},
  {'g', 102, 11, 28}, {'b', 107, 10}, {'b', 112, 10}, {'B', 110, 7, 2}, {'z', 116, 6}, {'o', 118, 8, 5}, {'F', 124, 10},
  {0}
};
static const Op L14[] = {   // Bamboo Bridges
  {'g', 0, 13, 12}, {'P', 2, 12}, {'B', 6, 9, 0}, {'b', 9, 12},
  {'=', 13, 12, 5}, {'=', 19, 11, 5}, {'o', 19, 10, 5}, {'z', 22, 7}, {'=', 25, 10, 5}, {'=', 31, 11, 4},
  {'g', 36, 13, 8}, {'c', 38, 12}, {'b', 41, 12}, {'B', 40, 9, 1},
  {'=', 45, 11, 3}, {'=', 49, 9, 3}, {'=', 53, 7, 3}, {'o', 53, 6, 3}, {'=', 57, 9, 3}, {'=', 61, 11, 3}, {'z', 55, 3},
  {'g', 65, 13, 10}, {'x', 68, 11, 2, 2}, {'b', 72, 12}, {'o', 68, 9, 2},
  {'g', 75, 13, 3}, {'s', 76, 12}, {'=', 80, 6, 6}, {'o', 80, 5, 6}, {'=', 87, 8, 4}, {'b', 88, 7},
  {'g', 92, 13, 12}, {'^', 95, 12, 2}, {'^', 99, 12, 2}, {'o', 95, 9, 2}, {'o', 99, 9, 2},
  {'=', 105, 11, 4}, {'=', 110, 10, 4}, {'z', 112, 6}, {'=', 115, 11, 4},
  {'g', 120, 13, 20}, {'b', 125, 12}, {'B', 127, 9, 0}, {'o', 129, 10, 4}, {'F', 134, 12},
  {0}
};
static const Op L15[] = {   // Thunder Grove: the Jungle King waits at the end
  {'g', 0, 11, 18}, {'P', 2, 10}, {'B', 6, 7, 1}, {'b', 10, 10}, {'b', 14, 10},
  {'g', 18, 9, 5}, {'o', 18, 7, 5},
  {'g', 23, 11, 12}, {'^', 27, 10, 2}, {'z', 30, 5}, {'x', 32, 9, 2, 2},
  {'g', 35, 11, 4},
  {'=', 41, 9, 3}, {'=', 46, 7, 3}, {'o', 46, 6, 3}, {'=', 51, 9, 3},
  {'g', 56, 11, 16}, {'c', 58, 10}, {'b', 62, 10}, {'b', 66, 10}, {'b', 70, 10}, {'=', 61, 8, 8}, {'o', 62, 7, 6},
  {'g', 72, 12, 3}, {'^', 72, 11, 3},
  {'g', 75, 11, 10}, {'s', 77, 10}, {'o', 80, 4, 4}, {'z', 82, 7},
  {'g', 87, 9, 5}, {'o', 87, 7, 5},
  {'g', 94, 11, 26}, {'x', 98, 9, 2, 2}, {'b', 103, 10}, {'b', 108, 10}, {'z', 106, 6}, {'B', 111, 7, 0}, {'F', 114, 10},
  {0}
};
static const Op L16[] = {   // Snowball Hills
  {'g', 0, 11, 16}, {'P', 3, 10}, {'B', 8, 7, 0}, {'b', 12, 10},
  {'g', 16, 10, 5}, {'g', 21, 9, 5}, {'o', 21, 7, 5}, {'g', 26, 10, 5},
  {'I', 31, 11, 12, 3}, {'b', 36, 10}, {'o', 33, 9, 6},
  {'g', 46, 11, 10}, {'x', 49, 9, 2, 2}, {'B', 52, 7, 1}, {'b', 54, 10},
  {'I', 59, 11, 5, 3}, {'I', 67, 11, 5, 3}, {'z', 65, 7}, {'o', 60, 9, 3}, {'o', 68, 9, 3},
  {'g', 75, 11, 12}, {'c', 77, 10}, {'^', 81, 10, 2}, {'b', 85, 10},
  {'g', 87, 9, 5}, {'o', 87, 7, 5}, {'g', 92, 7, 5}, {'o', 92, 5, 5}, {'g', 97, 9, 5},
  {'I', 102, 11, 14, 3}, {'b', 106, 10}, {'b', 112, 10}, {'z', 109, 6},
  {'g', 116, 11, 24}, {'B', 121, 7, 2}, {'b', 126, 10}, {'o', 128, 8, 5}, {'F', 134, 10},
  {0}
};
static const Op L17[] = {   // Crystal Cavern
  {'x', 0, 0, 140, 5},
  {'G', 0, 13, 16}, {'P', 2, 12}, {'v', 8, 5}, {'v', 12, 5}, {'b', 13, 12}, {'B', 5, 9, 0},
  {'I', 16, 13, 12, 3}, {'v', 19, 5}, {'v', 23, 5}, {'o', 18, 11, 8},
  {'G', 31, 11, 5}, {'o', 31, 9, 5},
  {'G', 39, 13, 14}, {'b', 43, 12}, {'b', 49, 12}, {'v', 45, 5}, {'v', 50, 5}, {'B', 41, 9, 1},
  {'=', 55, 11, 4}, {'=', 61, 10, 4}, {'o', 61, 9, 4}, {'v', 62, 5}, {'=', 67, 11, 4},
  {'G', 72, 13, 14}, {'c', 74, 12}, {'^', 78, 12, 2}, {'^', 83, 12, 2}, {'z', 81, 8}, {'v', 80, 5},
  {'I', 86, 13, 14, 3}, {'v', 89, 5}, {'v', 93, 5}, {'v', 97, 5}, {'b', 95, 12},
  {'G', 103, 13, 4}, {'s', 104, 12}, {'o', 107, 7, 4}, {'=', 107, 13, 3},
  {'G', 110, 11, 6}, {'o', 111, 9, 5},
  {'G', 118, 13, 22}, {'b', 123, 12}, {'b', 128, 12}, {'B', 125, 9, 2}, {'v', 127, 5}, {'F', 134, 12},
  {0}
};
static const Op L18[] = {   // Blizzard Pass: the Snow Queen waits at the end
  {'g', 0, 13, 14}, {'P', 2, 12}, {'B', 6, 9, 1}, {'b', 10, 12},
  {'I', 16, 12, 4, 1}, {'I', 22, 11, 4, 1}, {'o', 22, 10, 4}, {'I', 28, 12, 4, 1}, {'z', 25, 7},
  {'g', 34, 13, 12}, {'x', 38, 11, 2, 2}, {'b', 43, 12}, {'o', 38, 9, 2},
  {'I', 46, 13, 10, 3}, {'o', 48, 11, 6},
  {'=', 58, 11, 3}, {'=', 63, 9, 3}, {'o', 63, 8, 3}, {'=', 68, 11, 3}, {'z', 66, 5},
  {'g', 73, 13, 12}, {'c', 75, 12}, {'b', 79, 12}, {'b', 83, 12}, {'B', 81, 9, 0},
  {'g', 85, 11, 5}, {'g', 90, 9, 5}, {'o', 90, 7, 5},
  {'I', 97, 9, 4, 1}, {'I', 103, 10, 4, 1}, {'I', 109, 12, 4, 1}, {'z', 105, 6},
  {'g', 115, 13, 25}, {'^', 119, 12, 2}, {'b', 125, 12}, {'b', 131, 12}, {'B', 128, 9, 2}, {'F', 134, 12},
  {0}
};
static const Op L19[] = {   // Cactus Trail: frogs, flowers, a pretend wall and the helmet
  {'g', 0, 11, 20}, {'P', 3, 10}, {'i', 6, 10, 0}, {'B', 9, 7, 4}, {'f', 15, 10},
  {'g', 20, 9, 6}, {'o', 21, 7, 4}, {'p', 24, 8},
  {'g', 26, 11, 14}, {'y', 31, 10}, {'o', 29, 8, 5}, {'B', 35, 7, 0}, {'i', 34, 10, 1}, {'x', 38, 9, 2, 2},
  {'g', 40, 7, 10}, {'_', 42, 8, 6, 3}, {'h', 40, 8, 2, 3}, {'o', 43, 9, 4}, {'o', 43, 10, 4}, {'o', 43, 5, 5},
  {'g', 50, 11, 12}, {'c', 52, 10}, {'f', 56, 10},
  {'m', 63, 11, 6, 0}, {'o', 66, 8, 4},
  {'g', 75, 11, 30}, {'p', 84, 10}, {'B', 87, 7, 3}, {'i', 89, 10, 2}, {'k', 92, 9, 1, 2}, {'x', 92, 2, 1, 7}, {'o', 94, 9, 4}, {'f', 99, 10},
  {'g', 105, 12, 7}, {'m', 107, 10, 4, 1},
  {'g', 112, 6, 10}, {'o', 114, 4, 5}, {'p', 118, 5},
  {'g', 122, 11, 18}, {'b', 127, 10}, {'F', 134, 10},
  {0}
};
static const Op L20[] = {   // Sandstone Steps: riding the moving platforms
  {'g', 0, 13, 12}, {'P', 2, 12}, {'B', 6, 9, 1}, {'f', 9, 12},
  {'m', 13, 12, 5, 0}, {'o', 15, 9, 4},
  {'x', 23, 12, 5, 2}, {'y', 25, 11},
  {'m', 29, 12, 4, 1}, {'x', 34, 8, 6, 2}, {'o', 35, 6, 4}, {'p', 38, 7},
  {'m', 41, 8, 6, 0}, {'x', 51, 9, 5, 2}, {'c', 52, 8}, {'B', 54, 5, 4},
  {'=', 57, 10, 3}, {'=', 61, 11, 3},
  {'g', 65, 13, 14}, {'y', 69, 12}, {'y', 74, 12}, {'o', 68, 10, 8},
  {'m', 80, 13, 5, 0}, {'m', 89, 12, 4, 1}, {'x', 94, 8, 5, 2}, {'a', 96, 4},
  {'m', 100, 8, 5, 0}, {'o', 102, 5, 4},
  {'g', 108, 13, 32}, {'f', 113, 12}, {'B', 116, 9, 0}, {'B', 121, 9, 3}, {'k', 124, 11, 2, 2}, {'x', 124, 3, 2, 8}, {'o', 127, 11, 5}, {'F', 134, 12},
  {0}
};
static const Op L21[] = {   // Pyramid Path: King Croak waits at the end
  {'g', 0, 11, 30}, {'P', 2, 10}, {'B', 6, 7, 4}, {'y', 11, 10},
  {'x', 16, 9, 12, 2}, {'x', 18, 7, 8, 2}, {'x', 20, 5, 4, 2}, {'o', 20, 3, 4},
  {'_', 20, 9, 4, 2}, {'H', 16, 9, 4, 2}, {'o', 20, 10, 4}, {'o', 20, 9, 4},
  {'g', 30, 11, 10}, {'f', 33, 10}, {'f', 37, 10},
  {'m', 41, 11, 5, 0},
  {'g', 50, 11, 14}, {'c', 52, 10}, {'x', 54, 3, 10, 1}, {'a', 56, 4}, {'a', 60, 4}, {'o', 55, 8, 6},
  {'=', 66, 9, 3}, {'=', 71, 7, 3}, {'o', 71, 6, 3}, {'=', 76, 9, 3},
  {'g', 81, 11, 12}, {'y', 85, 10}, {'p', 90, 10},
  {'g', 93, 12, 6}, {'m', 94, 10, 3, 1}, {'g', 99, 7, 6}, {'o', 100, 5, 4},
  {'g', 105, 11, 15}, {'B', 109, 7, 0}, {'F', 114, 10},
  {0}
};
static const Op L22[] = {   // Cloud Hop
  {'x', 0, 10, 8, 2}, {'P', 2, 9}, {'B', 4, 6, 1},
  {'m', 9, 10, 5, 0}, {'x', 18, 10, 5, 2}, {'f', 20, 9},
  {'m', 24, 10, 4, 1}, {'x', 29, 6, 6, 2}, {'o', 30, 4, 4}, {'p', 33, 5},
  {'m', 36, 6, 6, 0}, {'x', 46, 7, 5, 2}, {'c', 47, 6}, {'a', 50, 2},
  {'=', 52, 9, 3}, {'=', 56, 11, 3}, {'x', 60, 11, 6, 2}, {'y', 62, 10}, {'B', 63, 7, 4},
  {'m', 67, 11, 5, 0}, {'m', 76, 11, 5, 1}, {'x', 81, 6, 5, 2}, {'o', 82, 4, 3},
  {'m', 87, 6, 5, 0}, {'a', 92, 1}, {'x', 96, 8, 6, 2}, {'f', 98, 7},
  {'=', 103, 10, 3}, {'m', 107, 10, 5, 0},
  {'x', 116, 10, 8, 2}, {'c', 117, 9}, {'p', 121, 9},
  {'m', 125, 10, 5, 0}, {'x', 134, 10, 16, 2}, {'b', 138, 9}, {'F', 144, 9},
  {0}
};
static const Op L23[] = {   // Bat Belfry: bats, a hidden room under the floor and cracked walls
  {'x', 0, 0, 140, 3},
  {'G', 0, 13, 18}, {'P', 2, 12}, {'i', 5, 12, 0}, {'a', 10, 3}, {'a', 14, 3}, {'B', 7, 9, 4},
  {'G', 18, 11, 5}, {'o', 18, 9, 5},
  {'G', 23, 13, 26}, {'B', 25, 9, 3}, {'y', 28, 12}, {'k', 33, 11, 1, 2}, {'x', 33, 3, 1, 8},
  {'H', 40, 13, 6, 1}, {'_', 40, 14, 6, 1}, {'o', 40, 14, 6}, {'i', 37, 12, 1}, {'p', 47, 12},
  {'m', 50, 13, 5, 0},
  {'G', 59, 13, 14}, {'c', 61, 12}, {'a', 64, 3}, {'a', 68, 3}, {'y', 67, 12}, {'=', 63, 9, 4}, {'o', 63, 8, 4},
  {'G', 73, 14, 6}, {'m', 74, 12, 4, 1}, {'G', 79, 8, 8}, {'o', 80, 6, 5}, {'p', 84, 7},
  {'G', 87, 13, 12}, {'f', 90, 12}, {'f', 95, 12}, {'B', 92, 9, 0},
  {'=', 100, 11, 3}, {'m', 104, 11, 5, 0},
  {'G', 113, 13, 27}, {'a', 117, 3}, {'y', 120, 12}, {'B', 123, 9, 3}, {'k', 127, 11, 2, 2}, {'x', 127, 3, 2, 8}, {'o', 130, 11, 3}, {'F', 134, 12},
  {0}
};
static const Op L24[] = {   // Storm Tower: the Bat Baron waits at the end
  {'x', 0, 10, 10, 2}, {'P', 2, 9}, {'B', 5, 6, 4}, {'y', 7, 9},
  {'m', 11, 10, 5, 0}, {'x', 20, 10, 6, 2}, {'a', 22, 4}, {'f', 23, 9},
  {'=', 27, 8, 3}, {'=', 31, 6, 3}, {'o', 31, 5, 3}, {'=', 35, 8, 3},
  {'x', 39, 10, 8, 2}, {'c', 41, 9}, {'p', 45, 9},
  {'m', 48, 10, 4, 1}, {'x', 53, 6, 6, 2}, {'o', 54, 4, 4}, {'a', 57, 1},
  {'m', 60, 6, 6, 0}, {'x', 70, 8, 6, 2}, {'y', 72, 7}, {'B', 74, 4, 1},
  {'m', 77, 8, 5, 0}, {'m', 86, 9, 4, 1}, {'x', 91, 5, 5, 2}, {'o', 92, 3, 3},
  {'=', 97, 7, 3}, {'=', 101, 9, 3},
  {'x', 105, 10, 15, 2}, {'f', 109, 9}, {'B', 112, 6, 2}, {'F', 115, 9},
  {0}
};
static const Level LEVELS[] = {
  { "1  JUNGLE FLOOR", 140, 14, 0xFF7EC8F0, 0xFFD4F1F9, 0xFF6FB07E, 0xFF3F8452, 0, L1,
    { "A: JUMP. HOLD IT TO JUMP HIGHER", "X OR Y: HEADBUTT. JUMP UNDER BOXES TO OPEN THEM", "SPRINGS BOUNCE YOU UP HIGH" } },
  { "2  CANOPY CLIMB", 110, 26, 0xFFF7A35C, 0xFFFDE2B0, 0xFFC97B5A, 0xFF7A4B3A, 0, L2,
    { "JUMP AT A WALL, THEN JUMP AGAIN TO WALL-JUMP", "CLIMB THE TOWER TO REACH THE FLAG" } },
  { "3  MISTY FALLS", 40, 36, 0xFF5FB4C9, 0xFFC8EEF0, 0xFF5E9C8C, 0xFF2F6B5E, 4, L4,
    { "THE FLAG IS AT THE VERY TOP. KEEP CLIMBING!" } },
  { "4  OLD TEMPLE", 150, 16, 0xFF3B2F6B, 0xFF9C6FA8, 0xFF5A4A86, 0xFF3A2F5C, 1, L3, { 0 } },
  { "5  BEETLE CAVERN", 130, 16, 0xFF1E1A24, 0xFF4A3B3A, 0xFF3A302E, 0xFF2A2220, 2, L5, { 0 } },
  { "6  SKY BRIDGE", 150, 16, 0xFF4A90E2, 0xFFBFE3FF, 0xFFFFFFFF, 0xFFE3F1FF, 3, L6, { 0 } },
  // world 2: Frosty Forest
  { "7  SNOWY TRAIL", 140, 14, 0xFF8FC4EC, 0xFFEAF6FF, 0xFFD5E6F2, 0xFF7FA6BE, 5, L7,
    { "ICE IS SLIPPERY. YOU KEEP SLIDING WHEN YOU STOP!" } },
  { "8  FROZEN LAKE", 150, 14, 0xFF8FC4EC, 0xFFEAF6FF, 0xFFD5E6F2, 0xFF7FA6BE, 5, L8, { 0 } },
  { "9  ICICLE CAVE", 130, 16, 0xFF0E2238, 0xFF1F4A6E, 0xFF18395A, 0xFF12304C, 6, L9,
    { "LOOK UP! ICICLES FALL WHEN YOU WALK UNDER THEM" } },
  // world 3: Ice Mountain
  { "10 NORTHERN LIGHTS", 150, 16, 0xFF0B1030, 0xFF2B3F7A, 0xFF7F93C4, 0xFF4A5C94, 7, L10, { 0 } },
  { "11 GLACIER GAP", 150, 16, 0xFF0B1030, 0xFF2B3F7A, 0xFF7F93C4, 0xFF4A5C94, 7, L11, { 0 } },
  { "12 THE SUMMIT", 40, 40, 0xFF0B1030, 0xFF2B3F7A, 0xFF7F93C4, 0xFF4A5C94, 7, L12, { 0 } },
  // world 4: Wild Jungle
  { "13 VINE VALLEY", 130, 14, 0xFFF08A4A, 0xFFFFE0A8, 0xFF7FA85E, 0xFF3F7A46, 0, L13, { 0 } },
  { "14 BAMBOO BRIDGES", 140, 16, 0xFF4FC2B8, 0xFFD8F7E4, 0xFF63B08A, 0xFF2F7A5A, 4, L14, { 0 } },
  { "15 THUNDER GROVE", 120, 14, 0xFF4A4A78, 0xFFB8A8C8, 0xFF4F7A6A, 0xFF2A5244, 0, L15, { 0 } },
  // world 5: Snowflake Peaks
  { "16 SNOWBALL HILLS", 140, 14, 0xFFF0A8C0, 0xFFFFF0E0, 0xFFE8DCEC, 0xFF9A9CC4, 5, L16, { 0 } },
  { "17 CRYSTAL CAVERN", 140, 16, 0xFF141038, 0xFF3A2F7A, 0xFF2A2460, 0xFF1C1848, 6, L17, { 0 } },
  { "18 BLIZZARD PASS", 140, 16, 0xFF10183C, 0xFF3F5490, 0xFF8FA3D0, 0xFF55689C, 7, L18, { 0 } },
  // world 6: Sunny Dunes
  { "19 CACTUS TRAIL", 140, 14, 0xFF58B8F0, 0xFFFFF0C8, 0xFFE8C888, 0xFF5FA05A, 8, L19,
    { "FROGS HOP AT YOU AND FLOWERS SPIT SEEDS. THE STAR THROWS BACK!", "SOME WALLS ARE NOT REAL. TRY WALKING INTO THEM", "THE HELMET SMASHES CRACKED BLOCKS: HEADBUTT THEM" } },
  { "20 SANDSTONE STEPS", 140, 16, 0xFFF0A050, 0xFFFFE8B0, 0xFFE0B070, 0xFF5A9456, 8, L20, { 0 } },
  { "21 PYRAMID PATH", 120, 14, 0xFF58B8F0, 0xFFFFF0C8, 0xFFE8C888, 0xFF5FA05A, 8, L21, { 0 } },
  // world 7: Cloud Castle
  { "22 CLOUD HOP", 150, 16, 0xFF5AA0F0, 0xFFD0ECFF, 0xFFFFFFFF, 0xFFE3F1FF, 3, L22, { 0 } },
  { "23 BAT BELFRY", 140, 16, 0xFF2A2050, 0xFF7A5A98, 0xFF4A3E78, 0xFF30284E, 1, L23,
    { "BATS SLEEP UNTIL YOU COME CLOSE", "THAT FLOOR LOOKS A LITTLE ODD..." } },
  { "24 STORM TOWER", 120, 16, 0xFF3A4670, 0xFF9AA8C8, 0xFFC8D0E4, 0xFFA8B4D0, 3, L24, { 0 } },
};
#define NLEVELS (int)(sizeof LEVELS / sizeof LEVELS[0])

// ============================================================ state
enum { ST_TITLE, ST_PLAY, ST_PAUSE, ST_CLEAR, ST_OVER, ST_WIN, ST_MAP };
enum { E_BEETLE, E_BEE, E_POW, E_ICE, E_BOSS, E_PLAT, E_FROG, E_SPINY, E_PLANT, E_BAT, E_SHOT };
// E_PLAT: a moving platform (k: 0 side to side, 1 up and down; vx: how far).  E_SHOT: k 0 a seed spat at Dino, 1 Dino's bouncing star
#define IS_FOE(t) ((t) == E_BEETLE || (t) == E_BEE || ((t) >= E_FROG && (t) <= E_BAT))   // E_ICE: an icicle (k: 0 hanging, 1 shaking, 2 falling)   // E_POW: a power-up (e->vx holds which: 1 double jump, 2 sword)
enum { SFX_JUMP, SFX_BANANA, SFX_STOMP, SFX_HURT, SFX_SPRING, SFX_CHECK, SFX_CLEAR, SFX_LIFE, SFX_SELECT, SFX_WALL, SFX_DASH, SFX_SWORD, SFX_JUMP2 };

typedef struct { int type, alive, dying, t, k; float x, y, vx, vy, by, fz, x0; } Ent;
typedef struct { float x, y, vx, vy; int life; uint32_t c; } Part;

static char G[MAXH][MAXW];
static unsigned char FAKE[MAXH][MAXW];   // a pretend wall: looks like ground, but you can walk through it
static int tile_raw, ride_i = -1;        // tile_raw: show pretend walls as ground (for painting). ride_i: the platform Dino stands on
static Bmp CRIT_C[4][2][8], HD_PLAT, HD_CRACK, HD_HELMI, HD_STARI, HD_HELM;
static char BOXK[MAXH][MAXW];   // what each ? box holds: 0 bananas, 1 double jump, 2 sword
static int LW, LH, lvl;
static const Level* L;
#define MAXENT 170
static Bmp FLAKE[3];
int g_lowq;   // set by the platform when frames are taking too long: draw Dino the quick way
static Bmp BEETLE_C[2][2][20], BEE_C[2][8], BOSS_C[2][20], QBEE_C[2][8];
static Ent ents[MAXENT]; static int nents;
static int boss_i, boss_hp, boss_max, boss_inv, boss_awake, boss_dead, arena_x0, arena_x1;
int g_boss_music;
static Part parts[160];
static int sign_x[4], sign_y[4], nsigns;
static int goal_x, goal_y;

static int state, frame, lives, bananas, total_bananas, state_t;
static float px, py, pvx, pvy, camx, camy;
static int spring_up, power, inv_t, atk_t, atk_cd, air_jumps, prev_atk;
static int face, on_ground, coyote, jbuf, wall_lock, wall_dir, dead_t, run_t, land_t;
static int start_x, start_y, check_x, check_y, prev_jump, prev_start, prev_down, prev_up, prev_left, prev_right;
static int ncheck;
#define PW 10
#define PH 13

static int lvl_frames; int g_no_hud;
// intro_t: counts up while a level is being introduced (Dino and the counters on black, then they
// fly to their places). over_sel: which button is picked on the game over screen.
static int intro_t, over_sel, dino_sx, dino_sy;
#define INTRO_HOLD 85
#define INTRO_FLY 60
// world map: node 0 is Dino's house, node n is level n
static int cleared[32], map_node, map_from, map_t, world;
// Each world is its own map. A stop is a level (kind >= 0, the level's index), Dino's house (-1),
// the way back to the world before (-2) or the way on to the next world (-3).
typedef struct { const char* name; int n; short xy[8][2]; signed char kind[8]; } World;
static const World WORLDS[7] = {
  { "JUNGLE ISLAND", 8, { {38, 146}, {84, 134}, {130, 108}, {96, 62}, {166, 50}, {226, 92}, {282, 44}, {306, 112} }, { -1, 0, 1, 2, 3, 4, 5, -3 } },
  { "FROSTY FOREST", 5, { {22, 150}, {80, 128}, {140, 100}, {204, 74}, {276, 46} }, { -2, 6, 7, 8, -3 } },
  { "ICE MOUNTAIN", 5, { {26, 150}, {92, 128}, {164, 98}, {238, 54}, {298, 104} }, { -2, 9, 10, 11, -3 } },
  { "WILD JUNGLE", 5, { {24, 150}, {84, 120}, {150, 138}, {214, 98}, {290, 62} }, { -2, 12, 13, 14, -3 } },
  { "SNOWFLAKE PEAKS", 5, { {24, 62}, {92, 98}, {168, 72}, {252, 120}, {300, 152} }, { -2, 15, 16, 17, -3 } },
  { "SUNNY DUNES", 5, { {24, 150}, {86, 124}, {150, 140}, {216, 100}, {292, 64} }, { -2, 18, 19, 20, -3 } },
  { "CLOUD CASTLE", 4, { {24, 150}, {96, 120}, {170, 90}, {250, 56} }, { -2, 21, 22, 23 } },
};
#define NWORLDS 7
#define SNOW_WORLD (world == 1 || world == 2 || world == 4)
#define MAP_XY (WORLDS[world].xy)
#ifndef SAVE_PATH
#define SAVE_PATH "amazingdino-save.txt"
#endif
#ifdef __WIIU__
#include <sys/stat.h>
#endif
static void save_progress(void) {
#ifdef __WIIU__
  mkdir("fs:/vol/external01/wiiu/apps/AmazingDino", 0777);
#endif
  FILE* f = fopen(SAVE_PATH, "w");
  if (!f) return;
  for (int i = 1; i <= NLEVELS; i++) fputc(cleared[i] ? '1' : '0', f);
  fprintf(f, "\n%d\n", total_bananas);
  fclose(f);
}
static void load_progress(void) {
  FILE* f = fopen(SAVE_PATH, "r");
#ifdef __WIIU__
  if (!f) f = fopen("fs:/vol/external01/wiiu/apps/CanopyDino/save.txt", "r");   // progress from before the game was renamed
#endif
  if (!f) return;
  char b[64] = { 0 };
  if (fgets(b, sizeof b, f)) for (int i = 1; i <= NLEVELS && b[i - 1]; i++) cleared[i] = b[i - 1] == '1';
  if (fscanf(f, "%d", &total_bananas) != 1) total_bananas = 0;
  fclose(f);
}
static int node_open(int n) {
  const World* w = &WORLDS[world];
  for (int k = 0; k < n; k++) if (w->kind[k] >= 0 && !cleared[w->kind[k] + 1]) return 0;   // every level before it is cleared
  return n < w->n;
}


static char tile(int tx, int ty) {
  if (tx < 0 || tx >= LW) return 'X';
  if (ty < 0 || ty >= LH) return ' ';
  if (FAKE[ty][tx] && !tile_raw) return ' ';
  return G[ty][tx];
}
static int solid_c(char c) { return c == '#' || c == 'X' || c == '?' || c == 'u' || c == 'I' || c == 'k'; }
#include "hd.h"
#include "anim.h"
static int box_solid(float x, float y, float w, float h) {
  int x0 = (int)floorf(x / T), x1 = (int)floorf((x + w - 0.01f) / T);
  int y0 = (int)floorf(y / T), y1 = (int)floorf((y + h - 0.01f) / T);
  for (int ty = y0; ty <= y1; ty++) for (int tx = x0; tx <= x1; tx++) if (solid_c(tile(tx, ty))) return 1;
  return 0;
}

static void spawn_parts(float x, float y, int n, uint32_t c, float spd, float up) {
  for (int k = 0; k < n; k++) {
    for (int i = 0; i < 160; i++) if (parts[i].life <= 0) {
      uint32_t h = hash2(frame * 7 + k, i);
      float a = (h % 628) / 100.0f, s = spd * (0.4f + (h >> 10) % 100 / 160.0f);
      parts[i] = (Part){ x, y, cosf(a) * s, sinf(a) * s - up, 18 + (int)(h >> 20) % 14, c };
      break;
    }
  }
}

// ---- every side-scrolling level gets a long extra stretch built from set pieces (always the same
// for the same level), and the last level of each world ends in a boss arena.
static uint32_t gen_seed;
static int grnd(int n) { gen_seed = gen_seed * 1664525u + 1013904223u; return (int)((gen_seed >> 10) % (uint32_t)n); }
static Ent* add_ent(int type, int tx, int ty) {
  if (nents >= MAXENT) return NULL;
  Ent* e = &ents[nents++];
  memset(e, 0, sizeof *e);
  e->type = type; e->alive = 1;
  e->x = e->x0 = tx * T + 2; e->y = (type == E_BEETLE || type == E_FROG || type == E_SPINY) ? ty * T + 6 : type == E_PLANT ? ty * T + 2 : ty * T; e->by = e->y;
  e->vx = type == E_SPINY ? -0.4f : -0.5f; e->t = tx * 13;
  return e;
}
static const char* const BOSS_NAME[7] = { "KING BEETLE", "FROST KING", "QUEEN BEE", "JUNGLE KING", "SNOW QUEEN", "KING CROAK", "BAT BARON" };
#define BOSS_BEE(k) ((k) == 2 || (k) == 4 || (k) == 6)
#define BOSS_COL(k) ((k) == 0 ? 2 : (k) == 1 ? 1 : 0)   // beetle3d colour: red, icy, green
static int boss_kind(int n) { return n == 5 ? 0 : n == 8 ? 1 : n == 10 ? 2 : n == 14 ? 3 : n == 17 ? 4 : n == 20 ? 5 : n == 23 ? 6 : -1; }
static void extend_level(int n) {
  boss_i = -1; boss_awake = boss_dead = 0; g_boss_music = 0;
  if (LH > 20) return;   // the tall climbing levels stay as they are
  int th = L->theme, stone = th == 1 || th == 2 || th == 6, snow = th >= 5 && th <= 7;
  char gc = stone ? 'X' : '#';
  int x = LW, gy = goal_y + 1, bk = boss_kind(n);
  if (gy < 7) gy = 7;
  if (gy > LH - 2) gy = LH - 2;
#define COL(cx, top) do { for (int yy = (top); yy < LH; yy++) G[yy][cx] = gc; } while (0)
  for (int c = goal_x - 1; c < LW; c++) if (c >= 0 && !solid_c(G[goal_y + 1][c])) COL(c, goal_y + 1);
  G[goal_y][goal_x] = 'c';   // the old flag spot becomes a checkpoint
  gy = goal_y + 1;
  gen_seed = 1234567u + n * 7919u;
  int end = LW + (bk >= 0 ? 90 : 130), cnt = 0, last = -1;
  while (x < end) {
    int k = grnd(n >= 18 ? 14 : n >= 12 ? 11 : (snow ? 9 : 8));   // the two newest worlds have extra set pieces
    if (!snow && k == 8) k = 10;
    if (k == last) k = (k + 1) % 8;
    last = k; cnt++;
    if (cnt % 5 == 0) { for (int i = 0; i < 6; i++) COL(x + i, gy); G[gy - 1][x + 2] = 'c'; x += 6; continue; }
    switch (k) {
      case 0: {   // a run with coins and beetles
        int w = 10 + grnd(5);
        for (int i = 0; i < w; i++) COL(x + i, gy);
        for (int i = 3; i < 8; i++) G[gy - 3 - (i > 3 && i < 7)][x + i] = 'o';
        add_ent(E_BEETLE, x + w - 3, gy - 1);
        if (grnd(2)) add_ent(E_BEETLE, x + 5, gy - 1);
        x += w; break;
      }
      case 1: {   // a gap to jump
        int w = 3 + grnd(2);
        for (int i = 0; i < 3; i++) COL(x + i, gy);
        x += 3;
        for (int i = 0; i < w; i++) G[gy - 3][x + i] = 'o';
        x += w;
        for (int i = 0; i < 3; i++) COL(x + i, gy);
        x += 3; break;
      }
      case 2: {   // a step up or down
        int ny = gy + (grnd(2) ? 1 : -1) * (1 + grnd(2));
        if (ny < 7) ny = gy + 1;
        if (ny > LH - 2) ny = gy - 1;
        for (int i = 0; i < 4; i++) COL(x + i, gy);
        x += 4; gy = ny;
        for (int i = 0; i < 5; i++) COL(x + i, gy);
        G[gy - 2][x + 2] = 'o'; G[gy - 2][x + 3] = 'o';
        x += 5; break;
      }
      case 3: {   // a log bridge over a pit, with a bee
        for (int i = 0; i < 2; i++) COL(x + i, gy);
        x += 2;
        int w = 8 + grnd(4);
        for (int i = 0; i < w; i++) { G[gy][x + i] = '='; if (i % 2) G[gy - 2][x + i] = 'o'; }
        add_ent(E_BEE, x + w / 2, gy - 4);
        x += w;
        for (int i = 0; i < 2; i++) COL(x + i, gy);
        x += 2; break;
      }
      case 4: {   // spikes to hop over
        for (int i = 0; i < 4; i++) COL(x + i, gy);
        x += 4;
        for (int i = 0; i < 2; i++) { COL(x + i, gy); G[gy - 1][x + i] = '^'; G[gy - 4][x + i] = 'o'; }
        x += 2;
        for (int i = 0; i < 4; i++) COL(x + i, gy);
        x += 4; break;
      }
      case 5: {   // a row of ? boxes
        for (int i = 0; i < 9; i++) COL(x + i, gy);
        for (int i = 3; i < 6; i++) { G[gy - 4][x + i] = '?'; BOXK[gy - 4][x + i] = 0; }
        if (grnd(3) == 0) BOXK[gy - 4][x + 4] = (char)(1 + grnd(2));
        add_ent(E_BEETLE, x + 7, gy - 1);
        x += 9; break;
      }
      case 6: {   // stone steps up and over
        for (int i = 0; i < 3; i++) COL(x + i, gy);
        x += 3;
        for (int i = 0; i < 6; i++) {
          int hgt = i < 2 ? 1 : (i < 4 ? 2 : 1);
          COL(x + i, gy);
          for (int j = 1; j <= hgt; j++) G[gy - j][x + i] = 'X';
          if (i == 2 || i == 3) G[gy - 4][x + i] = 'o';
        }
        x += 6;
        for (int i = 0; i < 3; i++) COL(x + i, gy);
        x += 3; break;
      }
      case 7: {   // a spring up to a high bridge of coins
        for (int i = 0; i < 15; i++) COL(x + i, gy);
        G[gy - 1][x + 2] = 's';
        for (int i = 4; i < 9; i++) { G[gy - 6][x + i] = '='; G[gy - 7][x + i] = 'o'; }
        add_ent(E_BEETLE, x + 10, gy - 1);
        x += 15; break;
      }
      case 8: {   // a slippery stretch of ice
        int w = 9 + grnd(4);
        for (int i = 0; i < w; i++) { COL(x + i, gy); G[gy][x + i] = 'I'; if (i > 2 && i < 7) G[gy - 2][x + i] = 'o'; }
        add_ent(E_BEETLE, x + w - 2, gy - 1);
        x += w; break;
      }
      case 9: {   // zigzag planks over a pit, with a bee
        for (int i = 0; i < 2; i++) COL(x + i, gy);
        x += 2;
        for (int j = 0; j < 4; j++) {
          int yy = gy - 1 - (j % 2) * 2;
          for (int i = 0; i < 4; i++) G[yy][x + j * 4 + i] = '=';
          G[yy - 2][x + j * 4 + 1] = 'o';
        }
        add_ent(E_BEE, x + 8, gy - 7);
        x += 16;
        for (int i = 0; i < 2; i++) COL(x + i, gy);
        x += 2; break;
      }
      case 11: {   // a moving platform across a wide pit
        for (int i = 0; i < 2; i++) COL(x + i, gy);
        x += 2;
        Ent* e = add_ent(E_PLAT, x, gy);
        if (e) { e->x = e->x0 = x * T; e->y = e->by = gy * T; e->vx = 6 * T; e->k = 0; e->t = 0; }
        for (int i = 2; i < 7; i++) G[gy - 3][x + i] = 'o';
        x += 9;
        for (int i = 0; i < 2; i++) COL(x + i, gy);
        x += 2; break;
      }
      case 12: {   // a frog and a seed-spitting flower
        for (int i = 0; i < 13; i++) COL(x + i, gy);
        add_ent(E_FROG, x + 4, gy - 1); add_ent(E_PLANT, x + 10, gy - 1);
        for (int i = 5; i < 9; i++) G[gy - 3][x + i] = 'o';
        x += 13; break;
      }
      case 13: {   // a star box, then spiky balls and a sleeping bat
        for (int i = 0; i < 15; i++) COL(x + i, gy);
        G[gy - 4][x + 2] = '?'; BOXK[gy - 4][x + 2] = 4;
        add_ent(E_SPINY, x + 8, gy - 1); add_ent(E_SPINY, x + 12, gy - 1); add_ent(E_BAT, x + 10, gy - 7);
        x += 15; break;
      }
      case 10: {
        for (int i = 0; i < 12; i++) COL(x + i, gy);
        if (snow) {   // icicles hanging under an ice ledge
          for (int i = 3; i < 9; i++) G[gy - 5][x + i] = 'I';
          for (int i = 4; i < 9; i += 2) { Ent* e = add_ent(E_ICE, x + i, gy - 4); if (e) { e->y = e->by = (gy - 4) * T; e->vx = 0; e->t = 0; } }
          for (int i = 5; i < 8; i++) G[gy - 2][x + i] = 'o';
        } else {      // a beetle parade under a plank roof
          for (int i = 2; i < 10; i++) { G[gy - 3][x + i] = '='; G[gy - 4][x + i] = 'o'; }
          add_ent(E_BEETLE, x + 4, gy - 1); add_ent(E_BEETLE, x + 7, gy - 1); add_ent(E_BEETLE, x + 10, gy - 1);
        }
        x += 12; break;
      }
    }
  }
  if (bk >= 0) {
    // the boss arena: a checkpoint and a sword box, then a wide flat floor
    for (int i = 0; i < 38; i++) COL(x + i, gy);
    G[gy - 1][x + 2] = 'c'; G[gy - 4][x + 5] = '?'; BOXK[gy - 4][x + 5] = 2;
    arena_x0 = (x + 9) * T; arena_x1 = (x + 34) * T;
    Ent* e = add_ent(E_BOSS, x + 26, gy - 1);
    if (e) {
      boss_i = (int)(e - ents); e->k = bk;
      e->x = e->x0 = (x + 26) * T; e->y = e->by = BOSS_BEE(bk) ? gy * T - 84 : gy * T - 26;
      boss_max = boss_hp = bk >= 5 ? 6 : bk >= 3 ? bk + 2 : 3 + bk;
    }
    x += 38;
  }
  for (int i = 0; i < 9; i++) COL(x + i, gy);
  goal_x = x + 4; goal_y = gy - 1;
  LW = x + 9;
#undef COL
}

static void load_level(int n) {
  lvl = n; L = &LEVELS[n]; LW = L->w; LH = L->h;
  memset(G, ' ', sizeof G); memset(BOXK, 0, sizeof BOXK); memset(FAKE, 0, sizeof FAKE); ride_i = -1;
  nents = 0; nsigns = 0; ncheck = 0;
  memset(parts, 0, sizeof parts);
  for (const Op* o = L->ops; o->op; o++) {
    int w = o->w ? o->w : 1, h = o->h ? o->h : 1;
    switch (o->op) {
      case 'g': case 'G':
        for (int x = o->x; x < o->x + w; x++) for (int y = o->y; y < LH; y++) G[y][x] = o->op == 'g' ? '#' : 'X';
        break;
      case 'x': for (int y = o->y; y < o->y + h; y++) for (int x = o->x; x < o->x + w; x++) G[y][x] = 'X'; break;
      case '=': case '^': case 'o':
        for (int x = o->x; x < o->x + w; x++) G[o->y][x] = o->op;
        break;
      case 's': G[o->y][o->x] = 's'; break;
      case 'B': G[o->y][o->x] = '?'; BOXK[o->y][o->x] = (char)o->w; break;
      case 'I': for (int y = o->y; y < o->y + h; y++) for (int x = o->x; x < o->x + w; x++) G[y][x] = 'I'; break;
      case 'v':
        if (nents < MAXENT) {
          Ent* e = &ents[nents++];
          memset(e, 0, sizeof *e);
          e->type = E_ICE; e->alive = 1; e->x = o->x * T + 2; e->y = e->by = o->y * T;
        }
        break;
      case 'c': G[o->y][o->x] = 'c'; break;
      case '_': for (int y = o->y; y < o->y + h; y++) for (int x = o->x; x < o->x + w; x++) G[y][x] = ' '; break;
      case 'k': for (int y = o->y; y < o->y + h; y++) for (int x = o->x; x < o->x + w; x++) G[y][x] = 'k'; break;   // cracked blocks
      case 'h': case 'H':   // a pretend wall hiding a secret
        for (int y = o->y; y < o->y + h; y++) for (int x = o->x; x < o->x + w; x++) { G[y][x] = o->op == 'H' ? 'X' : '#'; FAKE[y][x] = 1; }
        break;
      case 'm': { Ent* e = add_ent(E_PLAT, o->x, o->y); if (e) { e->x = e->x0 = o->x * T; e->y = e->by = o->y * T; e->vx = o->w * T; e->k = o->h; e->t = 0; } break; }
      case 'f': add_ent(E_FROG, o->x, o->y); break;
      case 'y': add_ent(E_SPINY, o->x, o->y); break;
      case 'p': add_ent(E_PLANT, o->x, o->y); break;
      case 'a': add_ent(E_BAT, o->x, o->y); break;
      case 'F': goal_x = o->x; goal_y = o->y; break;
      case 'P': start_x = o->x * T + 3; start_y = o->y * T + T - PH; break;
      case 'i': if (nsigns < 4) { sign_x[o->w] = o->x; sign_y[o->w] = o->y; nsigns++; } break;
      case 'b': case 'z':
        if (nents < MAXENT) {
          Ent* e = &ents[nents++];
          memset(e, 0, sizeof *e);
          e->type = o->op == 'b' ? E_BEETLE : E_BEE; e->alive = 1;
          e->x = o->x * T + 2; e->y = o->op == 'b' ? o->y * T + 6 : o->y * T; e->by = e->y;
          e->vx = -0.5f; e->t = o->x * 13;
          break;
        }
    }
  }
  for (int i = 0; i < nsigns; i++) if (!L->signs[i]) nsigns = i;
  extend_level(n);
  check_x = start_x; check_y = start_y;
  lvl_frames = 0;
  tile_raw = 1; hd_render_level(); tile_raw = 0;
  for (int d = 0; d < 2; d++) for (int i = 0; i < 20; i++) if (BOSS_C[d][i].px) { free(BOSS_C[d][i].px); BOSS_C[d][i].px = NULL; BOSS_C[d][i].w = 0; }
}

static void respawn(void) {
  px = check_x; py = check_y; pvx = pvy = 0; face = 1; dead_t = 0; wall_lock = 0; jbuf = 0; atk_t = 0; inv_t = 0;
  camx = px - SCR_W / 2; camy = py - SCR_H / 2;
  // beetles and bees come back when you respawn
  for (int i = 0; i < nents; i++) {
    Ent* e = &ents[i];
    if (e->type == E_POW) continue;
    if (e->type == E_BOSS) { if (!boss_dead) { e->alive = 1; e->dying = 0; e->x = e->x0; e->y = e->by; e->vy = 0; e->vx = -0.5f; boss_hp = boss_max; boss_inv = 0; boss_awake = 0; g_boss_music = 0; } continue; }
    if (e->type == E_ICE) { e->alive = 1; e->k = 0; e->y = e->by; e->vy = 0; continue; }
    if (e->type == E_SHOT) { e->alive = 0; continue; }
    if (e->type == E_PLAT) continue;
    if (e->type >= E_FROG) { e->alive = 1; e->dying = 0; e->x = e->x0; e->y = e->by; e->vy = 0; e->k = 0; continue; }
    if (!e->alive || e->dying) { e->alive = 1; e->dying = 0; e->y = e->by; }
  }
  ride_i = -1;
  // boxes that held a power-up fill up again, so a path that needs one can always be done
  for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) if (G[y][x] == 'u' && BOXK[y][x]) G[y][x] = '?';
}

static void start_level(int n) {
  load_level(n);
  // reset enemy spawn positions to the level's original ones
  respawn();
  state = ST_PLAY; state_t = 0; intro_t = 1;
}

void game_init(void) {
  state = ST_TITLE; state_t = 0; frame = 0;
  load_progress();
  hd_init(); m3d_build();
  for (int k = 0; k < 3; k++) {
    FLAKE[k] = bmp_new(8, 8);
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) FLAKE[k].px[y * 8 + x] = ((uint32_t)(cov(sd_circle(x + 0.5f, y + 0.5f, 4, 4, 1.4f + k * 0.7f)) * 230) << 24) | 0xFFFFFF;
  }
  load_level(0); respawn();
}

static void hurt_ex(int pit) {
  if (dead_t) return;
  if (!pit && inv_t) return;
  if (!pit && power) {
    // losing the power-up instead of a life
#ifdef TRACE
    printf("lost power at x=%.0f\n", px / 16);
#endif
    power = 0; inv_t = 100; pvy = -3.5f; pvx = -face * 2.0f; wall_lock = 10;
    plat_sfx(SFX_HURT);
    spawn_parts(px + PW / 2, py + 4, 12, 0xFFFFFFFF, 1.8f, 0.4f);
    return;
  }
  dead_t = 1; pvy = -5.0f; pvx = 0; power = 0;
  plat_sfx(SFX_HURT);
}
static void hurt(void) { hurt_ex(0); }

static void open_box(int tx, int ty) {
  if (tile(tx, ty) != '?') return;
  G[ty][tx] = 'u';
  int k = BOXK[ty][tx];
  if (k == 0) {
    for (int i = 0; i < 5; i++) { bananas++; total_bananas++; }
    if (bananas >= 50) { bananas -= 50; lives++; plat_sfx(SFX_LIFE); } else plat_sfx(SFX_BANANA);
    spawn_parts(tx * T + 8, ty * T - 2, 14, 0xFFF7D038, 2.0f, 1.2f);
  } else if (nents < MAXENT) {
    Ent* e = &ents[nents++];
    memset(e, 0, sizeof *e);
    e->type = E_POW; e->alive = 1; e->k = k; e->vx = 0.8f;
    e->x = tx * T + 2; e->by = (ty - 1) * T + 2; e->y = ty * T; e->t = 0;
    plat_sfx(SFX_CHECK);
  }
  spawn_parts(tx * T + 8, ty * T, 6, 0xFFD69A55, 1.2f, 0.8f);
}

static void break_block(int tx, int ty) {
  G[ty][tx] = ' ';
  plat_sfx(SFX_STOMP);
  spawn_parts(tx * T + 8, ty * T + 8, 12, 0xFFB8AFA0, 2.2f, 0.8f);
}
static Ent* add_shot(float x, float y, float vx, float vy, int k) {
  Ent* e = NULL;
  for (int i = 0; i < nents && !e; i++) if (ents[i].type == E_SHOT && !ents[i].alive) e = &ents[i];
  if (!e) { if (nents >= MAXENT) return NULL; e = &ents[nents++]; }
  memset(e, 0, sizeof *e);
  e->type = E_SHOT; e->alive = 1; e->k = k; e->x = x; e->y = y; e->vx = vx; e->vy = vy;
  return e;
}
static void boss_damage(Ent* e) {
  boss_hp--; boss_inv = 70; plat_sfx(SFX_STOMP); plat_sfx(SFX_CHECK);
  spawn_parts(e->x + 15, e->y + 6, 18, 0xFFFFFFFF, 2.6f, 0.5f);
  if (boss_hp <= 0) e->dying = 1;
}

// the area Dino's attack hits right now (headbutt, or the sword's longer reach)
static int attack_box(float* ax, float* ay, float* aw, float* ah) {
  if (atk_t <= 0 || dead_t) return 0;
  if (power == 2) {
    if (atk_t < 4) return 0;
    *aw = 22; *ah = PH + 8; *ay = py - 6;
    *ax = face > 0 ? px + PW - 2 : px - 20;
  } else {
    if (atk_t < 6) return 0;
    *aw = 8; *ah = PH - 2; *ay = py + 1;
    *ax = face > 0 ? px + PW - 2 : px - 6;
  }
  return 1;
}

static void collect(int tx, int ty) {
  G[ty][tx] = ' ';
  bananas++; total_bananas++;
  spawn_parts(tx * T + 8, ty * T + 8, 6, 0xFFF7D038, 1.5f, 0.5f);
  if (bananas >= 50) { bananas -= 50; lives++; plat_sfx(SFX_LIFE); }
  else plat_sfx(SFX_BANANA);
}

// ============================================================ player
static void update_player(const Input* in) {
  int jp = in->jump && !prev_jump;
  if (dead_t) {
    dead_t++;
    pvy += 0.3f; py += pvy;
    if (dead_t > 80) {
      lives--;
      if (lives < 0) { state = ST_OVER; state_t = 0; lives = 3; return; }
      respawn();
    }
    return;
  }

  int dir = (in->right ? 1 : 0) - (in->left ? 1 : 0);
  if (wall_lock > 0) { wall_lock--; dir = 0; }
  if (inv_t > 0) inv_t--;
  if (atk_cd > 0) atk_cd--;
  int ap = in->attack && !prev_atk;
  prev_atk = in->attack;
  if (ap && atk_cd == 0 && atk_t == 0) {
    if (power == 2) { atk_t = 16; atk_cd = 22; pvx += face * 0.8f; plat_sfx(SFX_SWORD); }
    else if (power == 4) {   // throw a bouncing star (two at a time at most)
      int n = 0; for (int i = 0; i < nents; i++) if (ents[i].alive && ents[i].type == E_SHOT && ents[i].k == 1) n++;
      if (n < 2) { add_shot(face > 0 ? px + PW : px - 6, py + 3, face * 3.4f, -1.2f, 1); atk_cd = 14; plat_sfx(SFX_SWORD); }
    } else { atk_t = 18; atk_cd = 30; pvx = face * 4.2f; if (pvy > 0.5f) pvy = 0.5f; plat_sfx(SFX_DASH); }
  }
  if (atk_t > 0) {
    atk_t--;
    if (power != 2) { dir = 0; if (atk_t > 6) { pvx = face * 4.2f; if (pvy > 0.6f) pvy = 0.6f; if (frame % 2 == 0) spawn_parts(px + (face > 0 ? 0 : PW), py + PH - 3, 1, 0xFFE8E0C8, 0.5f, 0.1f); } }
    float ax, ay, aw, ah;
    if (attack_box(&ax, &ay, &aw, &ah)) {
      int tx0 = (int)floorf(ax / T), tx1 = (int)floorf((ax + aw) / T), ty0 = (int)floorf(ay / T), ty1 = (int)floorf((ay + ah) / T);
      for (int ty = ty0; ty <= ty1; ty++) for (int tx = tx0; tx <= tx1; tx++) { if (tile(tx, ty) == '?') { open_box(tx, ty); if (power != 2) { atk_t = 4; pvx = -face * 1.5f; } } else if (tile(tx, ty) == 'k' && power == 3) break_block(tx, ty); }
    }
  }
  // standing on ice: slow to get going, slow to stop
  int on_ice = 0;
  if (on_ground) {
    int ty = (int)floorf((py + PH + 1) / T);
    on_ice = tile((int)floorf((px + 1) / T), ty) == 'I' || tile((int)floorf((px + PW - 1) / T), ty) == 'I';
  }
  float acc = on_ice ? 0.07f : on_ground ? 0.32f : 0.22f, maxv = 2.1f;
  if (atk_t > 6 && power != 2) maxv = 4.2f;
  if (dir) { pvx += dir * acc; face = dir; if (pvx > maxv) pvx = maxv; if (pvx < -maxv) pvx = -maxv; }
  else if (wall_lock <= 0) { float f = on_ice ? 0.02f : on_ground ? 0.3f : 0.08f; if (pvx > f) pvx -= f; else if (pvx < -f) pvx += f; else pvx = 0; }

  if (jp) jbuf = 7; else if (jbuf > 0) jbuf--;
  if (on_ground) { coyote = 6; air_jumps = 1; } else if (coyote > 0) coyote--;

  // walls next to the player
  wall_dir = 0;
  if (!on_ground) {
    if (box_solid(px + PW, py + 2, 1, PH - 4)) wall_dir = 1;
    else if (box_solid(px - 1, py + 2, 1, PH - 4)) wall_dir = -1;
  }

  if (jbuf && coyote) {
    pvy = -5.3f; jbuf = 0; coyote = 0; on_ground = 0;
    plat_sfx(SFX_JUMP);
    spawn_parts(px + PW / 2, py + PH, 4, 0xFFE8E0C8, 0.8f, 0.3f);
  } else if (jbuf && wall_dir) {
    pvy = -5.0f; pvx = -wall_dir * 2.3f; face = -wall_dir; wall_lock = 9; jbuf = 0;
    plat_sfx(SFX_WALL);
    spawn_parts(px + (wall_dir > 0 ? PW : 0), py + PH / 2, 5, 0xFFE8E0C8, 1.0f, 0.2f);
  } else if (jbuf && power == 1 && air_jumps > 0) {
    pvy = -4.9f; jbuf = 0; air_jumps = 0;
    plat_sfx(SFX_JUMP2);
    spawn_parts(px + PW / 2, py + PH, 10, 0xFFFFFFFF, 1.4f, -0.4f);
  }

  if (pvy >= 0) spring_up = 0;
  float g = ((in->jump || spring_up) && pvy < 0) ? 0.28f : 0.55f;
  pvy += g;
  float maxfall = 5.5f;
  if (wall_dir && dir == wall_dir && pvy > 0) { maxfall = 1.1f; if (frame % 6 == 0) spawn_parts(px + (wall_dir > 0 ? PW : 0), py + 4, 1, 0xFFE8E0C8, 0.4f, 0); }
  if (pvy > maxfall) pvy = maxfall;

  // move x
  float nx = px + pvx;
  if (box_solid(nx, py, PW, PH)) {
    if (pvx > 0) nx = floorf((nx + PW) / T) * T - PW;
    else if (pvx < 0) nx = (floorf(nx / T) + 1) * T;
    pvx = 0;
  }
  px = nx;
  // move y
  int was_ground = on_ground;
  on_ground = 0;
  float ny = py + pvy;
  if (pvy > 0) {
    float bot0 = py + PH, bot1 = ny + PH;
    int x0 = (int)floorf(px / T), x1 = (int)floorf((px + PW - 0.01f) / T);
    int ty = (int)floorf((bot1 - 0.01f) / T);
    int hit = 0;
    for (int tx = x0; tx <= x1; tx++) {
      char c = tile(tx, ty);
      if (solid_c(c) || (c == '=' && bot0 <= ty * T + 0.01f)) hit = 1;
    }
    ride_i = -1;
    if (hit) { ny = ty * T - PH; pvy = 0; on_ground = 1; }
    else for (int i = 0; i < nents; i++) {   // moving platforms: land on them from above
      Ent* e = &ents[i];
      if (e->alive && e->type == E_PLAT && px + PW > e->x && px < e->x + 48 && bot0 <= e->y + 4 && bot1 >= e->y) { ny = e->y - PH; pvy = 0; on_ground = 1; ride_i = i; break; }
    }
  } else if (pvy < 0) {
    if (box_solid(px, ny, PW, PH)) {
      int ty = (int)floorf(ny / T);
      int x0 = (int)floorf((px + 1) / T), x1 = (int)floorf((px + PW - 1.01f) / T);
      for (int tx = x0; tx <= x1; tx++) { open_box(tx, ty); if (tile(tx, ty) == 'k' && power == 3) break_block(tx, ty); }
      ny = (ty + 1) * T; pvy = 0;
    }
  }
  py = ny;
  if (on_ground && !was_ground) { land_t = 6; spawn_parts(px + PW / 2, py + PH, 3, 0xFFE8E0C8, 0.6f, 0.2f); }
  if (land_t) land_t--;

  // tiles the player touches
  int x0 = (int)floorf(px / T), x1 = (int)floorf((px + PW - 0.01f) / T);
  int y0 = (int)floorf(py / T), y1 = (int)floorf((py + PH - 0.01f) / T);
  for (int ty = y0; ty <= y1; ty++) for (int tx = x0; tx <= x1; tx++) {
    char c = tile(tx, ty);
    if (c == 'o') collect(tx, ty);
    else if (c == '^' && py + PH > ty * T + 9) hurt();
    else if (c == 's' && pvy >= 0 && py + PH > ty * T + 8) {
      pvy = -8.6f; on_ground = 0; coyote = 0; spring_up = 1; plat_sfx(SFX_SPRING);
      spawn_parts(tx * T + 8, ty * T + 10, 6, 0xFFFFFFFF, 1.2f, 0.4f);
    } else if (c == 'c' && (check_x != tx * T + 3 || check_y != ty * T + T - PH)) {
      check_x = tx * T + 3; check_y = ty * T + T - PH;
      plat_sfx(SFX_CHECK);
      spawn_parts(tx * T + 8, ty * T, 10, 0xFF9BE37A, 1.5f, 0.6f);
    }
  }
  // goal: a flag on a pole, touch anywhere on the pole
  if (!(boss_i >= 0 && !boss_dead) && px + PW > goal_x * T + 5) {
    state = ST_CLEAR; state_t = 0; plat_sfx(SFX_CLEAR);
    spawn_parts(goal_x * T + 8, goal_y * T - 32, 24, 0xFFF7D038, 2.5f, 0.5f);
  }
  if (py > LH * T + 24) hurt_ex(1);

  if (on_ground && pvx != 0) run_t++; else run_t = 0;
}

static void update_boss(Ent* e) {
  float bw = 30, bh = 24;
  if (e->dying) {
    e->dying++;
    if (e->dying % 5 == 0) { spawn_parts(e->x + 4 + (e->dying * 7) % 24, e->y + (e->dying * 5) % 20, 8, 0xFFFFE27A, 2.4f, 0.6f); plat_sfx(SFX_STOMP); }
    if (e->dying > 80) { e->alive = 0; boss_dead = 1; boss_awake = 0; g_boss_music = 0; plat_sfx(SFX_LIFE); total_bananas += 20; bananas += 20; }
    return;
  }
  if (!boss_awake) {
    if (px > arena_x0 + 16 && !dead_t) { boss_awake = 1; g_boss_music = 1; e->t = 1; } else return;
  }
  if (boss_inv > 0) boss_inv--;
  float spd = 0.7f + (e->k >= 3 ? 0.2f : 0.28f) * (boss_max - boss_hp);
  if (BOSS_BEE(e->k)) {
    // the Queen Bee hovers out of reach, then swoops down low at Dino
    if (e->vx == 0) e->vx = -spd;
    e->vx = e->vx > 0 ? spd : -spd;
    e->x += e->vx;
    int per = e->k >= 4 ? 220 : 260, cyc = e->t % per;
    if (e->k == 6 && e->t % 90 == 45 && !dead_t) { float dx = px - (e->x + 15); add_shot(e->x + 12, e->y + 18, dx > 0 ? 1.4f : -1.4f, 0.9f, 0); plat_sfx(SFX_WALL); }   // the Baron spits seeds down at Dino
    float ty = cyc < per / 2 ? e->by + sinf(e->t * 0.08f) * 6 : e->by + 56;
    e->y += (ty - e->y) * 0.07f;
  } else {
    // the beetle kings march up and down and leap at Dino
    int grounded = e->y >= e->by;
    if (grounded && e->t % (e->k == 5 ? 90 : e->k == 3 ? 100 : e->k ? 120 : 140) == 0) { e->vy = e->k == 5 ? -6.6f : -5.6f; e->vx = (px > e->x + 15 ? 1 : -1) * spd * 1.6f; grounded = 0; }
    e->vy += 0.3f; e->y += e->vy;
    if (e->y >= e->by) {
      if (e->vy > 2) { plat_sfx(SFX_STOMP); spawn_parts(e->x + 15, e->y + bh, 14, 0xFFE8E0C8, 2.2f, 0.4f); }
      e->y = e->by; e->vy = 0; e->vx = e->vx > 0 ? spd : -spd;
    }
    e->x += e->vx;
  }
  if (e->x < arena_x0) { e->x = arena_x0; e->vx = fabsf(e->vx); }
  if (e->x + bw > arena_x1) { e->x = arena_x1 - bw; e->vx = -fabsf(e->vx); }
  if (dead_t || state != ST_PLAY) return;
  float ax, ay, aw, ah; int hit = 0;
  if (!boss_inv && attack_box(&ax, &ay, &aw, &ah) && ax + aw > e->x && ax < e->x + bw && ay + ah > e->y && ay < e->y + bh) hit = 1;
  if (px + PW > e->x + 2 && px < e->x + bw - 2 && py + PH > e->y + 2 && py < e->y + bh) {
    if (pvy > 0.5f && py + PH - e->y < 13) { pvy = -5.6f; if (!boss_inv) hit = 2; }
    else if (!hit && boss_inv < 40) hurt();
  }
  if (hit) {
    boss_damage(e);
    if (hit == 1) { pvx = -face * 3.0f; pvy = -2.5f; wall_lock = 12; atk_t = 0; }
  }
}

static void update_ents(void) {
  for (int i = 0; i < nents; i++) {
    Ent* e = &ents[i];
    if (!e->alive) continue;
    e->t++;
    if (e->type == E_BOSS) { update_boss(e); continue; }
    if (e->dying) { if (++e->dying > 24) e->alive = 0; continue; }
    float ew = 12, eh = 10;
    if (e->type == E_ICE) {
      if (e->k == 0) {
        if (!dead_t && fabsf(px + PW / 2 - (e->x + 6)) < 16 && py > e->y && py - e->y < 170) { e->k = 1; e->t = 0; }
      } else if (e->k == 1) {
        if (e->t > 20) { e->k = 2; e->vy = 0; }
      } else {
        e->vy += 0.22f; if (e->vy > 5) e->vy = 5;
        e->y += e->vy;
        if (box_solid(e->x + 3, e->y + 4, 6, 10) || e->y > LH * T) {
          e->alive = 0; plat_sfx(SFX_STOMP);
          spawn_parts(e->x + 6, e->y + 12, 10, 0xFFCFEFFF, 1.6f, 0.6f);
          continue;
        }
      }
      if (!dead_t && state == ST_PLAY && px + PW > e->x + 3 && px < e->x + 9 && py + PH > e->y && py < e->y + 13) {
        if (e->k == 2) { e->alive = 0; spawn_parts(e->x + 6, e->y + 8, 8, 0xFFCFEFFF, 1.6f, 0.4f); }
        hurt();
      }
      continue;
    }
    if (e->type == E_POW) {
      if (e->t <= 16) e->y -= 1.0f;          // pop up out of the box
      else {                                  // then slide off and land
        if (e->t < 70 && !box_solid(e->x + e->vx, e->y, 12, 12)) e->x += e->vx;
        e->vy += 0.3f; if (e->vy > 4) e->vy = 4;
        float ny = e->y + e->vy;
        int ty = (int)floorf((ny + 12) / T);
        int x0 = (int)floorf(e->x / T), x1 = (int)floorf((e->x + 11) / T), land = 0;
        for (int tx = x0; tx <= x1; tx++) { char c = tile(tx, ty); if (solid_c(c) || (c == '=' && e->y + 12 <= ty * T + 0.5f)) land = 1; }
        if (land && e->vy > 0) { e->y = ty * T - 12; e->vy = 0; } else e->y = ny;
        if (e->y > LH * T + 40) e->alive = 0;
      }
      if (!dead_t && state == ST_PLAY && px + PW > e->x && px < e->x + 12 && py + PH > e->y && py < e->y + 12) {
        power = e->k; e->alive = 0; air_jumps = 1;
#ifdef TRACE
        printf("got power %d at x=%.0f\n", power, px / 16);
#endif
        plat_sfx(SFX_LIFE);
        spawn_parts(e->x + 6, e->y + 6, 16, power == 2 ? 0xFFD8DCE6 : 0xFFFFFFFF, 2.0f, 0.5f);
      }
      continue;
    }
    if (e->type == E_PLAT) {
      float ox = e->x, s2 = 0.5f - 0.5f * cosf(e->t * 0.025f);
      if (e->k) e->y = e->by - e->vx * s2; else e->x = e->x0 + e->vx * s2;
      if (ride_i == i && !dead_t && state == ST_PLAY) {   // carry Dino along
        if (!box_solid(px + e->x - ox, py, PW, PH)) px += e->x - ox;
        if (!box_solid(px, e->y - PH, PW, PH)) py = e->y - PH;
      }
      continue;
    }
    if (e->type == E_SHOT) {
      e->x += e->vx; if (e->k == 0) e->y += e->vy;
      int gone = e->t > 120 || box_solid(e->x, e->y + 1, 6, 4) || e->y > LH * T + 20;
      if (e->k == 1 && !gone) {   // Dino's star bounces along the ground
        e->vy += 0.25f; e->y += e->vy;
        if (box_solid(e->x, e->y, 6, 6)) { e->y -= e->vy; e->vy = e->vy > 0 ? -2.8f : 0.5f; }
        if (frame % 3 == 0) spawn_parts(e->x + 3, e->y + 3, 1, 0xFFFFF2A0, 0.4f, 0);
        for (int j = 0; j < nents && !gone; j++) {
          Ent* o = &ents[j];
          if (!o->alive || o->dying) continue;
          if (IS_FOE(o->type) && e->x + 6 > o->x && e->x < o->x + 12 && e->y + 6 > o->y && e->y < o->y + 14) { o->dying = 1; gone = 1; plat_sfx(SFX_STOMP); spawn_parts(o->x + 6, o->y + 4, 10, 0xFFFFE27A, 2.0f, 0.5f); }
          else if (o->type == E_BOSS && boss_awake && e->x + 6 > o->x && e->x < o->x + 30 && e->y + 6 > o->y && e->y < o->y + 24) { if (!boss_inv) boss_damage(o); gone = 1; }
        }
      } else if (e->k == 0 && !gone && !dead_t && state == ST_PLAY) {
        float ax, ay, aw, ah;
        if (attack_box(&ax, &ay, &aw, &ah) && ax + aw > e->x && ax < e->x + 6 && ay + ah > e->y && ay < e->y + 6) gone = 1;
        else if (px + PW > e->x + 1 && px < e->x + 5 && py + PH > e->y + 1 && py < e->y + 5) { hurt(); gone = 1; }
      }
      if (gone) { e->alive = 0; spawn_parts(e->x + 3, e->y + 3, 5, e->k ? 0xFFFFF2A0 : 0xFF6A4A2A, 1.2f, 0.2f); }
      continue;
    }
    if (e->type == E_PLANT) eh = 14;
    if (e->type == E_FROG) {
      // sits still, then hops at Dino
      int grounded = 0;
      e->vy += 0.3f; if (e->vy > 5) e->vy = 5;
      float ny = e->y + e->vy;
      if (e->vy > 0) {
        int ty = (int)floorf((ny + eh) / T), x0 = (int)floorf(e->x / T), x1 = (int)floorf((e->x + ew - 1) / T);
        for (int tx = x0; tx <= x1; tx++) { char c = tile(tx, ty); if (solid_c(c) || (c == '=' && e->y + eh <= ty * T + 0.5f)) grounded = 1; }
        if (grounded) { ny = ty * T - eh; e->vy = 0; }
      } else if (box_solid(e->x, ny, ew, eh)) { e->vy = 0; ny = e->y; }
      e->y = ny;
      if (grounded) { if (e->t % 80 == 0 && fabsf(px - e->x) < 190) { e->vy = -4.5f; e->k = px > e->x ? 1 : -1; } }
      else if (e->k) { float nx = e->x + e->k * 1.25f; if (!box_solid(nx, e->y, ew, eh)) e->x = nx; }
      e->vx = e->k > 0 ? 0.5f : -0.5f;
      if (e->y > LH * T + 40) { e->alive = 0; continue; }
    } else if (e->type == E_PLANT) {
      // stays put and spits seeds at Dino
      e->vx = px > e->x ? 0.5f : -0.5f;
      if (e->t % 130 == 65 && fabsf(px - e->x) < 150 && fabsf(py - e->y) < 70 && !dead_t) { add_shot(e->x + 3 + (e->vx > 0 ? 9 : -9), e->y + 1, e->vx > 0 ? 1.7f : -1.7f, 0, 0); plat_sfx(SFX_WALL); }
    } else if (e->type == E_BAT) {
      // hangs asleep until Dino comes near, then chases him
      if (e->k == 0) { if (!dead_t && fabsf(px - e->x) < 110 && py > e->y - 12 && py - e->y < 190) e->k = 1; }
      else {
        float dx = px - e->x, dy = py + 2 - e->y, d = sqrtf(dx * dx + dy * dy) + 0.01f;
        float mx = dx / d * 0.95f, my = dy / d * 0.95f + sinf(e->t * 0.2f) * 0.5f;
        if (!box_solid(e->x + mx, e->y, ew, eh)) e->x += mx;
        if (!box_solid(e->x, e->y + my, ew, eh)) e->y += my;
        e->vx = dx > 0 ? 0.5f : -0.5f;
      }
    } else if (e->type == E_BEETLE || e->type == E_SPINY) {
      // walk, turn at walls and ledges
      float nx = e->x + e->vx;
      int ahead = e->vx < 0 ? (int)floorf(nx / T) : (int)floorf((nx + ew) / T);
      int below = (int)floorf((e->y + eh + 1) / T);
      int wall = box_solid(nx, e->y, ew, eh);
      char fl = tile(ahead, below);
      int ledge = !(solid_c(fl) || fl == '=');
      if (wall || ledge) e->vx = -e->vx; else e->x = nx;
    } else {
      // bees bob up and down and patrol a few tiles each way
      e->vx = ((e->t / 110) % 2) ? 0.6f : -0.6f;
      if (!box_solid(e->x + e->vx, e->y, ew, eh)) e->x += e->vx;
      e->y = e->by + sinf(e->t * 0.07f) * 14;
    }
    // touch the player
    if (!dead_t && state == ST_PLAY) {
      float ex = e->x + 1, ey = e->y + (e->type == E_BEETLE ? 0 : 2);
      float ax, ay, aw, ah;
      if (attack_box(&ax, &ay, &aw, &ah) && ax + aw > ex && ax < ex + ew - 2 && ay + ah > ey && ay < ey + eh) {
        if (e->type == E_SPINY && power != 2 && power != 3) {   // a bare head bounces off the spikes
          pvx = -face * 2.6f; pvy = -2.0f; atk_t = 0; wall_lock = 10; plat_sfx(SFX_WALL);
          continue;
        }
        e->dying = 1; plat_sfx(SFX_STOMP);
        spawn_parts(e->x + 6, e->y + 4, 10, e->type == E_BEETLE ? 0xFF3FA34D : 0xFFF7D038, 2.0f, 0.5f);
        continue;
      }
      if (px + PW > ex && px < ex + ew - 2 && py + PH > ey && py < ey + eh) {
        if (pvy > 0.5f && py + PH - ey < 9 && e->type != E_SPINY) {
          e->dying = 1; pvy = -4.6f; plat_sfx(SFX_STOMP);
          spawn_parts(e->x + 6, e->y + 4, 8, e->type == E_BEETLE ? 0xFF3FA34D : 0xFFF7D038, 1.5f, 0.3f);
        } else hurt();
      }
    }
  }
}

static void update_parts(void) {
  for (int i = 0; i < 160; i++) if (parts[i].life > 0) {
    Part* p = &parts[i];
    p->x += p->vx; p->y += p->vy; p->vy += 0.12f; p->vx *= 0.96f; p->life--;
  }
}

static void update_camera(int snap) {
  float tx = px + PW / 2 - SCR_W / 2 + face * 28, ty = py + PH / 2 - SCR_H / 2 - 10;
  if (tx < 0) tx = 0;
  if (tx > LW * T - SCR_W) tx = LW * T - SCR_W;
  if (ty > LH * T - SCR_H) ty = LH * T - SCR_H;
  if (ty < 0) ty = 0;
  if (snap) { camx = tx; camy = ty; }
  else { camx += (tx - camx) * 0.12f; camy += (ty - camy) * 0.12f; }
}

void game_update(const Input* in) {
  frame++; state_t++;
  int jp = in->jump && !prev_jump, sp = in->start && !prev_start;
  switch (state) {
    case ST_TITLE:
      if ((jp || sp) && state_t > 20) { lives = 3; bananas = 0; state = ST_MAP; state_t = 0; map_from = map_node; map_t = 99; plat_sfx(SFX_SELECT); }
      break;
    case ST_PLAY:
      if (intro_t) {   // nothing moves until the intro is over
        if (++intro_t > INTRO_HOLD + INTRO_FLY) intro_t = 0;
        if (intro_t == INTRO_HOLD) plat_sfx(SFX_DASH);
        break;
      }
      if (sp) { state = ST_PAUSE; plat_sfx(SFX_SELECT); break; }
      lvl_frames++;
      update_player(in);
      dino_tick();
      if (state == ST_PLAY || state == ST_CLEAR) update_ents();
      update_parts();
      // a helmet or star box fills up again whenever Dino does not have that power, so no path can be shut for good
      if (frame % 90 == 0 && !dead_t) {
        int pow_out = 0; for (int i = 0; i < nents; i++) if (ents[i].alive && ents[i].type == E_POW) pow_out = 1;
        if (!pow_out) for (int y = 0; y < LH; y++) for (int x = 0; x < LW; x++) if (G[y][x] == 'u' && BOXK[y][x] >= 3 && BOXK[y][x] != power) G[y][x] = '?';
      }
      if (!dead_t) update_camera(0);
      break;
    case ST_PAUSE:
      if (sp) { state = ST_PLAY; plat_sfx(SFX_SELECT); }
      else if (in->down && !prev_down) { state = ST_MAP; state_t = 0; map_t = 99; }
      break;
    case ST_CLEAR:
      update_parts(); dino_tick();
      if (state_t > 150) {
        int first_all = !cleared[lvl + 1];
        cleared[lvl + 1] = 1;
        for (int i = 1; i <= NLEVELS; i++) if (!cleared[i]) first_all = 0;
        save_progress();
        if (first_all) { state = ST_WIN; state_t = 0; }
        else { state = ST_MAP; state_t = 0; map_t = 99; }
      }
      break;
    case ST_OVER:
      if (state_t == 1) { over_sel = 0; save_progress(); }
      if (state_t == 82 || state_t == 102) plat_sfx(SFX_STOMP);   // bump, bump: he lands on his bottom
      if (state_t > 125) {
        if ((in->left && !prev_left) || (in->right && !prev_right) || (in->up && !prev_up) || (in->down && !prev_down)) { over_sel ^= 1; plat_sfx(SFX_WALL); }
        if (jp || sp) {
          plat_sfx(SFX_SELECT);
          if (over_sel == 0) { lives = 3; start_level(lvl); update_camera(1); }
          else { state = ST_TITLE; state_t = 0; }
        }
      }
      break;
    case ST_WIN:
      if ((jp || sp) && state_t > 60) { state = ST_MAP; state_t = 0; map_t = 99; }
      break;
    case ST_MAP:
      if (map_t < 24) { map_t++; break; }
      {
        int fwd = (in->right && !prev_right) || (in->up && !prev_up);
        int back = (in->left && !prev_left) || (in->down && !prev_down);
        int kind = WORLDS[world].kind[map_node];
        if (fwd && map_node < WORLDS[world].n - 1 && node_open(map_node + 1)) { map_from = map_node; map_node++; map_t = 0; plat_sfx(SFX_WALL); }
        else if (back && map_node > 0) { map_from = map_node; map_node--; map_t = 0; plat_sfx(SFX_WALL); }
        else if (jp && kind >= 0) { start_level(kind); update_camera(1); plat_sfx(SFX_SELECT); }
        else if (jp && kind == -3 && world < NWORLDS - 1) { world++; map_node = map_from = 0; map_t = 99; state_t = 0; plat_sfx(SFX_CHECK); }
        else if (jp && kind == -2 && world > 0) { world--; map_node = map_from = WORLDS[world].n - 1; map_t = 99; state_t = 0; plat_sfx(SFX_CHECK); }
      }
      break;
  }
  prev_jump = in->jump; prev_start = in->start; prev_down = in->down;
  prev_up = in->up; prev_left = in->left; prev_right = in->right;
}

// ============================================================ drawing
static void blit_level(int CX, int CY) {
  for (int y = 0; y < FB_H; y++) {
    int sy = y + CY;
    if (sy < 0 || sy >= LVB.h || !LV_ROW[sy]) continue;
    const uint32_t* src = LVB.px + sy * LVB.w;
    uint32_t* dst = FB + y * FB_W;
    int x0 = CX < 0 ? -CX : 0, x1 = LVB.w - CX < FB_W ? LVB.w - CX : FB_W;
    for (int x = x0; x < x1; x++) {
      uint32_t c = src[x + CX], a = c >> 24;
      if (!a) continue;
      if (a == 255) dst[x] = c;
      else dst[x] = lerpc(dst[x], c, a + (a >> 7));
    }
  }
}
static void fb_shadow(float cx, float cy, float rx, float ry, float alpha) {
  for (int y = (int)(cy - ry - 1); y <= (int)(cy + ry + 1); y++) for (int x = (int)(cx - rx - 1); x <= (int)(cx + rx + 1); x++)
    fb_over(x, y, 0xFF000000, cov(sd_ellipse(x + 0.5f, y + 0.5f, cx, cy, rx, ry)) * alpha);
}
// one frame of a newer creature (fr picks the pose)
static void critter3d(Bmp* b, int type, int fr, float yaw, int dying, float S, int crown) {
  float sq = dying ? maxf(0.3f, 1 - dying * 0.03f) : 1;
  if (type == E_FROG) frog3d(b, dying ? sq : fr == 7 ? 1.28f : 1 + 0.04f * sinf(fr * 1.5708f), yaw, dying, S, crown);
  else if (type == E_SPINY) spiny3d(b, fr * 0.7854f, sq, yaw, dying, S);
  else if (type == E_PLANT) plant3d(b, fr == 3 ? 1 : fr * 0.25f, sq, yaw, dying, S);
  else bat3d(b, fr == 7 ? -1.5f : sinf(fr * 1.0472f), yaw + dying * 0.3f, dying, S, crown);
}
static void make_extra_art(void) {
  if (HD_PLAT.px) return;
  HD_PLAT = bmp_new(96, 22); HD_CRACK = bmp_new(32, 32); HD_HELMI = bmp_new(32, 32); HD_STARI = bmp_new(32, 32); HD_HELM = bmp_new(36, 22);
  for (int y = 0; y < 22; y++) for (int x = 0; x < 96; x++) {   // a wooden platform with a bright top
    float fx = x + 0.5f, fy = y + 0.5f, dd = sd_rrect(fx, fy, 48, 10, 47, 9, 5);
    if (dd > 1) continue;
    uint32_t* d = &HD_PLAT.px[y * 96 + x];
    uint32_t c = fy < 7 ? mixc(0xFFFFE9A8, 0xFFF2B85A, fy / 7) : mixc(0xFFC8823C, 0xFF8A5226, (fy - 7) / 12);
    if (x % 24 < 2 && fy > 7) c = 0xFF6B3F1D;
    over(d, OUTLINE, cov(dd)); over(d, c, cov(dd + 2));
  }
  for (int y = 0; y < 32; y++) for (int x = 0; x < 32; x++) {   // a cracked stone block
    float fx = x + 0.5f, fy = y + 0.5f, dd = sd_rrect(fx, fy, 16, 16, 15.5f, 15.5f, 4);
    uint32_t* d = &HD_CRACK.px[y * 32 + x];
    uint32_t c = mixc(0xFFC9BFAE, 0xFF8F8576, clampf((fx + fy) / 64 + (hashf(x / 3, y / 3) - 0.5f) * 0.25f, 0, 1));
    float cr = minf(minf(sd_seg(fx, fy, 6, 3, 13, 12), sd_seg(fx, fy, 13, 12, 9, 20)), minf(minf(sd_seg(fx, fy, 13, 12, 22, 15), sd_seg(fx, fy, 22, 15, 26, 27)), sd_seg(fx, fy, 9, 20, 12, 29)));
    over(d, OUTLINE, cov(dd)); over(d, c, cov(dd + 2)); over(d, 0xFF3A322A, cov(cr - 1.1f) * cov(dd + 2));
    if (dd > -4 && dd < -2 && fy < 16) over(d, 0xFFFFFFFF, 0.35f);
  }
  for (int y = 0; y < 32; y++) for (int x = 0; x < 32; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    // helmet icon: a dome with a brim
    float dome = maxf(sd_circle(fx, fy, 16, 19, 11), fy - 20), brim = sd_rrect(fx, fy, 16, 22, 14, 2.5f, 2), dall = minf(dome, brim);
    uint32_t* d = &HD_HELMI.px[y * 32 + x];
    if (dall < 3) { over(d, OUTLINE, cov(dall - 1.6f)); over(d, mixc(0xFFFFD27A, 0xFFE0862C, clampf((fy - 8) / 14, 0, 1)), cov(dome)); over(d, 0xFFB8641E, cov(brim)); over(d, 0xFFFFFFFF, cov(sd_circle(fx, fy, 12, 13, 2.2f)) * 0.8f); }
    // star icon: five points
    float a = atan2f(fy - 16, fx - 16) + 1.5708f, r = sqrtf((fx - 16) * (fx - 16) + (fy - 16) * (fy - 16));
    float k = fabsf(fmodf(a + 6.28318f * 3, 1.25664f) - 0.62832f) / 0.62832f, edge = 6 + 7 * k * k, ds = r - edge;
    d = &HD_STARI.px[y * 32 + x];
    if (ds < 3) { over(d, OUTLINE, cov(ds - 1.6f)); over(d, mixc(0xFFFFF2A0, 0xFFF2B02E, clampf(r / 12, 0, 1)), cov(ds)); }
  }
  for (int y = 0; y < 22; y++) for (int x = 0; x < 36; x++) {   // the helmet as Dino wears it
    float fx = x + 0.5f, fy = y + 0.5f;
    float dome = maxf(sd_circle(fx, fy, 18, 17, 13), fy - 17), brim = sd_rrect(fx, fy, 18, 18, 16, 2.2f, 2), dall = minf(dome, brim);
    uint32_t* d = &HD_HELM.px[y * 36 + x];
    if (dall < 3) { over(d, OUTLINE, cov(dall - 1.4f)); over(d, mixc(0xFFFFD27A, 0xFFE0862C, clampf((fy - 4) / 14, 0, 1)), cov(dome)); over(d, 0xFFB8641E, cov(brim)); over(d, 0xFFFFFFFF, cov(sd_circle(fx, fy, 13, 9, 2.4f)) * 0.8f); }
  }
}
static void draw_world(void) {
  make_extra_art();
  int CX = (int)floorf(camx * 2), CY = (int)floorf(camy * 2);
  int tx0 = CX / HT, ty0 = CY / HT;
  blit_level(CX, CY);
  for (int i = 0; i < nsigns; i++) hblit(&HD_SIGN, sign_x[i] * HT - CX, sign_y[i] * HT - CY, 0);
  // goal pole with a rippling flag that runs up the pole when you finish
  {
    float X = goal_x * HT - CX, Y = goal_y * HT - CY;
    if (X > -80 && X < FB_W + 20) {
      int raised = state == ST_CLEAR ? (state_t < 40 ? state_t : 40) : 0;
      fb_capsule(X + 8, Y + 28, X + 24, Y + 28, 5, OUTLINE); fb_capsule(X + 8, Y + 28, X + 24, Y + 28, 3.4f, 0xFF8C8F9C);
      fb_capsule(X + 16, Y - 92, X + 16, Y + 26, 4.4f, OUTLINE);
      fb_capsule(X + 16, Y - 92, X + 16, Y + 26, 2.8f, 0xFFE8ECF4); fb_capsule(X + 17.3f, Y - 92, X + 17.3f, Y + 26, 1.2f, 0xFFAEB6C8);
      fb_flag(X + 19, Y - 86 + 56 - raised * 1.4f, 46, 28, 0xFFF0584A, 0xFFC8322C, 1, 1);
      fb_dot(X + 16, Y - 96, 7.6f, OUTLINE, 1); fb_dot(X + 16, Y - 96, 6, 0xFFF7D038, 1); fb_dot(X + 14, Y - 98, 2.2f, 0xFFFFF7C8, 1);
    }
  }
  for (int ty = ty0; ty <= ty0 + FB_H / HT + 1; ty++)
    for (int tx = tx0 - 1; tx <= tx0 + FB_W / HT + 1; tx++) {
      char c = tile(tx, ty);
      if (c == ' ' || c == '#' || c == 'X' || tx < 0 || tx >= LW) continue;
      int X = tx * HT - CX, Y = ty * HT - CY;
      if (c == 'o') hblit(&HD_COIN[(frame / 2 + tx * 5) % COIN_FRAMES], X, Y + (int)lroundf(sinf((frame + tx * 9) * 0.08f) * 2), 0);
      else if (c == '?') hblit(&HD_QBOX[0], X - 4, Y - 6, 0);
      else if (c == 'u') hblit(&HD_QBOX[1], X - 4, Y - 6, 0);
      else if (c == 'k') hblit(&HD_CRACK, X, Y, 0);
      else if (c == 's') hblit(&HD_SPRING, X, Y, 0);
      else if (c == 'c') {
        int on = check_x == tx * T + 3 && check_y == ty * T + T - PH;
        fb_capsule(X + 10, Y + 30, X + 22, Y + 30, 3.4f, OUTLINE); fb_capsule(X + 10, Y + 30, X + 22, Y + 30, 2, 0xFF8C8F9C);
        fb_capsule(X + 16, Y + 3, X + 16, Y + 29, 3.4f, OUTLINE); fb_capsule(X + 16, Y + 3, X + 16, Y + 29, 1.9f, 0xFFB5793A);
        fb_flag(X + 18, Y + 3, 22, 14, on ? 0xFF7FE06A : 0xFFC4C8D2, on ? 0xFF3FA34D : 0xFF8C8F9C, on ? 1 : 0.15f, 0);
      }
    }
  for (int i = 0; i < nents; i++) {
    Ent* e = &ents[i];
    if (!e->alive) continue;
    float ex = e->x * 2 - CX, ey = e->y * 2 - CY;
    if (ex < -80 || ex > FB_W + 80 || ey < -80 || ey > FB_H + 80) continue;
    if (e->type == E_ICE) {
      hblit(&HD_ICICLE, (int)ex + (e->k == 1 ? (int)lroundf(sinf(e->t * 1.6f) * 2) : 0), (int)ey, 0);
      continue;
    }
    if (e->type == E_POW) {
      float bob = sinf(e->t * 0.1f) * 3;
      fb_dot(ex + 12, ey + 12 + bob, 17 + sinf(e->t * 0.15f) * 2, 0xFFFFFFFF, 0.28f);
      hblit(e->k == 2 ? &HD_SWORDI : e->k == 3 ? &HD_HELMI : e->k == 4 ? &HD_STARI : &HD_WINGI, (int)ex - 4, (int)(ey - 4 + bob), 0);
      continue;
    }
    if (e->type == E_PLAT) { hblit(&HD_PLAT, (int)ex, (int)ey, 0); continue; }
    if (e->type == E_SHOT) {
      if (e->k) { fb_dot(ex + 6, ey + 6, 8, OUTLINE, 1); fb_dot(ex + 6, ey + 6, 6.4f, 0xFFF7D038, 1); fb_dot(ex + 4.5f, ey + 4.5f, 2.6f, 0xFFFFFBE0, 1); }
      else { fb_dot(ex + 6, ey + 6, 6.5f, OUTLINE, 1); fb_dot(ex + 6, ey + 6, 5, 0xFF7A5230, 1); fb_dot(ex + 4.5f, ey + 4.5f, 1.8f, 0xFFC89A6A, 1); }
      continue;
    }
    float want = e->vx > 0 ? 1 : -1;
    if (e->type == E_BOSS && !boss_awake) want = -0.3f;
    if (e->fz == 0) e->fz = want;
    e->fz += (want - e->fz) * 0.22f;   // turns round smoothly instead of snapping
    // A creature walking or flying steadily repeats the same few frames, so each frame of each
    // model is drawn once (the first time it is needed) and kept. Only turning round and being
    // squashed are drawn fresh.
    int steady = !e->dying && fabsf(e->fz) > 0.97f, dir = e->fz > 0;
    if (e->type == E_BOSS) {
      float S = 2.4f;
      if (boss_inv && (frame / 3) % 2 && !e->dying) continue;   // flashes after a hit
      const Bmp* pic = &ESCR;
      if (e->k >= 5) {   // King Croak the frog and the Bat Baron
        int bat = e->k == 6, fr = bat ? (e->t / 2) % 6 : (e->y < e->by - 1 ? 7 : (e->t / 10) % 4), ty = bat ? E_BAT : E_FROG;
        if (!bat) { float air = e->by - e->y; fb_shadow(ex + 30, (e->by + 24) * 2 - CY + 1, 30 - air * 0.2f, 5, 0.2f); }
        if (steady) { Bmp* c = &BOSS_C[dir][fr]; if (!c->px) critter3d(c, ty, fr, dir ? 1.0f : -1.0f, 0, S, 1); pic = c; }
        else { RT_SS = 1; critter3d(&ESCR, ty, fr, e->fz, e->dying, S, 1); RT_SS = 2; }
        hblit(pic, (int)ex + 30 - (int)(26 * S), (int)ey + (bat ? 24 - (int)(36 * S) + e->dying : 48 - (int)(46 * S)), 0);
        continue;
      }
      if (BOSS_BEE(e->k)) {
        if (steady) { Bmp* c = &QBEE_C[dir][e->t % 8]; if (!c->px) bee3d(c, (e->t % 8) * 0.7854f, 0, dir ? 1.0f : -1.0f, 0, S, 1); pic = c; }
        else { RT_SS = 1; bee3d(&ESCR, (e->t % 8) * 0.7854f, 0, e->fz * 1.0f + e->dying * 0.3f, e->dying, S, 1); RT_SS = 2; }
        hblit(pic, (int)ex + 30 - (int)(28 * S), (int)ey + 24 - (int)(28 * S) + e->dying, 0);
      } else {
        float air = e->by - e->y;
        fb_shadow(ex + 30, (e->by + 24) * 2 - CY + 1, 30 - air * 0.2f, 5, 0.2f);
        if (steady) { Bmp* c = &BOSS_C[dir][e->t % 20]; if (!c->px) beetle3d(c, (e->t % 20) * 0.31416f, 1, dir ? 1.0f : -1.0f, BOSS_COL(e->k), 0, 0, S, 1); pic = c; }
        else { RT_SS = 1; beetle3d(&ESCR, (e->t % 20) * 0.31416f, e->dying ? maxf(0.3f, 1 - e->dying * 0.02f) : 1, e->fz * 1.0f + e->dying * 0.25f, BOSS_COL(e->k), e->dying, e->t, S, 1); RT_SS = 2; }
        hblit(pic, (int)ex + 30 - (int)(26 * S), (int)ey + 48 - (int)(36 * S), 0);
      }
      continue;
    }
    if (e->type >= E_FROG && e->type <= E_BAT) {
      const Bmp* pic = &ESCR;
      int ti = e->type - E_FROG, fr = 0, eh2 = e->type == E_PLANT ? 28 : 20;
      if (e->type == E_FROG) fr = e->vy != 0 ? 7 : (e->t / 10) % 4;
      else if (e->type == E_SPINY) fr = e->t % 8;
      else if (e->type == E_PLANT) { int c = e->t % 130; fr = c > 45 && c < 75 ? 3 : (e->t / 12) % 2; }
      else fr = e->k == 0 ? 7 : (e->t / 2) % 6;
      if (steady) { Bmp* c = &CRIT_C[ti][dir][fr]; if (!c->px) critter3d(c, e->type, fr, dir ? 1.15f : -1.15f, 0, 1, 0); pic = c; }
      else { RT_SS = 1; critter3d(&ESCR, e->type, fr, e->fz * 1.15f, e->dying, 1, 0); RT_SS = 2; }
      if (e->type == E_BAT) hblit(pic, (int)ex + 12 - 26, (int)ey + 10 - 28 + e->dying * e->dying / 3, 0);
      else { if (!e->dying) fb_shadow(ex + 12, ey + eh2 + 1, 12, 2.5f, 0.18f); hblit(pic, (int)ex + 12 - 26, (int)ey + eh2 - 38, 0); }
    } else if (e->type == E_BEETLE) {
      const Bmp* pic = &ESCR;
      int sn = L->theme >= 5 && L->theme <= 7;
      if (steady) { Bmp* c = &BEETLE_C[sn][dir][e->t % 20]; if (!c->px) beetle3d(c, (e->t % 20) * 0.31416f, 1, dir ? 1.15f : -1.15f, sn, 0, 0, 1, 0); pic = c; }
      else beetle3d(&ESCR, (e->t % 20) * 0.31416f, e->dying ? maxf(0.28f, 1 - e->dying * 0.14f) : 1, e->fz * 1.15f, sn, e->dying, e->t, 1, 0);
      if (!e->dying) fb_shadow(ex + 12, ey + 21, 12, 2.5f, 0.18f);
      hblit(pic, (int)ex + 12 - 26, (int)ey + 20 - 36, 0);
    } else {
      const Bmp* pic = &ESCR;
      if (steady) { Bmp* c = &BEE_C[dir][e->t % 8]; if (!c->px) bee3d(c, (e->t % 8) * 0.7854f, 0, dir ? 1.15f : -1.15f, 0, 1, 0); pic = c; }
      else bee3d(&ESCR, (e->t % 8) * 0.7854f, e->dying ? e->dying * 0.35f : 0, e->fz * 1.15f, e->dying, 1, 0);
      hblit(pic, (int)ex + 12 - 28, (int)ey + 12 - 28 + e->dying * e->dying / 3, 0);
    }
  }
  // Dino
  {
    DinoPose p; dino_pose(&p);
    int walling = !on_ground && wall_dir && pvy > 0 && !dead_t;
    int flip = face < 0;
    if (walling) flip = wall_dir > 0;   // back to the wall
    int X = (int)floorf((px + PW * 0.5f) * 2) - CX, Y = (int)floorf((py + PH) * 2) - CY;
    int show = !(inv_t && (inv_t / 4) % 2);
    if (on_ground && !dead_t && !intro_t) fb_shadow(X, Y + 1, 12, 2.8f, 0.2f);
    dino_sx = X; dino_sy = Y;
    if (intro_t) show = 0;
    if (show) { if (g_lowq) RT_SS = 1; dino3d(&DSCR, &p, DA.yaw, 1); RT_SS = 2; hblit(&DSCR, X - 36, Y - 64, 0); }
    if (power == 3 && show) hblit(&HD_HELM, X - 18 + (flip ? -2 : 2), Y - 62, 0);
    if (power == 2 && show && !dead_t) {
      // the sword: held up behind him, or swung in an arc
      float hx = X + (flip ? -7 : 7), hy = Y - 22, ang;
      if (atk_t > 0) { float k = (16 - atk_t) / 16.0f; ang = -2.2f + k * k * (3 - 2 * k) * 3.0f; }
      else ang = -2.0f + sinf(frame * 0.07f) * 0.05f;
      if (atk_t > 2) for (int q = 5; q >= 1; q--) {   // a fading trail behind the blade
        float a2 = ang - q * 0.16f; if (flip) a2 = 3.14159f - a2;
        fb_capsule(hx + cosf(a2) * 16, hy + sinf(a2) * 16, hx + cosf(a2) * 32, hy + sinf(a2) * 32, 2.2f, mixc(0xFFFFFFFF, 0xFFBFE8FF, q / 5.0f));
      }
      if (flip) ang = 3.14159f - ang;
      float ca = cosf(ang), sa = sinf(ang);
      fb_capsule(hx + ca * 2, hy + sa * 2, hx + ca * 32, hy + sa * 32, 3.8f, OUTLINE);
      fb_capsule(hx + ca * 9 - sa * 5, hy + sa * 9 + ca * 5, hx + ca * 9 + sa * 5, hy + sa * 9 - ca * 5, 3.4f, OUTLINE);
      fb_capsule(hx + ca * 2, hy + sa * 2, hx + ca * 8, hy + sa * 8, 2, 0xFF8B5A2B);
      fb_capsule(hx + ca * 11, hy + sa * 11, hx + ca * 32, hy + sa * 32, 2.3f, 0xFFF4F7FC);
      fb_capsule(hx + ca * 11 + sa, hy + sa * 11 - ca, hx + ca * 30 + sa, hy + sa * 30 - ca, 0.8f, 0xFFAEB6C8);
      fb_capsule(hx + ca * 9 - sa * 5, hy + sa * 9 + ca * 5, hx + ca * 9 + sa * 5, hy + sa * 9 - ca * 5, 1.9f, 0xFFF2B02E);
    }
  }
  for (int i = 0; i < 160; i++) if (parts[i].life > 0)
    fb_dot(parts[i].x * 2 - CX, parts[i].y * 2 - CY, parts[i].life > 10 ? 2.8f : 1.2f + parts[i].life * 0.15f, parts[i].c, parts[i].life > 8 ? 1 : parts[i].life / 8.0f);
  // sign text when you stand near a sign
  for (int i = 0; i < nsigns; i++) {
    float dx = px + PW / 2 - (sign_x[i] * T + 8), dy = py - sign_y[i] * T;
    if (dx * dx < 48 * 48 && dy * dy < 40 * 40) {
      const char* s = L->signs[i];
      int w = text_w(s, 1) + 8;
      int bx = SCR_W / 2 - w / 2, by = 30;
      fill(bx, by, w, 13, 0xFF1B1020);
      fill(bx + 1, by + 1, w - 2, 11, 0xFFFDF3DC);
      text_raw(s, bx + 4, by + 4, 1, 0xFF3B2414);
    }
  }
}

// a still Dino for the HUD, the title and the map
static void draw_dino_at(int X, int Y, float S, const DinoPose* p, float yaw) {
  static Bmp big;
  if (S > 1.5f) RT_SS = 1;   // when he is drawn big, skip the draw-double-and-shrink step: four times quicker
  dino3d(&big, p, yaw, S);
  RT_SS = 2;
  hblit(&big, X - (int)(36 * S), Y - (int)(64 * S), 0);
}

static int hud_cx, hud_cy, hud_lx, hud_ly;   // how far the coin and lives counters are from their usual places
static void draw_hud(void) {
  char buf[64];
  hblit(&HD_COIN[(frame / 3) % COIN_FRAMES], 6 + hud_cx * 2, 3 + hud_cy * 2, 0);
  snprintf(buf, sizeof buf, "%02d", bananas);
  text(buf, 21 + hud_cx, 6 + hud_cy, 2, 0xFFFFF4C2);
  static Bmp hud_dino;   // the little Dino by the lives count is drawn once and kept
  if (!hud_dino.px) { DinoPose p; dino_idle_pose(&p, 50); dino3d(&hud_dino, &p, 0.5f, 0.72f); }
  hblit(&hud_dino, FB_W - 94 - (int)(36 * 0.72f) + hud_lx * 2, 34 - (int)(64 * 0.72f) + hud_ly * 2, 0);
  snprintf(buf, sizeof buf, "x%d", lives < 0 ? 0 : lives);
  text(buf, SCR_W - 34 + hud_lx, 6 + hud_ly, 2, 0xFFFFFFFF);
  if (intro_t) return;
  if (boss_awake && boss_i >= 0) {
    const char* nm = BOSS_NAME[ents[boss_i].k];
    text(nm, (SCR_W - text_w(nm, 1)) / 2, 4, 1, 0xFFFFFFFF);
    for (int i = 0; i < boss_max; i++) {
      float hx = FB_W / 2 - (boss_max - 1) * 11 + i * 22, hy = 32;
      fb_dot(hx, hy, 8.5f, 0xFF1B1020, 1); fb_dot(hx, hy, 6.5f, i < boss_hp ? 0xFFF0584A : 0xFF4A4458, 1);
      if (i < boss_hp) fb_dot(hx - 2, hy - 2, 2, 0xFFFFC0B8, 1);
    }
  } else if (boss_i >= 0 && !boss_dead && px > arena_x0 - 200) text_c("DEFEAT THE BOSS TO REACH THE FLAG", 24, 1, 0xFFFFF4C2);
  if (power) {
    fill(SCR_W - 82, 1, 18, 18, 0xFF1B1020); fill(SCR_W - 81, 2, 16, 16, 0xFF3B2F6B);
    make_extra_art();
    hblit(power == 2 ? &HD_SWORDI : power == 3 ? &HD_HELMI : power == 4 ? &HD_STARI : &HD_WINGI, (SCR_W - 81) * 2, 4, 0);
  }
}

static void draw_kip_big(int x, int y, int sc, const char* const* s, int flip) {
  for (int j = 0; j < 20; j++) for (int i = 0; i < 20; i++) {
    char c = s[j][flip ? 19 - i : i];
    if (c != '.') fill(x + i * sc, y + j * sc, sc, sc, pal(c));
  }
}

static void disc(int cx, int cy, int r, uint32_t c) {
  for (int j = -r; j <= r; j++) for (int i = -r; i <= r; i++) if (i * i + j * j <= r * r) put(cx + i, cy + j, c);
}
static void pine(int x, int y, uint32_t green) {
  fill(x - 1, y, 3, 5, 0xFF6B3F1D);
  for (int t = 0; t < 3; t++) for (int j = 0; j < 6; j++) {
    int hw = 1 + j * (3 + t) / 5;
    for (int i = -hw; i <= hw; i++) put(x + i, y - 15 + t * 4 + j, j < 2 ? 0xFFFFFFFF : green);
  }
}
static void boat(int x, int y) {
  for (int j = 0; j < 5; j++) fill(x - 9 + j, y + j, 18 - 2 * j, 1, j == 0 ? 0xFF3B2414 : 0xFF8A5A2B);
  fill(x, y - 14, 1, 14, 0xFF3B2414);
  for (int j = 0; j < 11; j++) fill(x + 1, y - 13 + j, 1 + j * 7 / 11, 1, 0xFFFFFFFF);
}
static void map_jungle(void) {
  // the island: a blob of circles, sand round the edge
  static const short BLOB[][3] = { {40, 140, 30}, {84, 130, 34}, {130, 110, 34}, {100, 66, 34}, {166, 56, 34},
    {226, 92, 38}, {280, 52, 32}, {150, 90, 30}, {190, 130, 28}, {60, 100, 24}, {250, 140, 22} };
  int nb = sizeof BLOB / sizeof BLOB[0];
  for (int b = 0; b < nb; b++) disc(BLOB[b][0], BLOB[b][1], BLOB[b][2] + 5, 0xFFF0D9A0);
  for (int b = 0; b < nb; b++) disc(BLOB[b][0], BLOB[b][1], BLOB[b][2], 0xFF5DB85A);
  for (int y = 0; y < SCR_H; y++) for (int x = 0; x < SCR_W; x++)
    if (FB[y * SCR_W + x] == 0xFF5DB85A && hash2(x, y) % 9 == 0) put(x, y, 0xFF4FA64C);
  // mountain with the beetle cave (level 5)
  for (int j = 0; j < 46; j++) for (int i = -j; i <= j; i++) put(226 + i, 50 + j, j < 10 ? 0xFFF4F4F4 : ((i + j) % 9 ? 0xFF8C8F9C : 0xFF757884));
  disc(226, 92, 9, 0xFF2A2220); fill(217, 92, 19, 9, 0xFF2A2220);
  // cliff and cloud for the sky bridge (level 6)
  fill(270, 30, 24, 26, 0xFF8A5A2B); fill(270, 30, 24, 3, 0xFF5DB85A);
  for (int k = 0; k < 3; k++) disc(270 + k * 12, 24 - (k % 2) * 4, 9, 0xFFFFFFFF);
  // waterfall (level 3)
  for (int y = 10; y < 66; y++) for (int i = -5; i <= 5; i++) {
    int streak = ((y - frame + i * 5) % 9 + 9) % 9 < 2;
    put(96 + i, y, streak ? 0xFFEAF8FF : 0xFF8ED0EA);
  }
  fill(84, 4, 24, 8, 0xFF757884);
  // temple (level 4)
  for (int k = 0; k < 4; k++) fill(150 + k * 4, 20 + k * 6, 32 - k * 8, 7, k % 2 ? 0xFF8A7BA8 : 0xFF7A6B98);
  fill(163, 38, 6, 6, 0xFF3A2F5C);
  // trees
  static const short TREES[][2] = { {20, 120}, {60, 116}, {110, 128}, {150, 120}, {180, 138}, {200, 118}, {70, 80}, {128, 78}, {256, 130}, {62, 152}, {196, 72} };
  for (unsigned t = 0; t < sizeof TREES / sizeof TREES[0]; t++) {
    fill(TREES[t][0] - 1, TREES[t][1], 3, 6, 0xFF6B3F1D);
    disc(TREES[t][0], TREES[t][1] - 3, 6, 0xFF2E7D32); disc(TREES[t][0] - 2, TREES[t][1] - 5, 2, 0xFF4CAF50);
  }
  // Dino's house
  fill(26, 136, 24, 14, 0xFFE8D5B0); fill(35, 142, 6, 8, 0xFF6B3F1D);
  for (int j = 0; j < 11; j++) fill(38 - j - 3, 126 + j, 2 * j + 7, 1, 0xFF4C8C2B);
  // pier and boat to the Frosty Forest
  fill(292, 110, 12, 3, 0xFF8A5A2B); fill(294, 113, 2, 4, 0xFF5A3A1E); fill(300, 113, 2, 4, 0xFF5A3A1E);
  boat(308, 118 + (frame / 30) % 2);
}
static void map_snow(int night) {
  const World* w = &WORLDS[world];
  uint32_t rim = night ? 0xFF7F96CC : 0xFFBFE0F5, snow = night ? 0xFFC9D6F0 : 0xFFF4FAFF, shade = night ? 0xFFB3C2E4 : 0xFFDCEBF7;
  // the land follows the path, with a few extra lumps
  for (int pass = 0; pass < 2; pass++) {
    for (int n = 0; n < w->n; n++) {
      disc(w->xy[n][0], w->xy[n][1], 30 + 5 * !pass, pass ? snow : rim);
      if (n + 1 < w->n) disc((w->xy[n][0] + w->xy[n + 1][0]) / 2, (w->xy[n][1] + w->xy[n + 1][1]) / 2 + 8, 30 + 5 * !pass, pass ? snow : rim);
    }
    disc(120, 140, 26 + 5 * !pass, pass ? snow : rim); disc(250, 110, 30 + 5 * !pass, pass ? snow : rim); disc(190, 130, 24 + 5 * !pass, pass ? snow : rim);
  }
  for (int y = 0; y < SCR_H; y++) for (int x = 0; x < SCR_W; x++)
    if (FB[y * SCR_W + x] == snow && hash2(x, y) % 11 == 0) put(x, y, shade);
  uint32_t green = night ? 0xFF1F4A4A : 0xFF2E7D5A;
  if (!night) {
    // frozen lake by level 8, ice cave by level 9
    for (int j = -10; j <= 10; j++) for (int i = -22; i <= 22; i++) if (i * i / 484.0f + j * j / 100.0f <= 1)
      put(150 + i, 128 + j, (i + j * 2 + 60) % 13 < 2 ? 0xFFEAF8FF : 0xFFA9DAF5);
    for (int j = 0; j < 30; j++) for (int i = -j; i <= j; i++) put(214 + i, 40 + j, j < 8 ? 0xFFFFFFFF : ((i + j) % 9 ? 0xFF8FB4D4 : 0xFF6F98BC));
    disc(206, 68, 6, 0xFF12304C); fill(200, 68, 13, 6, 0xFF12304C);
    static const short P[][2] = { {50, 118}, {66, 150}, {104, 112}, {118, 88}, {172, 100}, {176, 62}, {236, 100}, {254, 124}, {246, 66}, {100, 150}, {30, 128} };
    for (unsigned t = 0; t < sizeof P / sizeof P[0]; t++) pine(P[t][0], P[t][1], green);
    boat(8, 156 + (frame / 30) % 2);
    // ski lift up to Ice Mountain
    fill(282, 20, 2, 26, 0xFF575A66); fill(276, 20, 14, 2, 0xFF575A66);
    for (int i = 0; i < 40; i++) put(283 + i, 20 - i / 2, 0xFF2B1A12);
    fill(296 + (frame / 8) % 6, 14, 6, 5, 0xFFD8463C);
  } else {
    // the mountain, with the summit at the last stop
    for (int j = 0; j < 100; j++) for (int i = -j * 3 / 4; i <= j * 3 / 4; i++) {
      int x = 238 + i, y = 34 + j;
      if ((unsigned)x < SCR_W && (unsigned)y < SCR_H && FB[y * SCR_W + x] != 0)
        put(x, y, j < 16 + (i * 7 % 5) ? 0xFFFFFFFF : ((i * 2 + j) % 11 < 2 ? 0xFF4A5C94 : (i < 0 ? 0xFF8FA6D6 : 0xFF6F86BC)));
    }
    fill(238, 22, 1, 12, 0xFFE0E0E0);
    for (int j = 0; j < 6; j++) fill(239, 22 + j, 7 - j / 2 + (frame / 8) % 2, 1, 0xFFD8463C);
    static const short P[][2] = { {56, 120}, {70, 152}, {118, 108}, {128, 140}, {196, 128}, {150, 70}, {40, 126}, {104, 150} };
    for (unsigned t = 0; t < sizeof P / sizeof P[0]; t++) pine(P[t][0], P[t][1], green);
    boat(310, 112 + (frame / 30) % 2);
    // lift station back down
    fill(12, 128, 2, 24, 0xFF575A66); fill(6, 128, 14, 2, 0xFF575A66);
    fill(2 + (frame / 8) % 6, 132, 6, 5, 0xFFD8463C);
  }
  // falling snow
  for (int i = 0; i < 50; i++) {
    uint32_t h = hash2(i, 77);
    int x = (int)(h % SCR_W + frame * (0.2f + (i % 3) * 0.1f)) % SCR_W, y = (int)((h >> 9) % SCR_H + frame * (0.4f + (i % 4) * 0.15f)) % SCR_H;
    put(x, y, 0xFFFFFFFF);
  }
}
// The two newest worlds: the land follows the path, with trees scattered on it and one landmark.
static uint32_t map_px(int x, int y) { return (unsigned)x < SCR_W && (unsigned)y < SCR_H ? FB[y * 2 * FB_W + x * 2] : 0; }
static void map_land(int kind) {   // 0 jungle, 1 snow, 2 desert, 3 clouds
  int snowy = kind == 1;
  const World* w = &WORLDS[world];
  uint32_t rim = snowy ? 0xFFBFE0F5 : 0xFFF0D9A0, land = snowy ? 0xFFF4FAFF : 0xFF4FAE52, shade = snowy ? 0xFFDCEBF7 : 0xFF409A46;
  if (kind == 2) { rim = 0xFFFFF0C0; land = 0xFFEBCB82; shade = 0xFFD9B46A; }
  if (kind == 3) { rim = 0xFFD6EAFB; land = 0xFFFFFFFF; shade = 0xFFEAF2FC; }
  for (int pass = 0; pass < 2; pass++) {
    for (int n = 0; n < w->n; n++) {
      disc(w->xy[n][0], w->xy[n][1], 28 + 5 * !pass, pass ? land : rim);
      if (n + 1 < w->n) disc((w->xy[n][0] + w->xy[n + 1][0]) / 2, (w->xy[n][1] + w->xy[n + 1][1]) / 2 + 6, 30 + 5 * !pass, pass ? land : rim);
    }
    for (int k = 0; k < 4; k++) { uint32_t h = hash2(k, 31 + world); disc(50 + (int)(h % 220), 50 + (int)((h >> 9) % 90), 20 + 5 * !pass, pass ? land : rim); }
  }
  for (int y = 0; y < SCR_H; y++) for (int x = 0; x < SCR_W; x++)
    if (map_px(x, y) == land && hash2(x, y + world * 50) % 10 == 0) put(x, y, shade);
  // the landmark sits behind the last level of the world
  int lx = w->xy[snowy ? 3 : 3][0], ly = w->xy[3][1];
  if (kind == 2) {   // a pyramid
    lx += 4; ly -= 44;
    for (int j = 0; j < 34; j++) for (int i = -j; i <= j; i++) put(lx + i, ly + j, i < -j / 3 ? 0xFFF2D48A : ((j % 6) == 5 ? 0xFFB08A48 : 0xFFD0A860));
  } else if (kind == 1 || kind == 3) {   // a castle (of ice, or up in the clouds)
    lx += 26; ly -= 30;
    fill(lx - 12, ly, 24, 22, 0xFFA9DAF5); fill(lx - 16, ly - 10, 8, 32, 0xFF8FC4EC); fill(lx + 8, ly - 10, 8, 32, 0xFF8FC4EC);
    for (int j = 0; j < 8; j++) { fill(lx - 12 - j / 2 - 0, ly - 18 + j, 1 + j, 1, 0xFFEAF8FF); fill(lx + 12 - j / 2, ly - 18 + j, 1 + j, 1, 0xFFEAF8FF); }
    fill(lx - 3, ly + 12, 6, 10, 0xFF24346E); fill(lx - 12, ly, 24, 2, 0xFFEAF8FF);
    fill(lx, ly - 12, 1, 12, 0xFFE0E0E0); for (int j = 0; j < 5; j++) fill(lx + 1, ly - 12 + j, 6 - j + (frame / 8) % 2, 1, 0xFFD8463C);
  } else {       // a volcano with a little smoke
    lx += 2; ly -= 46;
    for (int j = 0; j < 36; j++) for (int i = -j * 3 / 4 - 3; i <= j * 3 / 4 + 3; i++)
      put(lx + i, ly + j, j < 5 ? ((i + frame / 10) % 3 ? 0xFFFF8A2C : 0xFFFFD23C) : ((i * 2 + j) % 9 < 2 ? 0xFF5A3A2E : (i < 0 ? 0xFF8A5A46 : 0xFF74483A)));
    for (int k = 0; k < 3; k++) disc(lx + k * 5 - 2, ly - 6 - k * 6 - (frame / 20) % 3, 4 + k, 0xFFE8E0E0);
  }
  uint32_t green = 0xFF2E7D5A;
  for (int t = 0; t < 40; t++) {
    uint32_t h = hash2(t, 61 + world);
    int x = 14 + (int)(h % 292), y = 34 + (int)((h >> 10) % 124), ok = 1;
    for (int dy = -8; dy <= 4 && ok; dy += 4) for (int dx = -6; dx <= 6; dx += 6) { uint32_t c = map_px(x + dx, y + dy); if (c != land && c != shade) ok = 0; }
    for (int n = 0; n < w->n && ok; n++) {
      int ax = w->xy[n][0], ay = w->xy[n][1];
      if (abs(x - ax) < 14 && abs(y - ay) < 16) ok = 0;
      if (n + 1 < w->n) for (int q = 1; q < 6; q++) if (abs(x - (ax + (w->xy[n + 1][0] - ax) * q / 6)) < 9 && abs(y - 3 - (ay + (w->xy[n + 1][1] - ay) * q / 6)) < 10) ok = 0;
    }
    if (!ok) continue;
    if (kind == 3) { disc(x - 4, y, 4, 0xFFEAF2FC); disc(x + 3, y - 1, 5, 0xFFEAF2FC); }
    else if (kind == 2) { fill(x - 1, y - 6, 3, 11, 0xFF4F9A52); fill(x - 5, y - 3, 4, 2, 0xFF4F9A52); fill(x - 5, y - 6, 2, 4, 0xFF4F9A52); fill(x + 2, y - 1, 4, 2, 0xFF4F9A52); fill(x + 4, y - 5, 2, 5, 0xFF4F9A52); }
    else if (snowy) pine(x, y, green);
    else { fill(x - 1, y, 3, 6, 0xFF6B3F1D); disc(x, y - 3, 6, t % 3 ? 0xFF2E7D32 : 0xFF3F9A3A); disc(x - 2, y - 5, 2, 0xFF5FC25A); }
  }
  boat(w->xy[0][0] - 16, w->xy[0][1] + 8 + (frame / 30) % 2);
  if (!snowy && w->kind[w->n - 1] == -3) boat(w->xy[w->n - 1][0] + 16, w->xy[w->n - 1][1] + 6 + (frame / 30) % 2);
  if (snowy) for (int i = 0; i < 50; i++) {
    uint32_t h = hash2(i, 77);
    int x = (int)(h % SCR_W + frame * (0.2f + (i % 3) * 0.1f)) % SCR_W, y = (int)((h >> 9) % SCR_H + frame * (0.4f + (i % 4) * 0.15f)) % SCR_H;
    put(x, y, 0xFFFFFFFF);
  }
}
static void draw_map(void) {
  char buf[64];
  const World* w = &WORLDS[world];
  static uint32_t* map_keep; static int keep_world = -1, keep_n;
  if (!map_keep) map_keep = (uint32_t*)malloc(FB_W * FB_H * 4);
  if (keep_world == world && (keep_n++ & 3)) { memcpy(FB, map_keep, FB_W * FB_H * 4); goto dino; }
  keep_world = world;
  // sea
  uint32_t sea0 = 0xFF2F7FC1, sea1 = 0xFF5BB3E6, wave = 0xFF9ED6F5;
  if (world == 6) { sea0 = 0xFF4A90E2; sea1 = 0xFFBFE3FF; wave = 0xFFFFFFFF; }
  if (world == 1 || world == 4) { sea0 = 0xFF3F7FB4; sea1 = 0xFF86BFE2; wave = 0xFFD6ECFA; }
  if (world == 2) { sea0 = 0xFF0B1030; sea1 = 0xFF24346E; wave = 0xFF4A5C94; }
  for (int y = 0; y < SCR_H; y++) {
    fill(0, y, SCR_W, 1, blend(sea0, sea1, y / (float)SCR_H));
  }
  for (int i = 0; i < 50; i++) {
    uint32_t h = hash2(i, 5);
    int x = (int)(h % SCR_W + frame / 6) % SCR_W, y = (h >> 9) % SCR_H;
    if ((frame / 15 + i) % 4) fill(x, y, 3, 1, wave);
  }
  if (world == 0) map_jungle(); else if (world >= 3) map_land(world == 4 ? 1 : world == 5 ? 2 : world == 6 ? 3 : 0); else map_snow(world == 2);
  // path between the stops
  for (int n = 0; n + 1 < w->n; n++) {
    int open = node_open(n + 1);
    float x0 = w->xy[n][0], y0 = w->xy[n][1], x1 = w->xy[n + 1][0], y1 = w->xy[n + 1][1];
    float len = sqrtf((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
    for (float d = 6; d < len - 5; d += 6) {
      int x = (int)(x0 + (x1 - x0) * d / len), y = (int)(y0 + (y1 - y0) * d / len);
      fill(x - 1, y - 1, 3, 3, open ? (SNOW_WORLD ? 0xFFE08A2C : 0xFFFFF4C2) : (SNOW_WORLD ? 0xFF9FB4CC : 0xFF3E7D3B));
    }
  }
  for (int n = 0; n < w->n; n++) {
    int x = w->xy[n][0], y = w->xy[n][1], kind = w->kind[n];
    if (kind == -1) continue;
    if (kind < 0) {
      // an arrow marker for the way to another world
      uint32_t c = node_open(n) ? 0xFF58D6E8 : 0xFF8C8F9C;
      disc(x, y, 6, 0xFF2B1A12); disc(x, y, 5, c);
      int dir = kind == -3 ? 1 : -1;
      for (int j = -2; j <= 2; j++) fill(x - dir * 2 + (dir > 0 ? 0 : -(2 - abs(j))), y + j, 3 - abs(j) + 1, 1, 0xFF2B1A12);
      continue;
    }
    uint32_t c = cleared[kind + 1] ? 0xFFF7D038 : (node_open(n) ? 0xFFD8463C : 0xFF8C8F9C);
    disc(x, y, 6, 0xFF2B1A12); disc(x, y, 5, c); disc(x - 1, y - 2, 2, blend(c, 0xFFFFFFFF, 0.5f));
    snprintf(buf, sizeof buf, "%d", kind + 1);
    text_raw(buf, x - (kind + 1 >= 10 ? 3 : 1), y - 2, 1, 0xFF2B1A12);
  }
  memcpy(map_keep, FB, FB_W * FB_H * 4);
dino:;
  // Dino walking between stops
  float t = map_t >= 24 ? 1 : map_t / 24.0f;
  float dx = w->xy[map_from][0] + (w->xy[map_node][0] - w->xy[map_from][0]) * t;
  float dy = w->xy[map_from][1] + (w->xy[map_node][1] - w->xy[map_from][1]) * t;
  int moving = map_t < 24;
  {
    DinoPose p; dino_idle_pose(&p, frame);
    if (moving) for (int i = 0; i < 2; i++) { float a = map_t * 0.55f + i * 3.14159f; p.lx[i] = sinf(a) * 7.5f + (i ? -2.8f : 2.8f); p.ly[i] = maxf(0, cosf(a)) * 5.5f; p.tilt = 0.12f; p.bob = fabsf(cosf(map_t * 0.55f)) * 1.5f; }
    int X = (int)(dx * 2), Y = (int)(dy * 2) - 2;
    fb_shadow(X, Y + 1, 12, 2.8f, 0.22f);
    draw_dino_at(X, Y, 1, &p, moving ? (w->xy[map_node][0] < w->xy[map_from][0] ? -0.8f : 0.8f) : 0.3f);
  }
  // labels
  fill(0, 0, SCR_W, 16, 0xFF1B1020);
  int kind = w->kind[map_node];
  if (kind == -1) text_c("DINO'S HOUSE", 3, 2, 0xFFFFFFFF);
  else if (kind == -3 || kind == -2) {
    snprintf(buf, sizeof buf, "TO %s", WORLDS[world + (kind == -3 ? 1 : -1)].name);
    text(buf, 4, 3, 2, 0xFFFFFFFF);
    text("A: GO", SCR_W - text_w("A: GO", 1) - 4, 6, 1, 0xFF58D6E8);
  } else {
    text(LEVELS[kind].name, 4, 3, 2, 0xFFFFFFFF);
    const char* hint = cleared[kind + 1] ? "CLEARED!  A: PLAY" : "A: PLAY";
    text(hint, SCR_W - text_w(hint, 1) - 4, 6, 1, cleared[kind + 1] ? 0xFFF7D038 : 0xFFFFF4C2);
  }
  int nc = 0; for (int i = 1; i <= NLEVELS; i++) nc += cleared[i];
  fill(0, SCR_H - 11, SCR_W, 11, 0xFF1B1020);
  snprintf(buf, sizeof buf, "%s   CLEARED %d/%d   COINS %d   LIVES %d", w->name, nc, NLEVELS, total_bananas, lives);
  text(buf, 4, SCR_H - 8, 1, 0xFFFFF4C2);
}

void game_draw(uint32_t* fb) {
  FB = fb;
  char buf[64];
  if (state == ST_TITLE) {
    static uint32_t* title_bg;   // the sky, trees and hill never change, so they are painted once and kept
    if (!title_bg) {
      title_bg = (uint32_t*)malloc(FB_W * FB_H * 4);
      hd_background();
      // a round grassy hill for Dino to stand on
      for (int y = 280; y < FB_H; y++) for (int x = 0; x < FB_W; x++) {
        float dd = sd_ellipse(x + 0.5f, y + 0.5f, 320, 700, 620, 410);
        if (dd > 1) continue;
        uint32_t c = dd > -12 ? mixc(0xFFC8F27A, 0xFF6CC943, clampf(-dd / 12, 0, 1)) : mixc(0xFF6CC943, 0xFF3B9A36, clampf((-dd - 12) / 60, 0, 1));
        fb_over(x, y, c, cov(dd));
      }
      memcpy(title_bg, FB, FB_W * FB_H * 4);
    } else memcpy(FB, title_bg, FB_W * FB_H * 4);
    {
      DinoPose p; dino_idle_pose(&p, state_t);
      float hop = fabsf(sinf(state_t * 0.05f));
      p.bob = hop * 5; p.sq = 1 + 0.06f * sinf(state_t * 0.1f); p.spk = sinf(state_t * 0.1f + 1) * 0.15f; p.mouth = hop > 0.5f ? (hop - 0.5f) * 2 : 0;
      fb_shadow(320, 293, 44 - hop * 6, 6, 0.2f);
      draw_dino_at(320, 292, 3, &p, 0.35f + sinf(state_t * 0.025f) * 0.45f);
    }
    hblit(&HD_COIN[(frame / 2) % COIN_FRAMES], 80, 250 + (int)(sinf(frame * 0.08f) * 4), 0);
    hblit(&HD_COIN[(frame / 2 + 8) % COIN_FRAMES], 528, 250 + (int)(sinf(frame * 0.08f + 2) * 4), 0);
    text_c("AMAZING DINO", 24, 5, 0xFFF7D038);
    text_c("A LITTLE JUNGLE ADVENTURE", 58, 1, 0xFF3B2414);
#ifdef __EMSCRIPTEN__
    if ((state_t / 30) % 2 == 0) text_c("PRESS JUMP TO START", 80, 2, 0xFFFFFFFF);
#else
    if ((state_t / 30) % 2 == 0) text_c("PRESS A TO START", 80, 2, 0xFFFFFFFF);
    if (!g_no_hud) text_c("MOVE: STICK   JUMP: A OR B   HEADBUTT: X OR Y   PAUSE: +", 167, 1, 0xFFFFF4C2);
#endif
    return;
  }
  if (state == ST_MAP) { draw_map(); return; }
  if (state == ST_PLAY && intro_t) {
    // the level intro: Dino on black with his coins and lives above him, then everything flies into place
    float k = clampf((intro_t - INTRO_HOLD) / (float)INTRO_FLY, 0, 1), e = k * k * (3 - 2 * k);
    if (k <= 0) memset(FB, 0, FB_W * FB_H * 4);
    else { hd_background(); draw_world(); darken(0, 0, SCR_W, SCR_H, 1 - e); }
    if (k <= 0) { for (int i = 0; i < FB_W * FB_H; i++) FB[i] = 0xFF000000; text_c(L->name, 14, 3, 0xFFFFFFFF); dino_sx = 320; dino_sy = 250; }
    else if (k < 0.5f) text_c(L->name, 14 - (int)(e * 80), 3, 0xFFFFFFFF);
    hud_cx = (int)(100 * (1 - e)); hud_lx = (int)(-96 * (1 - e)); hud_cy = hud_ly = (int)(38 * (1 - e));
    draw_hud();
    hud_cx = hud_cy = hud_lx = hud_ly = 0;
    DinoPose p; dino_idle_pose(&p, intro_t);
    float hop = fabsf(sinf(intro_t * 0.09f)) * (1 - e);
    p.bob = hop * 5; p.mouth = hop > 0.5f ? (hop - 0.5f) * 2 : 0; p.mood = k <= 0 ? 4 : 0; p.spk = sinf(intro_t * 0.15f) * 0.15f * (1 - e);
    if (k > 0) { p.tilt = 0.25f * sinf(k * 3.14159f); p.air = k < 0.9f; }
    float S = 2.6f + (1 - 2.6f) * e, X = 320 + (dino_sx - 320) * e, Y = 262 + (dino_sy - 262) * e - sinf(k * 3.14159f) * 40;
    draw_dino_at((int)X, (int)Y, S, &p, 0.35f + 0.45f * e);
    return;
  }
  if (state == ST_OVER && state_t >= 40) {
    // game over: Dino drops in from the top, bumps down onto his bottom and sits there looking sad
    for (int i = 0; i < FB_W * FB_H; i++) FB[i] = 0xFF000000;
    int t = state_t - 46; float y, sq = 1; int sat = 0;
    if (t < 36) y = -90 + 0.27f * t * t;                                   // falling
    else if (t < 56) { y = 260 - 34 * sinf((t - 36) * 3.14159f / 20); sq = 1 - 0.22f * expf(-(t - 36) / 5.0f); }   // one bounce
    else { y = 260; sq = 1 - 0.2f * expf(-(t - 56) / 7.0f); sat = 1; }
    if (t >= 0) {
      DinoPose p; dino_idle_pose(&p, state_t);
      p.sq = sq; p.mood = sat ? 5 : 2; p.mouth = 0; p.air = !sat;
      if (!sat) { p.tilt = 0.5f; for (int i = 0; i < 2; i++) { p.lx[i] = sinf(state_t * 0.6f + i * 3.14159f) * 5; p.ly[i] = 3; } }
      else { p.tilt = -0.12f; p.bob = -4; p.spk = -0.3f; p.eye = 0.6f; for (int i = 0; i < 2; i++) { p.lx[i] = 12 + i * 4; p.ly[i] = 2.5f; } }   // legs out in front
      if (t >= 36) fb_shadow(320, 262, 40, 6, 0.0f);
      draw_dino_at(320, (int)y, 2.6f, &p, 0.3f);
    }
    if (state_t > 105) text_c("GAME OVER", 22 - (state_t < 117 ? (117 - state_t) * 4 : 0), 4, 0xFFD8463C);
    if (state_t > 125) {
      static const char* const BT[2] = { "CONTINUE?", "BACK TO TITLE" };
      static const int BX[2] = { 34, 150 }, BW[2] = { 96, 136 };
      for (int b = 0; b < 2; b++) {
        int on = over_sel == b, by = 142 - (on ? (frame / 12) % 2 : 0);
        fill(BX[b] - 2, by - 2, BW[b] + 4, 26, on ? 0xFFF7D038 : 0xFF4A4458);
        fill(BX[b], by, BW[b], 22, on ? 0xFF3B2F6B : 0xFF1B1020);
        text(BT[b], BX[b] + (BW[b] - text_w(BT[b], 2)) / 2, by + 6, 2, on ? 0xFFFFFFFF : 0xFF9A94A8);
      }
      text_c("LEFT / RIGHT TO CHOOSE, A TO PICK", 171, 1, 0xFFFFF4C2);
    }
    return;
  }
  hd_background();
  draw_world();
  if (L->theme == 5 || L->theme == 7) {
    for (int i = 0; i < 90; i++) {   // soft snowflakes drifting down
      uint32_t h = hash2(i, 31);
      float fx = fmodf(h % FB_W + frame * (0.5f + (i % 3) * 0.24f) + sinf(frame * 0.03f + i) * 6 - camx * 1.2f, FB_W), fy = fmodf((h >> 9) % FB_H + frame * (1.0f + (i % 4) * 0.4f) - camy * 1.2f, FB_H);
      if (fx < 0) fx += FB_W;
      if (fy < 0) fy += FB_H;
      hblit(&FLAKE[i % 3], (int)fx - 4, (int)fy - 4, 0);
    }
  }
  if (!g_no_hud) draw_hud();
  if (state == ST_PAUSE) {
    darken(0, 0, SCR_W, SCR_H, 0.5f);
    text_c("PAUSED", 60, 4, 0xFFFFFFFF);
    text_c("+ TO KEEP PLAYING", 95, 1, 0xFFFFF4C2);
    text_c("DOWN TO GO BACK TO THE MAP", 105, 1, 0xFFFFF4C2);
  }
  if (state == ST_CLEAR) {
    text_c("LEVEL CLEAR!", 50, 4, 0xFFF7D038);
    snprintf(buf, sizeof buf, "TIME %d.%d", lvl_frames / 60, (lvl_frames % 60) / 6);
    text_c(buf, 82, 2, 0xFFFFFFFF);
  }
  if (state == ST_OVER) {
    darken(0, 0, SCR_W, SCR_H, state_t / 40.0f);   // the level fades to black
  }
  if (state == ST_WIN) {
    darken(0, 0, SCR_W, SCR_H, 0.55f);
    text_c("YOU WIN!", 36, 5, 0xFFF7D038);
    { DinoPose p; dino_idle_pose(&p, state_t); p.mood = 4; p.mouth = 1; p.bob = 6 * fabsf(sinf(state_t * 0.12f)); p.spk = sinf(state_t * 0.24f) * 0.2f; draw_dino_at(FB_W / 2, 246, 2.6f, &p, sinf(state_t * 0.05f) * 0.5f); }
    snprintf(buf, sizeof buf, "COINS: %d", total_bananas);
    text_c(buf, 128, 2, 0xFFFFFFFF);
    if (state_t > 60) text_c("PRESS A", 150, 2, 0xFFFFF4C2);
  }
}

// ============================================================ sound
typedef struct { float f0, f1, t, dur, vol, phase; int wave; uint32_t lfsr; } Voice;
static Voice voices[8];

void synth_trigger(int id) {
  Voice v = { 0 };
  v.wave = 0; v.vol = 0.18f; v.lfsr = 1;
  switch (id) {
    case SFX_JUMP: v.f0 = 330; v.f1 = 720; v.dur = 0.12f; break;
    case SFX_WALL: v.f0 = 420; v.f1 = 820; v.dur = 0.10f; break;
    case SFX_BANANA: v.f0 = 1320; v.f1 = 1760; v.dur = 0.09f; v.vol = 0.12f; break;
    case SFX_STOMP: v.f0 = 500; v.f1 = 90; v.dur = 0.15f; v.wave = 2; v.vol = 0.22f; break;
    case SFX_HURT: v.f0 = 600; v.f1 = 80; v.dur = 0.6f; v.wave = 1; v.vol = 0.2f; break;
    case SFX_SPRING: v.f0 = 200; v.f1 = 1200; v.dur = 0.3f; v.wave = 1; break;
    case SFX_CHECK: v.f0 = 660; v.f1 = 1320; v.dur = 0.25f; v.wave = 3; v.vol = 0.15f; break;
    case SFX_CLEAR: v.f0 = 523; v.f1 = 1046; v.dur = 1.2f; v.wave = 3; v.vol = 0.18f; break;
    case SFX_LIFE: v.f0 = 784; v.f1 = 1568; v.dur = 0.5f; v.wave = 3; v.vol = 0.16f; break;
    case SFX_SELECT: v.f0 = 880; v.f1 = 880; v.dur = 0.08f; v.vol = 0.12f; break;
    case SFX_DASH: v.f0 = 900; v.f1 = 200; v.dur = 0.18f; v.wave = 2; v.vol = 0.16f; break;
    case SFX_SWORD: v.f0 = 2400; v.f1 = 600; v.dur = 0.14f; v.wave = 2; v.vol = 0.14f; break;
    case SFX_JUMP2: v.f0 = 500; v.f1 = 1100; v.dur = 0.14f; v.wave = 1; v.vol = 0.2f; break;
    default: return;
  }
  for (int i = 0; i < 8; i++) if (voices[i].t >= voices[i].dur) { voices[i] = v; return; }
  voices[0] = v;
}

// ---- music. Every tune here is our own: a melody line, a chord for each bar, and the band
// (bouncy bass, off-beat chord "pah"s and a little drum kit) is worked out from the chords.
// Melody: one character per eighth note. '.' = rest, '-' = hold, otherwise semitones above the
// tune's base note (0-9, then A=10, B=11, C=12 ...). Chords: two characters per bar, the root
// (same code) and M for major or m for minor.
// Each tune also has its own band: patterns (one character per eighth note, repeating) say when
// the bass, chord stabs, drums and the "bah" singers come in, so no two tunes share a backing.
//   bass: 0 root, 7 fifth, C octave, 3 third, A seventh.   snare: x snare and clap, c clap only.
//   bah: b the singers sing the chord, l one low singer.   arp: 1 marimba runs all the time, 2 on the beat only.
typedef struct {
  int base, bpm, lead, spb; float swing;   // spb: eighth notes in a bar (8, or 6 for a waltz)
  const char *mel, *chords, *bass, *stab, *kick, *snare, *hat, *bah;
  int arp, bell, harm, brass;
} Song;
enum { SONG_TITLE, SONG_MAP, SONG_GRASS, SONG_WILD, SONG_CAVE, SONG_FALLS, SONG_SNOW, SONG_ICE, SONG_NIGHT, SONG_DESERT, SONG_CASTLE, SONG_BOSS, NSONGS };
static const Song SONGS[NSONGS] = {
  { 62, 168, 4, 8, 0.60f,   // Hello Dino (title): a little parade
    "7.77C.G." "E.C.9.7-" "9.99C.9." "7--.4.2." "4.57C.G." "H.G.E.C-" "E.EGE.B." "C--....."
    "H.HGH.L." "J.G.E.G-" "H.HGH.L." "N--.L.J." "O.L.J.G." "H.E.C.9." "B.EGJ.N." "O--.....",
    "0M5M5M7M0M9m7M0M5M0M5M7M0M9m7M0M",
    "0.0.7.7.", "..x...x.", "x...x...", "..x...xx", "x.x.x.x.",
    "........" "........" "........" "....b.b." "........" "........" "........" "..b.b.b.", 2, 1, 1, 1 },
  { 67, 112, 2, 8, 0.66f,   // Which Way? (map): a lazy stroll with the singers going bah on the off-beats
    "4--57--4" "0--24---" "5--79--7" "7--54-2-" "4--57--C" "E--C9---" "5-7-9-B-" "C-----..",
    "0M9m5M7M0M9m5M0M",
    "0...7...", "", "x.......", "....c...", "x.x.x.x.", "..b...b.", 0, 0, 1, 0 },
  { 60, 160, 0, 8, 0.62f,   // Jungle Hop (grassy levels)
    "C.CG.EC." "9.C.7-.." "5.59.C.H" "G.E.B..." "C.CG.EC." "9.C.E-.." "H.G.E.B." "C--....."
    "9.CE.C9." "7.BE.B7." "5.9C.95." "7.B.E.G." "L.J.G.E." "H.E.C.9." "7.B.E.J." "O--.C...",
    "0M0M5M7M0M9m7M0M9m4m5M7M9m5M7M0M",
    "0.7.0.7.", ".x.x.x.x", "x...x...", "..x...x.", "x.x.x.x.",
    "........" "........" "........" "....b.b." "........" "........" "........" "....b.b.", 0, 1, 1, 0 },
  { 64, 150, 3, 8, 0.5f,    // Bongo Party (the Wild Jungle): steel drum and a three-three-two beat
    "7..9..C." "E..C..9." "9..C..H." "G.E.B.7." "7..9..C." "H..L..H." "J.G.E.B." "C--....."
    "H.HH.L.H" "G.GG.J.G" "E.EE.H.E" "E.B.7.B." "H.HH.L.O" "N.L.J.G." "H.E.B.E." "C--.C.C.",
    "0M0M5M7M0M5M7M0M5M0M2m7M5M0M7M0M",
    "0..0..7.", "...x..x.", "x..x..x.", "....c...", "xxxxxxxx", "........" "b..b..b.", 0, 0, 1, 0 },
  { 57, 104, 1, 8, 0.5f,    // Sneaky Steps (temples and caves): tiptoeing, nearly no drums
    "0.3.7.3." "8-7-...." "5.8.C.8." "B-7-...." "C.F.J.F." "K-J-H-F-" "E.B.7.B." "C--.....",
    "0m0m5m7M0m8M7M0m",
    "0...7...", "", "", "......c.", "..x...x.", "........" "....l.l.", 0, 0, 0, 0 },
  { 65, 150, 0, 6, 0.5f,    // Up in the Clouds (waterfalls and sky): a waltz
    "7-C-G-" "E---C-" "9-C-H-" "E---B-" "C-E-G-" "H---L-" "J-H-E-" "C-----"
    "L-J-H-" "G---C-" "H-E-9-" "B---7-" "9-C-H-" "G-J-O-" "N-J-E-" "C-----",
    "0M0M5M7M9m5M7M0M5M0M2m7M5M0M7M0M",
    "0.....", "..x.x.", "x.....", "", "..x.x.",
    "......" "......" "......" "......" "......" "......" "......" "..b.b.", 2, 1, 1, 0 },
  { 62, 132, 2, 8, 0.67f,   // Frosty Skip (snow): skipping along with sleigh bells
    "G.EC.E.G" "L.HC.H.L" "J.GE.G.J" "G--.C..." "L.GC.G.L" "O.LH.L.O" "N.JE.J.N" "O--....."
    "H.L.O.L." "G.J.O.J." "E.H.L.H." "J--.E.J." "L.O.L.H." "J.G.E.C." "E.G.J.N." "O--.C...",
    "0M5M7M0M9m5M7M0M5M0M2m7M5M0M7M0M",
    "0.7.0.7.", "", "x...x...", "..c...c.", "xxxxxxxx", "........" "......bb", 2, 0, 1, 0 },
  { 64, 96, 2, 8, 0.5f,     // Crystal Echo (ice caves): slow and sparkly
    "J---F-C-" "K---F---" "M---J-F-" "N---J---" "O-M-K-J-" "K-H-F-H-" "E-J-N-J-" "C-------",
    "0m8M3M7M0m5m7M0m",
    "0.......", "", "", "", "....x...", "b......." "........", 1, 1, 0, 0 },
  { 58, 120, 0, 8, 0.58f,   // Northern Lights (snow at night): a gentle song
    "7---C-G-" "J---G-E-" "H---L-O-" "N-L-J---" "L---O-L-" "H---C-9-" "B-E-J-N-" "O-----..",
    "0M4m5M7M9m5M7M0M",
    "0...7.0.", "..x...x.", "x...x...", "....c...", "x.x.x.x.", "....b..." "....b.b.", 0, 1, 1, 0 },
  { 62, 126, 1, 8, 0.5f,    // Dune Dance (the desert)
    "0.14.5.4" "1-0-...." "4.57.8.7" "5-8-...." "7.8A.C.A" "8.7.5.4." "5.8.5.1." "0--.....",
    "0M1M0M5m0M5m1M0M",
    "0..0..7.", "..x...x.", "x..x..x.", "", "x.xxx.xx", "", 0, 0, 0, 0 },
  { 57, 138, 1, 8, 0.5f,    // Castle in the Clouds
    "C.E.F.J." "N.J.E.B." "C.F.J.O." "M.H.E.A." "K.O.K.F." "H.K.H.C." "E.J.N.J." "O--.C...",
    "0m7M0mAM8M5m7M0m",
    "0.7.3.7.", "", "", "", "..x...x.", "", 2, 0, 0, 0 },
  { 62, 184, 4, 8, 0.5f,    // Big Trouble (bosses)
    "0.03.5.7" "8.7.5.3." "8.8C.F.C" "E.B.7-.." "C.CF.H.J" "K.J.H.F." "E.EJ.N.J" "O--.C.C.",
    "0m0m8M7M0m5m7M0m",
    "0.0C0.0C", "x..x..x.", "x.x.x.x.", "..x...x.", "xxxxxxxx", "b..b...." "........", 0, 0, 0, 1 },
};
static int note_code(char c) { return c >= '0' && c <= '9' ? c - '0' : c - 'A' + 10; }
static float midi_hz(int n) { return 440.0f * powf(2.0f, (n - 69) / 12.0f); }
static int song_now = -1, mus_step;
static float mus_t, mus_len;
static struct { float hz, ph, ph2, t; int on; } m_lead;
// the band: besides the lead there is a pool of short notes, each played by one "instrument"
enum { I_BASS, I_STAB, I_MARIMBA, I_BRASS, I_BELL, I_KICK, I_SNARE, I_HAT, I_CLAP, I_BAH, NINS };
typedef struct { float hz, ph, t, env, env2; int ins; } MNote;
static float SINT[1024], M_DEC[NINS], M_DEC2[NINS]; static int m_rate;
#define SIN1(ph) SINT[(int)((ph) * 1024) & 1023]   /* sine of a phase 0..1, from a table */
#define NPOOL 36
static MNote m_pool[NPOOL];
static struct { float hz, ph, t; int on; } m_harm;
static uint32_t m_noise = 12345;

static void m_play(int ins, float hz) {
  int best = 0;
  for (int i = 0; i < NPOOL; i++) { if (m_pool[i].ins < 0) { best = i; break; } if (m_pool[i].t > m_pool[best].t) best = i; }
  m_pool[best].ins = ins; m_pool[best].hz = hz; m_pool[best].ph = 0; m_pool[best].t = 0; m_pool[best].env = m_pool[best].env2 = 1;
}
static int pick_song(void) {
  if (state == ST_TITLE) return SONG_TITLE;
  if (state == ST_MAP) return SONG_MAP;
  if (state == ST_PLAY || state == ST_PAUSE) {
    if (g_boss_music) return SONG_BOSS;
    if (lvl >= 21) return SONG_CASTLE;   // the whole Cloud Castle world
    switch (L->theme) {
      case 1: case 2: return SONG_CAVE;
      case 8: return SONG_DESERT;
      case 3: return SONG_FALLS;
      case 4: return lvl >= 12 ? SONG_WILD : SONG_FALLS;
      case 5: return SONG_SNOW;
      case 6: return SONG_ICE;
      case 7: return SONG_NIGHT;
      default: return lvl >= 12 ? SONG_WILD : SONG_GRASS;
    }
  }
  return -1;
}
static char m_pat(const char* p) { return p && *p ? p[mus_step % (int)strlen(p)] : '.'; }
// one eighth-note step: who plays what
static void music_step(const Song* sg, int half) {
  int nsteps = (int)strlen(sg->mel), bar = mus_step / sg->spb, st = mus_step % sg->spb;
  int root = sg->base - 24 + note_code(sg->chords[(bar * 2) % strlen(sg->chords)]);
  int minor = sg->chords[(bar * 2 + 1) % strlen(sg->chords)] == 'm';
  if (root > sg->base - 24 + 7) root -= 12;
  int tri[4] = { root + 24, root + 24 + (minor ? 3 : 4), root + 24 + 7, root + 36 };
  if (half) {   // the "and" between steps
    static const int UP[8] = { 1, 2, 3, 2, 1, 2, 3, 0 };
    if (sg->arp == 1) m_play(I_MARIMBA, midi_hz(tri[UP[st]] + 12));
    return;
  }
  char m = sg->mel[mus_step % nsteps];
  if (m == '.') { m_lead.on = 0; m_harm.on = 0; }
  else if (m != '-') {
    int note = sg->base + note_code(m);
    m_lead.hz = midi_hz(note); m_lead.t = 0; m_lead.on = 1;
    if (sg->bell) m_play(I_BELL, midi_hz(note + 12));         // a glockenspiel doubles the tune an octave up
    if (sg->harm) {   // a second voice sings along underneath, on the nearest chord note
      int h = note - 3;
      for (; h > note - 10; h--) { int pc = ((h - root) % 12 + 12) % 12; if (pc == 0 || pc == 7 || pc == (minor ? 3 : 4)) break; }
      m_harm.hz = midi_hz(h); m_harm.t = 0; m_harm.on = 1;
    }
  }
  char c = m_pat(sg->bass);
  if (c != '.') m_play(I_BASS, midi_hz(root + (c == '7' ? 7 : c == 'C' ? 12 : c == '3' ? (minor ? 3 : 4) : c == 'A' ? 10 : 0)));
  if (m_pat(sg->stab) == 'x') for (int i = 0; i < 3; i++) m_play(I_STAB, midi_hz(tri[i]));
  if (sg->brass && st == 0 && bar % 2 == 0) for (int i = 0; i < 3; i++) m_play(I_BRASS, midi_hz(tri[i] - (i == 2 ? 12 : 0)));
  if (sg->arp) { static const int ARP[8] = { 0, 1, 2, 3, 2, 1, 2, 3 }; m_play(I_MARIMBA, midi_hz(tri[ARP[st]] + 12)); }
  if (m_pat(sg->kick) == 'x') m_play(I_KICK, 0);
  c = m_pat(sg->snare);
  if (c == 'x') m_play(I_SNARE, 0);
  if (c == 'x' || c == 'c') m_play(I_CLAP, 0);
  if (m_pat(sg->hat) == 'x') m_play(I_HAT, 0);
  c = '.';   // the singers are switched off for now (the patterns are kept in case they come back)
  if (c == 'b') for (int i = 0; i < 3; i++) m_play(I_BAH, midi_hz(tri[i]) * (1 + (i - 1) * 0.004f));   // the singers: "bah!"
  if (c == 'l') m_play(I_BAH, midi_hz(root + 24));
}
// ---- recorded music. When the game is built with MUSIC_BIN_PATH, the songs are real-instrument
// recordings (made by tools/mkmidi.py from the same tunes), packed four bits a sample and played
// straight from memory; each one loops without a gap. Without it, the little synth band below plays.
#ifdef MUSIC_BIN_PATH
#ifdef MUSIC_PTR
const unsigned char* music_bin;   // the web build loads music.bin itself and points this at it
#else
__asm__(".pushsection .rodata\n.balign 4\n.global music_bin\nmusic_bin:\n.incbin \"" MUSIC_BIN_PATH "\"\n.popsection\n");
extern const unsigned char music_bin[];
#endif
#include "music_tab.h"
#define MUS_RATE 32000
static struct { int song, pos, pred, idx, s0, s1; float frac, gain; } MS = { -1 };
static int ms_next(void) {
  static const short STEP[89] = { 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 };
  static const signed char IDX[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
  int code = (music_bin[MUS_TAB[MS.song].off + (MS.pos >> 1)] >> ((MS.pos & 1) ? 4 : 0)) & 15, step = STEP[MS.idx];
  int dq = step >> 3; if (code & 4) dq += step; if (code & 2) dq += step >> 1; if (code & 1) dq += step >> 2;
  MS.pred += code & 8 ? -dq : dq; if (MS.pred > 32767) MS.pred = 32767; if (MS.pred < -32768) MS.pred = -32768;
  MS.idx += IDX[code & 7]; if (MS.idx < 0) MS.idx = 0; if (MS.idx > 88) MS.idx = 88;
  if (++MS.pos >= MUS_TAB[MS.song].n) { MS.pos = 0; MS.pred = MUS_TAB[MS.song].first; MS.idx = 0; }   // round again
  return MS.pred;
}
static float stream_sample(int want, float dt) {
  if (want != MS.song) { MS.song = want; MS.pos = 0; MS.idx = 0; MS.frac = 0; MS.gain = 0; if (want >= 0) MS.pred = MS.s0 = MS.s1 = MUS_TAB[want].first; }
  if (want < 0) return 0;
  MS.frac += MUS_RATE * dt;
  while (MS.frac >= 1) { MS.frac -= 1; MS.s0 = MS.s1; MS.s1 = ms_next(); }
  if (MS.gain < 1) MS.gain += dt * 8;
  return (MS.s0 + (MS.s1 - MS.s0) * MS.frac) * (1.0f / 32768) * 0.62f * MS.gain * (state == ST_PAUSE ? 0.35f : 1);
}
#endif
static float music_sample(float dt) {
  static int half_done;
  int want = pick_song();
#ifdef MUSIC_BIN_PATH
#ifdef MUSIC_PTR
  if (music_bin)   // no recorded music loaded: play the built-in band instead
#endif
  return stream_sample(want, dt);
#endif
  if (want != song_now) {
    song_now = want; mus_step = -1; mus_t = mus_len = 0; m_lead.on = 0; m_harm.on = 0; half_done = 1;
    for (int i = 0; i < NPOOL; i++) m_pool[i].ins = -1;
  }
  if (want < 0) return 0;
  const Song* sg = &SONGS[want];
  mus_t += dt;
  if (mus_t >= mus_len) {
    mus_t -= mus_len;
    mus_step = (mus_step + 1) % (int)strlen(sg->mel);
    float beat = 60.0f / sg->bpm;                       // two eighth notes, the first one longer (swing)
    mus_len = beat * (mus_step % sg->spb % 2 == 0 ? sg->swing : 1 - sg->swing);
    music_step(sg, 0); half_done = 0;
  } else if (!half_done && mus_t >= mus_len * 0.5f) { half_done = 1; music_step(sg, 1); }
  float s = 0, quiet = state == ST_PAUSE ? 0.35f : 1;
  // lead and second voice. Their loudness curves change slowly, so they are only worked out every 32 samples.
  {
    static int ctl; static float env, henv, e9, e35, e5, vib;
    if ((ctl++ & 31) == 0) {
      env = m_lead.on ? minf(1, m_lead.t * 90) * (0.55f + 0.45f * expf(-m_lead.t * 7)) : 0;
      henv = m_harm.on ? minf(1, m_harm.t * 40) * (0.6f + 0.4f * expf(-m_harm.t * 5)) : 0;
      e9 = expf(-m_lead.t * 9); e35 = expf(-m_lead.t * 3.5f); e5 = expf(-m_lead.t * 5);
      vib = m_lead.t > 0.18f ? 1 + 0.006f * SIN1(m_lead.t * 5.73f - (int)(m_lead.t * 5.73f)) : 1;
    }
    m_lead.ph += m_lead.hz * vib * dt; m_lead.ph -= (int)m_lead.ph;
    m_lead.ph2 += m_lead.hz * 3 * dt; m_lead.ph2 -= (int)m_lead.ph2;
    float w, tri = 4 * fabsf(m_lead.ph - 0.5f) - 1;
    if (sg->lead == 4) w = ((2 * m_lead.ph - 1) * 0.5f + tri * 0.45f + 0.2f * SIN1(m_lead.ph2)) * (0.7f + 0.3f * e5);   // brassy
    else if (sg->lead == 3) w = (SIN1(m_lead.ph) + 0.55f * SIN1(m_lead.ph2) * e9 + 0.25f * tri) * (0.25f + 0.75f * e5) * 1.5f;   // steel drum
    else if (sg->lead == 2) w = (SIN1(m_lead.ph) + 0.45f * SIN1(m_lead.ph2) * e9) * e35 * 1.5f;          // bell
    else if (sg->lead == 1) w = ((m_lead.ph < 0.3f ? 1 : -1) * 0.6f + tri * 0.5f) * e5;             // plucky
    else w = tri * 0.75f + (m_lead.ph < 0.5f ? 0.3f : -0.3f) + 0.25f * SIN1(m_lead.ph2);            // round whistle
    s += w * env * 0.075f;
    m_lead.t += dt;
    m_harm.ph += m_harm.hz * dt; m_harm.ph -= (int)m_harm.ph;   // a soft flute
    float h2 = m_harm.ph * 2; h2 -= (int)h2;
    s += (SIN1(m_harm.ph) + 0.3f * SIN1(h2)) * henv * 0.034f;
    m_harm.t += dt;
  }
  m_noise = m_noise * 1664525u + 1013904223u;
  float nz = ((m_noise >> 16) & 0xFFFF) / 32768.0f - 1;
  for (int i = 0; i < NPOOL; i++) {
    MNote* n = &m_pool[i];
    if (n->ins < 0) continue;
    float t = n->t, w = 0, len = 0.3f;
    n->ph += n->hz * dt; n->ph -= (int)n->ph;
    n->env *= M_DEC[n->ins]; n->env2 *= M_DEC2[n->ins];   // fading out: one multiply instead of working out a curve
    switch (n->ins) {
      case I_BASS: len = 0.3f; w = ((4 * fabsf(n->ph - 0.5f) - 1) + 0.5f * SIN1(n->ph)) * n->env * 0.12f; break;
      case I_STAB: len = 0.2f; w = ((n->ph < 0.5f ? 0.5f : -0.5f) + SIN1(n->ph)) * n->env * 0.024f; break;
      case I_MARIMBA: { len = 0.22f; float p4 = n->ph * 4; p4 -= (int)p4; w = (SIN1(n->ph) + 0.35f * SIN1(p4) * n->env2) * n->env * 0.034f; break; }
      case I_BRASS: len = 0.3f; w = ((2 * n->ph - 1) * 0.7f + (4 * fabsf(n->ph - 0.5f) - 1) * 0.5f) * minf(1, t * 60) * n->env * 0.03f; break;
      case I_BELL: { len = 0.35f; float p3 = n->ph * 3; p3 -= (int)p3; w = (SIN1(n->ph) + 0.5f * SIN1(p3)) * n->env * 0.02f; break; }
      case I_KICK: len = 0.14f; n->ph += (50 + 110 * n->env2) * dt; n->ph -= (int)n->ph; w = SIN1(n->ph) * (1 - t / 0.14f) * 0.16f; break;
      case I_SNARE: len = 0.09f; w = nz * (1 - t / 0.09f) * 0.04f; break;
      case I_HAT: len = 0.022f; w = nz * (1 - t / 0.022f) * 0.018f; break;
      case I_BAH: {   // a sung "bah": each beat of the voice rings at two mouth notes, which open up from "b" to "ah"
        len = 0.3f;
        float g = minf(1, t * 28), tau = n->ph / n->hz, win = (1 - n->ph) * (1 - n->ph);
        float f1 = (330 + 470 * g) * tau, f2 = (950 + 300 * g) * tau, f3 = 2600 * tau;
        w = (SIN1(f1 - (int)f1) + 0.5f * SIN1(f2 - (int)f2) + 0.12f * SIN1(f3 - (int)f3)) * win * minf(1, t * 150) * minf(1, (len - t) * 30) * n->env * 0.085f;
        break;
      }
      case I_CLAP: len = 0.07f; w = nz * (t < 0.012f || (t > 0.02f && t < 0.07f) ? 1 : 0) * (1 - t / 0.07f) * 0.03f; break;
    }
    s += w;
    n->t += dt;
    if (n->t >= len) n->ins = -1;
  }
  return s * quiet;
}

void synth_render(int16_t* out, int frames, int rate) {
  if (m_rate != rate) {
    static const float K[NINS] = { 9, 24, 18, 9, 12, 0, 0, 0, 0, 6 }, K2[NINS] = { 0, 0, 60, 0, 0, 40, 0, 0, 0, 0 };
    m_rate = rate;
    for (int i = 0; i < 1024; i++) SINT[i] = sinf(i * 6.28318f / 1024);
    for (int i = 0; i < NINS; i++) { M_DEC[i] = expf(-K[i] / rate); M_DEC2[i] = expf(-K2[i] / rate); }
  }
  for (int f = 0; f < frames; f++) {
    float dt = 1.0f / rate;
    float s = music_sample(dt);
    for (int i = 0; i < 8; i++) {
      Voice* v = &voices[i];
      if (v->t >= v->dur) continue;
      float k = v->t / v->dur, fr;
      if (v->wave == 3) { int n = (int)(k * 4); fr = v->f0 * (n == 0 ? 1 : n == 1 ? 1.26f : n == 2 ? 1.5f : 2.0f); }
      else fr = v->f0 + (v->f1 - v->f0) * k;
      v->phase += fr * dt; v->phase -= (int)v->phase;
      float w;
      if (v->wave == 2) {
        if (v->phase < fr * dt) { uint32_t b = ((v->lfsr >> 0) ^ (v->lfsr >> 1)) & 1; v->lfsr = (v->lfsr >> 1) | (b << 14); }
        w = (v->lfsr & 1) ? 1 : -1;
      } else if (v->wave == 1) w = 4 * fabsf(v->phase - 0.5f) - 1;
      else w = v->phase < 0.5f ? 1 : -1;
      s += w * v->vol * (1 - k);
      v->t += dt;
    }
    s = s / (1 + 0.6f * fabsf(s)) * 1.3f;
    if (s > 1) s = 1;
    if (s < -1) s = -1;
    int16_t o = (int16_t)(s * 26000);
    out[f * 2] = o; out[f * 2 + 1] = o;
  }
}
