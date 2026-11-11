// Canopy Dino: game logic, drawing and sound. See game.h.
#include "game.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define T 16                 // tile size in pixels
#define MAXW 160
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
        *d = mixc(*d, mixc(0xFF1B1020, col, ((p >> 16) & 255) / 255.0f), (p >> 24) / 255.0f);
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
  int theme;   // 0 jungle, 1 temple, 2 cave, 3 sky, 4 waterfalls, 5 snow, 6 ice cave, 7 snow at night
  const Op* ops;
  const char* signs[4];
} Level;

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
};
#define NLEVELS (int)(sizeof LEVELS / sizeof LEVELS[0])

// ============================================================ state
enum { ST_TITLE, ST_PLAY, ST_PAUSE, ST_CLEAR, ST_OVER, ST_WIN, ST_MAP };
enum { E_BEETLE, E_BEE, E_POW, E_ICE };   // E_ICE: an icicle (k: 0 hanging, 1 shaking, 2 falling)   // E_POW: a power-up (e->vx holds which: 1 double jump, 2 sword)
enum { SFX_JUMP, SFX_BANANA, SFX_STOMP, SFX_HURT, SFX_SPRING, SFX_CHECK, SFX_CLEAR, SFX_LIFE, SFX_SELECT, SFX_WALL, SFX_DASH, SFX_SWORD, SFX_JUMP2 };

typedef struct { int type, alive, dying, t, k; float x, y, vx, vy, by, fz; } Ent;
typedef struct { float x, y, vx, vy; int life; uint32_t c; } Part;

static char G[MAXH][MAXW];
static char BOXK[MAXH][MAXW];   // what each ? box holds: 0 bananas, 1 double jump, 2 sword
static int LW, LH, lvl;
static const Level* L;
static Ent ents[64]; static int nents;
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
// world map: node 0 is Dino's house, node n is level n
static int cleared[20], map_node, map_from, map_t, world;
// Each world is its own map. A stop is a level (kind >= 0, the level's index), Dino's house (-1),
// the way back to the world before (-2) or the way on to the next world (-3).
typedef struct { const char* name; int n; short xy[8][2]; signed char kind[8]; } World;
static const World WORLDS[3] = {
  { "JUNGLE ISLAND", 8, { {38, 146}, {84, 134}, {130, 108}, {96, 62}, {166, 50}, {226, 92}, {282, 44}, {306, 112} }, { -1, 0, 1, 2, 3, 4, 5, -3 } },
  { "FROSTY FOREST", 5, { {22, 150}, {80, 128}, {140, 100}, {204, 74}, {276, 46} }, { -2, 6, 7, 8, -3 } },
  { "ICE MOUNTAIN", 4, { {26, 150}, {92, 128}, {164, 98}, {238, 54} }, { -2, 9, 10, 11 } },
};
#define NWORLDS 3
#define MAP_XY (WORLDS[world].xy)
#ifndef SAVE_PATH
#define SAVE_PATH "canopydino-save.txt"
#endif
#ifdef __WIIU__
#include <sys/stat.h>
#endif
static void save_progress(void) {
#ifdef __WIIU__
  mkdir("fs:/vol/external01/wiiu/apps/CanopyDino", 0777);
#endif
  FILE* f = fopen(SAVE_PATH, "w");
  if (!f) return;
  for (int i = 1; i <= NLEVELS; i++) fputc(cleared[i] ? '1' : '0', f);
  fprintf(f, "\n%d\n", total_bananas);
  fclose(f);
}
static void load_progress(void) {
  FILE* f = fopen(SAVE_PATH, "r");
  if (!f) return;
  char b[32] = { 0 };
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
  return G[ty][tx];
}
static int solid_c(char c) { return c == '#' || c == 'X' || c == '?' || c == 'u' || c == 'I'; }
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

static void load_level(int n) {
  lvl = n; L = &LEVELS[n]; LW = L->w; LH = L->h;
  memset(G, ' ', sizeof G); memset(BOXK, 0, sizeof BOXK);
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
        if (nents < 64) {
          Ent* e = &ents[nents++];
          memset(e, 0, sizeof *e);
          e->type = E_ICE; e->alive = 1; e->x = o->x * T + 2; e->y = e->by = o->y * T;
        }
        break;
      case 'c': G[o->y][o->x] = 'c'; break;
      case 'F': goal_x = o->x; goal_y = o->y; break;
      case 'P': start_x = o->x * T + 3; start_y = o->y * T + T - PH; break;
      case 'i': if (nsigns < 4) { sign_x[o->w] = o->x; sign_y[o->w] = o->y; nsigns++; } break;
      case 'b': case 'z':
        if (nents < 64) {
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
  check_x = start_x; check_y = start_y;
  lvl_frames = 0;
  hd_render_level();
}

static void respawn(void) {
  px = check_x; py = check_y; pvx = pvy = 0; face = 1; dead_t = 0; wall_lock = 0; jbuf = 0; atk_t = 0; inv_t = 0;
  camx = px - SCR_W / 2; camy = py - SCR_H / 2;
  // beetles and bees come back when you respawn
  for (int i = 0; i < nents; i++) {
    Ent* e = &ents[i];
    if (e->type == E_POW) continue;
    if (e->type == E_ICE) { e->alive = 1; e->k = 0; e->y = e->by; e->vy = 0; continue; }
    if (!e->alive || e->dying) { e->alive = 1; e->dying = 0; e->y = e->by; }
  }
}

static void start_level(int n) {
  load_level(n);
  // reset enemy spawn positions to the level's original ones
  respawn();
  state = ST_PLAY; state_t = 0;
}

void game_init(void) {
  state = ST_TITLE; state_t = 0; frame = 0;
  load_progress();
  hd_init(); m3d_build();
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
  } else if (nents < 64) {
    Ent* e = &ents[nents++];
    memset(e, 0, sizeof *e);
    e->type = E_POW; e->alive = 1; e->k = k; e->vx = 0.8f;
    e->x = tx * T + 2; e->by = (ty - 1) * T + 2; e->y = ty * T; e->t = 0;
    plat_sfx(SFX_CHECK);
  }
  spawn_parts(tx * T + 8, ty * T, 6, 0xFFD69A55, 1.2f, 0.8f);
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
    else { atk_t = 18; atk_cd = 30; pvx = face * 4.2f; if (pvy > 0.5f) pvy = 0.5f; plat_sfx(SFX_DASH); }
  }
  if (atk_t > 0) {
    atk_t--;
    if (power != 2) { dir = 0; if (atk_t > 6) { pvx = face * 4.2f; if (pvy > 0.6f) pvy = 0.6f; if (frame % 2 == 0) spawn_parts(px + (face > 0 ? 0 : PW), py + PH - 3, 1, 0xFFE8E0C8, 0.5f, 0.1f); } }
    float ax, ay, aw, ah;
    if (attack_box(&ax, &ay, &aw, &ah)) {
      int tx0 = (int)floorf(ax / T), tx1 = (int)floorf((ax + aw) / T), ty0 = (int)floorf(ay / T), ty1 = (int)floorf((ay + ah) / T);
      for (int ty = ty0; ty <= ty1; ty++) for (int tx = tx0; tx <= tx1; tx++) if (tile(tx, ty) == '?') { open_box(tx, ty); if (power != 2) { atk_t = 4; pvx = -face * 1.5f; } }
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
    if (hit) { ny = ty * T - PH; pvy = 0; on_ground = 1; }
  } else if (pvy < 0) {
    if (box_solid(px, ny, PW, PH)) {
      int ty = (int)floorf(ny / T);
      int x0 = (int)floorf((px + 1) / T), x1 = (int)floorf((px + PW - 1.01f) / T);
      for (int tx = x0; tx <= x1; tx++) open_box(tx, ty);
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
  if (px + PW > goal_x * T + 5 && px < goal_x * T + 11 && py + PH > goal_y * T - 3 * T && py < goal_y * T + T) {
    state = ST_CLEAR; state_t = 0; plat_sfx(SFX_CLEAR);
    spawn_parts(goal_x * T + 8, goal_y * T - 32, 24, 0xFFF7D038, 2.5f, 0.5f);
  }
  if (py > LH * T + 24) hurt_ex(1);

  if (on_ground && pvx != 0) run_t++; else run_t = 0;
}

static void update_ents(void) {
  for (int i = 0; i < nents; i++) {
    Ent* e = &ents[i];
    if (!e->alive) continue;
    e->t++;
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
    if (e->type == E_BEETLE) {
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
        e->dying = 1; plat_sfx(SFX_STOMP);
        spawn_parts(e->x + 6, e->y + 4, 10, e->type == E_BEETLE ? 0xFF3FA34D : 0xFFF7D038, 2.0f, 0.5f);
        continue;
      }
      if (px + PW > ex && px < ex + ew - 2 && py + PH > ey && py < ey + eh) {
        if (pvy > 0.5f && py + PH - ey < 9) {
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
      if (sp) { state = ST_PAUSE; plat_sfx(SFX_SELECT); break; }
      lvl_frames++;
      update_player(in);
      dino_tick();
      if (state == ST_PLAY || state == ST_CLEAR) update_ents();
      update_parts();
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
      if ((jp || sp) && state_t > 60) { state = ST_TITLE; state_t = 0; save_progress(); }
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
    if (sy < 0 || sy >= LVB.h) continue;
    const uint32_t* src = LVB.px + sy * LVB.w;
    uint32_t* dst = FB + y * FB_W;
    for (int x = 0; x < FB_W; x++) {
      int sx = x + CX;
      if ((unsigned)sx >= (unsigned)LVB.w) continue;
      uint32_t c = src[sx], a = c >> 24;
      if (!a) continue;
      if (a == 255) dst[x] = c;
      else {
        uint32_t d = dst[x], ia = 255 - a;
        dst[x] = 0xFF000000 | (((((c >> 16) & 255) * a + ((d >> 16) & 255) * ia) / 255) << 16)
               | (((((c >> 8) & 255) * a + ((d >> 8) & 255) * ia) / 255) << 8) | (((c & 255) * a + (d & 255) * ia) / 255);
      }
    }
  }
}
static void fb_shadow(float cx, float cy, float rx, float ry, float alpha) {
  for (int y = (int)(cy - ry - 1); y <= (int)(cy + ry + 1); y++) for (int x = (int)(cx - rx - 1); x <= (int)(cx + rx + 1); x++)
    fb_over(x, y, 0xFF000000, cov(sd_ellipse(x + 0.5f, y + 0.5f, cx, cy, rx, ry)) * alpha);
}
static void draw_world(void) {
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
      hblit(e->k == 2 ? &HD_SWORDI : &HD_WINGI, (int)ex - 4, (int)(ey - 4 + bob), 0);
      continue;
    }
    float want = e->vx > 0 ? 1 : -1;
    if (e->fz == 0) e->fz = want;
    e->fz += (want - e->fz) * 0.22f;   // turns round smoothly instead of snapping
    if (e->type == E_BEETLE) {
      float sq = e->dying ? maxf(0.28f, 1 - e->dying * 0.14f) : 1 + sinf(e->t * 0.64f) * 0.02f;
      beetle3d(&ESCR, e->t * 0.32f, sq, e->fz * 1.15f, L->theme >= 5, e->dying, e->t);
      if (!e->dying) fb_shadow(ex + 12, ey + 21, 12, 2.5f, 0.18f);
      hblit(&ESCR, (int)ex + 12 - 26, (int)ey + 20 - 36, 0);
    } else {
      float tilt = cosf(e->t * 0.07f) * 0.22f + (e->dying ? e->dying * 0.35f : 0);
      bee3d(&ESCR, e->t, tilt, e->fz * 1.15f, e->dying);
      hblit(&ESCR, (int)ex + 12 - 28, (int)ey + 12 - 28 + e->dying * e->dying / 3, 0);
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
    if (on_ground && !dead_t) fb_shadow(X, Y + 1, 12, 2.8f, 0.2f);
    if (show) { dino3d(&DSCR, &p, DA.yaw, 1); hblit(&DSCR, X - 36, Y - 64, 0); }
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
  dino3d(&big, p, yaw, S);
  hblit(&big, X - (int)(36 * S), Y - (int)(64 * S), 0);
}

static void draw_hud(void) {
  char buf[64];
  hblit(&HD_COIN[(frame / 3) % COIN_FRAMES], 6, 3, 0);
  snprintf(buf, sizeof buf, "%02d", bananas);
  text(buf, 21, 6, 2, 0xFFFFF4C2);
  DinoPose p; dino_idle_pose(&p, frame);
  draw_dino_at(FB_W - 94, 34, 0.72f, &p, 0.5f);
  snprintf(buf, sizeof buf, "x%d", lives < 0 ? 0 : lives);
  text(buf, SCR_W - 34, 6, 2, 0xFFFFFFFF);
  if (power) {
    fill(SCR_W - 82, 1, 18, 18, 0xFF1B1020); fill(SCR_W - 81, 2, 16, 16, 0xFF3B2F6B);
    hblit(power == 2 ? &HD_SWORDI : &HD_WINGI, (SCR_W - 81) * 2, 4, 0);
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
static void draw_map(void) {
  char buf[64];
  const World* w = &WORLDS[world];
  // sea
  uint32_t sea0 = 0xFF2F7FC1, sea1 = 0xFF5BB3E6, wave = 0xFF9ED6F5;
  if (world == 1) { sea0 = 0xFF3F7FB4; sea1 = 0xFF86BFE2; wave = 0xFFD6ECFA; }
  if (world == 2) { sea0 = 0xFF0B1030; sea1 = 0xFF24346E; wave = 0xFF4A5C94; }
  for (int y = 0; y < SCR_H; y++) {
    fill(0, y, SCR_W, 1, blend(sea0, sea1, y / (float)SCR_H));
  }
  for (int i = 0; i < 50; i++) {
    uint32_t h = hash2(i, 5);
    int x = (int)(h % SCR_W + frame / 6) % SCR_W, y = (h >> 9) % SCR_H;
    if ((frame / 15 + i) % 4) fill(x, y, 3, 1, wave);
  }
  if (world == 0) map_jungle(); else map_snow(world == 2);
  // path between the stops
  for (int n = 0; n + 1 < w->n; n++) {
    int open = node_open(n + 1);
    float x0 = w->xy[n][0], y0 = w->xy[n][1], x1 = w->xy[n + 1][0], y1 = w->xy[n + 1][1];
    float len = sqrtf((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
    for (float d = 6; d < len - 5; d += 6) {
      int x = (int)(x0 + (x1 - x0) * d / len), y = (int)(y0 + (y1 - y0) * d / len);
      fill(x - 1, y - 1, 3, 3, open ? (world ? 0xFFE08A2C : 0xFFFFF4C2) : (world ? 0xFF9FB4CC : 0xFF3E7D3B));
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
    hd_background();
    // a round grassy hill for Dino to stand on
    for (int y = 280; y < FB_H; y++) for (int x = 0; x < FB_W; x++) {
      float dd = sd_ellipse(x + 0.5f, y + 0.5f, 320, 700, 620, 410);
      if (dd > 1) continue;
      uint32_t c = dd > -12 ? mixc(0xFFC8F27A, 0xFF6CC943, clampf(-dd / 12, 0, 1)) : mixc(0xFF6CC943, 0xFF3B9A36, clampf((-dd - 12) / 60, 0, 1));
      fb_over(x, y, c, cov(dd));
    }
    {
      DinoPose p; dino_idle_pose(&p, state_t);
      float hop = fabsf(sinf(state_t * 0.05f));
      p.bob = hop * 5; p.sq = 1 + 0.06f * sinf(state_t * 0.1f); p.spk = sinf(state_t * 0.1f + 1) * 0.15f; p.mouth = hop > 0.5f ? (hop - 0.5f) * 2 : 0;
      fb_shadow(320, 293, 44 - hop * 6, 6, 0.2f);
      draw_dino_at(320, 292, 3, &p, 0.35f + sinf(state_t * 0.025f) * 0.45f);
    }
    hblit(&HD_COIN[(frame / 2) % COIN_FRAMES], 80, 250 + (int)(sinf(frame * 0.08f) * 4), 0);
    hblit(&HD_COIN[(frame / 2 + 8) % COIN_FRAMES], 528, 250 + (int)(sinf(frame * 0.08f + 2) * 4), 0);
    text_c("CANOPY DINO", 24, 5, 0xFFF7D038);
    text_c("A LITTLE JUNGLE ADVENTURE", 58, 1, 0xFF3B2414);
    if ((state_t / 30) % 2 == 0) text_c("PRESS A TO START", 80, 2, 0xFFFFFFFF);
    text_c("MOVE: STICK   JUMP: A OR B   HEADBUTT: X OR Y   PAUSE: +", 167, 1, 0xFFFFF4C2);
    return;
  }
  if (state == ST_MAP) { draw_map(); return; }
  hd_background();
  draw_world();
  if (L->theme == 5 || L->theme == 7) {
    for (int i = 0; i < 90; i++) {   // soft snowflakes drifting down
      uint32_t h = hash2(i, 31);
      float fx = fmodf(h % FB_W + frame * (0.5f + (i % 3) * 0.24f) + sinf(frame * 0.03f + i) * 6 - camx * 1.2f, FB_W), fy = fmodf((h >> 9) % FB_H + frame * (1.0f + (i % 4) * 0.4f) - camy * 1.2f, FB_H);
      if (fx < 0) fx += FB_W;
      if (fy < 0) fy += FB_H;
      fb_dot(fx, fy, 1.4f + (i % 3) * 0.7f, 0xFFFFFFFF, 0.9f);
    }
  }
  if (!g_no_hud) draw_hud();
  if (state == ST_PLAY && state_t < 120) {
    text_c(L->name, 60, 3, 0xFFFFFFFF);
  }
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
    darken(0, 0, SCR_W, SCR_H, 0.6f);
    text_c("GAME OVER", 60, 4, 0xFFD8463C);
    if (state_t > 60) text_c("PRESS A", 100, 2, 0xFFFFFFFF);
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
typedef struct { int base, bpm, lead; float swing; const char* mel; const char* chords; } Song;
enum { SONG_TITLE, SONG_MAP, SONG_GRASS, SONG_CAVE, SONG_SNOW, SONG_BOSS, NSONGS };
static const Song SONGS[NSONGS] = {
  { 62, 164, 0, 0.60f,   // Hello Dino (title)
    "0.47.C.7" "9.CH.L.H" "J.EB.7.B" "C.7C.G.." "L.GL.G.C" "H.CH.L.H" "J.EJ.E.B" "E.B.7-.."
    "0.47.C.G" "H.L.H.C9" "B.E.J.EB" "C.G.J.G." "L.H.L.H." "J.E.J.N." "O.JG.C.G" "C--.0...",
    "0M5M7M0M9m5M7M7M0M5M7M0M5M7M0M0M" },
  { 67, 124, 2, 0.62f,   // Which Way? (map)
    "7.47.C.7" "9.49.C.G" "H.C9.5.9" "7.B.E.B7" "C.GC.J.G" "G.C9.C.G" "H.C9.E.B" "C--.0...",
    "0M9m5M7M0M9m5M0M" },
  { 60, 150, 0, 0.62f,   // Jungle Hop (grassy levels)
    "7.47.C.7" "974.0.24" "5.9C.95." "7.BE.B7-" "7.47.C.G" "EC9.C.EG" "H.GE.C9." "B.E.C--."
    "9.CH.GH." "G.CG.EC." "E.HE.95." "7BE.J.HE" "G.JG.EC." "L.JG.E.C" "9CH.BEJ." "O--.C...",
    "0M0M5M7M0M9m5M7M5M0M2m7M0M9m7M0M" },
  { 57, 118, 1, 0.5f,    // Sneaky Steps (temples and caves)
    "C..F.E.C" "7..C.A.7" "H.FH.K.H" "J.EJ.N.J" "O.JF.C.." "K.FC.8.C" "H.FH.K.H" "J.N.J-.."
    "K.K.O.K." "M.H.M.J." "O.J.F.J." "C.F.J-.." "H.K.H.F." "J.N.J.E." "C.7C.F.J" "O--.C...",
    "0m0m5m7M0m8M5m7M8MAM0m0m5m7M0m0m" },
  { 65, 132, 2, 0.58f,   // Frosty Skip (snow levels)
    "C.7C.G.E" "C.9.7.9C" "H.EC.9C." "E.B.7.BE" "G.CG.J.G" "E.C9.C.9" "E.H.L.HE" "J.G.E-.."
    "H.E.C.EH" "J.E.B.EJ" "G.B.7.BG" "L.G.C.9." "H.E.9.5." "E.B.7.B." "C.7C.G.J" "C--.0...",
    "0M9m5M7M0M9m2m7M5M7M4m9m2m7M0M0M" },
  { 64, 172, 1, 0.5f,    // Big Trouble (bosses)
    "0.0C.0.A" "0.0C.F.E" "8.8F.8.C" "A.AH.E.A" "C.CJ.C.F" "C.EF.H.J" "K.JF.C.F" "J.N.J.E-",
    "0m0m8MAM0m0m8M7M" },
};
static int note_code(char c) { return c >= '0' && c <= '9' ? c - '0' : c - 'A' + 10; }
static float midi_hz(int n) { return 440.0f * powf(2.0f, (n - 69) / 12.0f); }
static int song_now = -1, mus_step;
static float mus_t, mus_len;
static struct { float hz, ph, ph2, t; int on; } m_lead;
static struct { float hz, ph, t; } m_bass, m_stab[3];
static struct { float t, ph; } m_kick;
static float m_snare = 9, m_hat = 9; static uint32_t m_noise = 12345;
int g_boss_music;   // set by the game while a boss is awake

static int pick_song(void) {
  if (state == ST_TITLE) return SONG_TITLE;
  if (state == ST_MAP) return SONG_MAP;
  if (state == ST_PLAY || state == ST_PAUSE) {
    if (g_boss_music) return SONG_BOSS;
    int th = L->theme;
    return th == 1 || th == 2 || th == 6 ? SONG_CAVE : (th == 5 || th == 7 ? SONG_SNOW : SONG_GRASS);
  }
  return -1;
}
static void music_step(const Song* sg) {
  int nsteps = (int)strlen(sg->mel), bar = mus_step / 8, st = mus_step % 8;
  char m = sg->mel[mus_step % nsteps];
  if (m == '.') m_lead.on = 0;
  else if (m != '-') { m_lead.hz = midi_hz(sg->base + note_code(m)); m_lead.t = 0; m_lead.on = 1; }
  int root = sg->base - 24 + note_code(sg->chords[(bar * 2) % strlen(sg->chords)]);
  int minor = sg->chords[(bar * 2 + 1) % strlen(sg->chords)] == 'm';
  if (root > sg->base - 24 + 7) root -= 12;
  if (st == 0 || st == 4 || st == 7) { m_bass.hz = midi_hz(st == 4 ? root + 7 : (st == 7 ? root + 12 : root)); m_bass.t = 0; }
  if (st == 2 || st == 6 || (sg->bpm > 160 && st == 3)) {
    int tri[3] = { root + 24, root + 24 + (minor ? 3 : 4), root + 24 + 7 };
    for (int i = 0; i < 3; i++) { m_stab[i].hz = midi_hz(tri[i]); m_stab[i].t = 0; }
    m_snare = 0;
  }
  if (st == 0 || st == 4 || (sg->bpm > 160 && st == 5)) { m_kick.t = 0; m_kick.ph = 0; }
  if (st % 2 == 1 || sg->bpm > 160) m_hat = 0;
}
static float music_sample(float dt) {
  int want = pick_song();
  if (want != song_now) { song_now = want; mus_step = -1; mus_t = mus_len = 0; m_lead.on = 0; m_bass.t = 9; m_stab[0].t = m_stab[1].t = m_stab[2].t = 9; m_kick.t = 9; m_snare = m_hat = 9; }
  if (want < 0) return 0;
  const Song* sg = &SONGS[want];
  mus_t += dt;
  if (mus_t >= mus_len) {
    mus_t -= mus_len;
    mus_step = (mus_step + 1) % (int)strlen(sg->mel);
    float beat = 60.0f / sg->bpm;                       // two eighth notes, the first one longer (swing)
    mus_len = beat * (mus_step % 2 == 0 ? sg->swing : 1 - sg->swing);
    music_step(sg);
  }
  float s = 0, quiet = state == ST_PAUSE ? 0.35f : 1;
  // lead
  {
    float env = m_lead.on ? minf(1, m_lead.t * 90) * (0.55f + 0.45f * expf(-m_lead.t * 7)) : 0;
    if (!m_lead.on && m_lead.t < 0.08f) env = 0.5f * (1 - m_lead.t / 0.08f);
    float vib = m_lead.t > 0.18f ? 1 + 0.006f * sinf(m_lead.t * 36) : 1;
    m_lead.ph += m_lead.hz * vib * dt; m_lead.ph -= (int)m_lead.ph;
    m_lead.ph2 += m_lead.hz * 2 * dt; m_lead.ph2 -= (int)m_lead.ph2;
    float w;
    if (sg->lead == 2) w = (sinf(m_lead.ph * 6.28318f) + 0.45f * sinf(m_lead.ph2 * 6.28318f * 1.5f) * expf(-m_lead.t * 9)) * expf(-m_lead.t * 3.5f) * 1.5f;   // bell
    else if (sg->lead == 1) w = ((m_lead.ph < 0.3f ? 1 : -1) * 0.6f + (4 * fabsf(m_lead.ph - 0.5f) - 1) * 0.5f) * expf(-m_lead.t * 5);                       // plucky
    else w = (4 * fabsf(m_lead.ph - 0.5f) - 1) * 0.75f + (m_lead.ph < 0.5f ? 0.3f : -0.3f) + 0.25f * sinf(m_lead.ph2 * 6.28318f);                          // round whistle
    s += w * env * 0.085f;
    m_lead.t += dt;
  }
  // bass: a bouncy, short note
  if (m_bass.t < 0.4f) {
    m_bass.ph += m_bass.hz * dt; m_bass.ph -= (int)m_bass.ph;
    s += ((4 * fabsf(m_bass.ph - 0.5f) - 1) + 0.5f * sinf(m_bass.ph * 6.28318f)) * expf(-m_bass.t * 7) * 0.13f;
    m_bass.t += dt;
  }
  // off-beat chord "pah"
  for (int i = 0; i < 3; i++) if (m_stab[i].t < 0.25f) {
    m_stab[i].ph += m_stab[i].hz * dt; m_stab[i].ph -= (int)m_stab[i].ph;
    s += ((m_stab[i].ph < 0.5f ? 0.5f : -0.5f) + sinf(m_stab[i].ph * 6.28318f)) * expf(-m_stab[i].t * 22) * 0.028f;
    m_stab[i].t += dt;
  }
  // drums
  if (m_kick.t < 0.14f) { m_kick.ph += (50 + 110 * expf(-m_kick.t * 40)) * dt; s += sinf(m_kick.ph * 6.28318f) * (1 - m_kick.t / 0.14f) * 0.16f; m_kick.t += dt; }
  m_noise = m_noise * 1664525u + 1013904223u;
  float nz = ((m_noise >> 16) & 0xFFFF) / 32768.0f - 1;
  if (m_snare < 0.09f) { s += nz * (1 - m_snare / 0.09f) * 0.045f; m_snare += dt; }
  if (m_hat < 0.025f) { s += nz * (1 - m_hat / 0.025f) * 0.022f; m_hat += dt; }
  return s * quiet;
}

void synth_render(int16_t* out, int frames, int rate) {
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
