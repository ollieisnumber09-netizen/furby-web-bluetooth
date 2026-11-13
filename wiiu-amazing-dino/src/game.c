// Amazing Dino: game logic, drawing and sound. See game.h.
#include "game.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define T 16                 // tile size in pixels
#define MAXW 660
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
  int theme;   // 0 jungle, 1 temple, 2 cave, 3 sky, 4 waterfalls, 5 snow, 6 ice cave, 7 snow at night, 8 desert, 9 underwater, 10 beach
  const Op* ops;
  const char* signs[4];
} Level;
// more ops: m moving platform (w: how many tiles it travels, h: 1 = up and down)  k cracked blocks w*h (the helmet smashes them)
//           h / H a pretend wall of earth / stone w*h (walk through it)  _ dig out a room w*h
//           f frog  y spiky ball  p seed-spitting flower  a bat   B boxes: w = 3 helmet, 4 star

// ops: g ground (dirt to the bottom)  G ground (stone to the bottom)  x stone block w*h
//      = one-way plank  ^ spikes  o bananas  b beetle  z bee  s spring  c checkpoint
//      F goal flag  P start  i sign (w = which text)
// ---- the levels: 7 worlds of 8. Every level is built by build_level() from set pieces, the same
// way every time. The last level of each world ends with a boss.
#define C_JUNGLE 0xFF7EC8F0, 0xFFD4F1F9, 0xFF6FB07E, 0xFF3F8452
#define C_EVENING 0xFFF7A35C, 0xFFFDE2B0, 0xFFC97B5A, 0xFF7A4B3A
#define C_FALLS 0xFF5FB4C9, 0xFFC8EEF0, 0xFF5E9C8C, 0xFF2F6B5E
#define C_TEMPLE 0xFF3B2F6B, 0xFF9C6FA8, 0xFF5A4A86, 0xFF3A2F5C
#define C_CAVE 0xFF1E1A24, 0xFF4A3B3A, 0xFF3A302E, 0xFF2A2220
#define C_SKY 0xFF4A90E2, 0xFFBFE3FF, 0xFFFFFFFF, 0xFFE3F1FF
#define C_SNOW 0xFF8FC4EC, 0xFFEAF6FF, 0xFFD5E6F2, 0xFF7FA6BE
#define C_ICECAVE 0xFF0E2238, 0xFF1F4A6E, 0xFF18395A, 0xFF12304C
#define C_NIGHT 0xFF0B1030, 0xFF2B3F7A, 0xFF7F93C4, 0xFF4A5C94
#define C_VINE 0xFFF08A4A, 0xFFFFE0A8, 0xFF7FA85E, 0xFF3F7A46
#define C_BAMBOO 0xFF4FC2B8, 0xFFD8F7E4, 0xFF63B08A, 0xFF2F7A5A
#define C_THUNDER 0xFF4A4A78, 0xFFB8A8C8, 0xFF4F7A6A, 0xFF2A5244
#define C_PINKSNOW 0xFFF0A8C0, 0xFFFFF0E0, 0xFFE8DCEC, 0xFF9A9CC4
#define C_CRYSTAL 0xFF141038, 0xFF3A2F7A, 0xFF2A2460, 0xFF1C1848
#define C_BLIZZARD 0xFF10183C, 0xFF3F5490, 0xFF8FA3D0, 0xFF55689C
#define C_DESERT 0xFF58B8F0, 0xFFFFF0C8, 0xFFE8C888, 0xFF5FA05A
#define C_SUNSET 0xFFF0A050, 0xFFFFE8B0, 0xFFE0B070, 0xFF5A9456
#define C_CLOUD 0xFF5AA0F0, 0xFFD0ECFF, 0xFFFFFFFF, 0xFFE3F1FF
#define C_BELFRY 0xFF2A2050, 0xFF7A5A98, 0xFF4A3E78, 0xFF30284E
#define C_STORM 0xFF3A4670, 0xFF9AA8C8, 0xFFC8D0E4, 0xFFA8B4D0
// water levels: sky0 is the water near the top, sky1 the deep water, far the rocks behind, nearc the weed
#define C_LAGOON 0xFF4FC8E0, 0xFF0E4A7A, 0xFF2A7A9A, 0xFF1E8A6A
#define C_REEF 0xFF58D0E8, 0xFF124A8A, 0xFF3A6AA8, 0xFFE0607A
#define C_WRECK 0xFF2A6A9A, 0xFF061A30, 0xFF1A3A5A, 0xFF2A5A4A
#define C_ICEWATER 0xFF9AD8F0, 0xFF1A3A6A, 0xFF6A9AC8, 0xFF5A8AB8
#define C_OASIS 0xFF5ADCD0, 0xFF1A6A7A, 0xFF4A9A8A, 0xFF3AAA5A
#define C_RIVER 0xFF6AC8A8, 0xFF0F4A3A, 0xFF2A6A5A, 0xFF3A8A3A
#define C_SKYPOOL 0xFF9AD0FF, 0xFF3A5AB0, 0xFF7A9AE0, 0xFFC8D8FF
// beach levels: far is the sea, nearc the palm trees
#define C_BEACH 0xFF5AB8F0, 0xFFE0F4FF, 0xFF2A9AD0, 0xFF3A9A4A
#define C_BEACHSET 0xFFF08A6A, 0xFFFFE0B0, 0xFF3A7AB0, 0xFF2A7A3A
#define LV(nm, th, col, ...) { nm, 0, 0, col, th, NULL, { __VA_ARGS__ } }
static const Level LEVELS[] = {
  // world 1: Jungle Island
  LV("1  JUNGLE FLOOR", 0, C_JUNGLE, "A: JUMP. HOLD IT TO JUMP HIGHER", "X OR Y: HEADBUTT. JUMP UNDER BOXES TO OPEN THEM", "FIND THE 3 KEY PIECES. THEY OPEN THE DOOR AT THE END"),
  LV("2  TREETOP TRAIL", 0, C_EVENING, "JUMP AT A WALL, THEN JUMP AGAIN TO WALL-JUMP", "CRUMBLY PLATFORMS DROP AWAY SOON AFTER YOU LAND"),
  LV("3  MISTY FALLS", 4, C_FALLS, "JUMP INTO A POOL TO SWIM. PRESS A TO SWIM UP"),
  LV("4  LAGOON DIVE", 9, C_LAGOON, "UNDERWATER: PRESS A AGAIN AND AGAIN TO SWIM UP", "HOLD DOWN TO SINK FASTER"),
  LV("5  OLD TEMPLE", 1, C_TEMPLE, "SOME WALLS ARE NOT REAL. TRY WALKING INTO THEM"),
  LV("6  BEETLE BURROWS", 2, C_CAVE, 0),
  LV("7  SKY BRIDGE", 3, C_SKY, 0),
  LV("8  KING BEETLE'S DEN", 2, C_CAVE, 0),
  // world 2: Coral Coast
  LV("9  SANDY SHORE", 10, C_BEACH, "CRABS SCUTTLE FAST, THEN STOP TO SNAP. JUMP ON THEM!"),
  LV("10 TIDE POOLS", 10, C_BEACH, "FISH DART AT YOU WHEN YOU SWIM IN FRONT OF THEM"),
  LV("11 CORAL REEF", 9, C_REEF, "JELLYFISH STING IF YOU LAND ON THEM. HEADBUTT THEM!"),
  LV("12 PALM GROVE", 10, C_BEACHSET, "BIRDS SWOOP DOWN WHEN YOU WALK UNDER THEM"),
  LV("13 SHIPWRECK", 9, C_WRECK, 0),
  LV("14 LIGHTHOUSE CLIFFS", 3, C_SKY, 0),
  LV("15 SEA CAVE", 9, C_LAGOON, 0),
  LV("16 CROAK'S LAGOON", 10, C_BEACHSET, 0),
  // world 3: Frosty Forest
  LV("17 SNOWY TRAIL", 5, C_SNOW, "ICE IS SLIPPERY. YOU KEEP SLIDING WHEN YOU STOP!", "PENGUINS FLOP DOWN AND SLIDE AT YOU. JUMP OVER THEM"),
  LV("18 PINE HILLS", 5, C_PINKSNOW, "SNOWMEN THROW SNOWBALLS. HEADBUTT THEM OR JUMP ON THEM"),
  LV("19 FROZEN LAKE", 9, C_ICEWATER, 0),
  LV("20 ICICLE CAVE", 6, C_ICECAVE, "LOOK UP! ICICLES FALL WHEN YOU WALK UNDER THEM"),
  LV("21 PENGUIN SLOPES", 5, C_SNOW, 0),
  LV("22 SNOWMAN VALLEY", 7, C_NIGHT, 0),
  LV("23 CRYSTAL CAVERN", 6, C_CRYSTAL, 0),
  LV("24 FROST KING'S HALL", 6, C_ICECAVE, 0),
  // world 4: Sunny Dunes
  LV("25 CACTUS TRAIL", 8, C_DESERT, "MOLES POP UP AND THROW EARTH. HIT THEM WHILE THEY ARE UP", "THE STAR POWER THROWS BOUNCING STARS: PRESS X"),
  LV("26 DUNE HOPPERS", 8, C_SUNSET, "ARMADILLOS ROLL UP INTO A BALL. ONLY STOMP THEM UNROLLED"),
  LV("27 OASIS SPRINGS", 9, C_OASIS, 0),
  LV("28 MOLE TUNNELS", 1, C_TEMPLE, "THE HELMET SMASHES CRACKED BLOCKS: HEADBUTT THEM"),
  LV("29 SANDSTONE STEPS", 8, C_SUNSET, 0),
  LV("30 PYRAMID PATH", 1, C_TEMPLE, 0),
  LV("31 SUNSET RIDGE", 8, C_SUNSET, 0),
  LV("32 QUEEN BEE'S HIVE", 8, C_DESERT, 0),
  // world 5: Wild Jungle
  LV("33 VINE VALLEY", 0, C_VINE, 0),
  LV("34 BAMBOO BRIDGES", 4, C_BAMBOO, 0),
  LV("35 RIVER RAPIDS", 9, C_RIVER, 0),
  LV("36 THUNDER GROVE", 0, C_THUNDER, 0),
  LV("37 ARMADILLO RUN", 0, C_VINE, 0),
  LV("38 HIDDEN RUINS", 1, C_TEMPLE, 0),
  LV("39 MOSSY DEPTHS", 9, C_RIVER, 0),
  LV("40 JUNGLE KING'S THRONE", 0, C_THUNDER, 0),
  // world 6: Snowflake Peaks
  LV("41 SNOWBALL HILLS", 5, C_PINKSNOW, 0),
  LV("42 NORTHERN LIGHTS", 7, C_NIGHT, 0),
  LV("43 GLACIER GAP", 7, C_BLIZZARD, 0),
  LV("44 ICY DEPTHS", 9, C_ICEWATER, 0),
  LV("45 CRYSTAL MINES", 6, C_CRYSTAL, "BATS SLEEP UNTIL YOU COME CLOSE"),
  LV("46 BLIZZARD PASS", 7, C_BLIZZARD, 0),
  LV("47 THE SUMMIT", 7, C_NIGHT, 0),
  LV("48 SNOW QUEEN'S PEAK", 7, C_BLIZZARD, 0),
  // world 7: Cloud Castle
  LV("49 CLOUD HOP", 3, C_CLOUD, 0),
  LV("50 WINDY HEIGHTS", 3, C_SKY, 0),
  LV("51 RAINBOW BRIDGE", 3, C_CLOUD, 0),
  LV("52 SKY POOL", 9, C_SKYPOOL, 0),
  LV("53 BAT BELFRY", 1, C_BELFRY, 0),
  LV("54 STORM TOWER", 3, C_STORM, 0),
  LV("55 CASTLE GATES", 1, C_BELFRY, 0),
  LV("56 BAT BARON'S KEEP", 1, C_BELFRY, 0),
};
#define NLEVELS (int)(sizeof LEVELS / sizeof LEVELS[0])

// ============================================================ state
enum { ST_TITLE, ST_PLAY, ST_PAUSE, ST_CLEAR, ST_OVER, ST_WIN, ST_MAP };
enum { E_BEETLE, E_BEE, E_POW, E_ICE, E_BOSS, E_PLAT, E_SHOT, E_FROG, E_SPINY, E_PLANT, E_BAT,
       E_CRAB, E_FISH, E_JELLY, E_ROLL, E_PENGUIN, E_MOLE, E_BIRD, E_SNOWMAN };
#define NCRIT (E_SNOWMAN - E_FROG + 1)
// E_PLAT: a moving platform (k: 0 side to side, 1 up and down, 2 drops when stood on; vx: how far).
// E_SHOT: k 0 a seed spat at Dino, 1 Dino's bouncing star, 2 a clod of earth, 3 a snowball (2 and 3 fly in an arc)
#define IS_FOE(t) ((t) == E_BEETLE || (t) == E_BEE || ((t) >= E_FROG && (t) <= E_SNOWMAN))   // E_ICE: an icicle (k: 0 hanging, 1 shaking, 2 falling)   // E_POW: a power-up (e->vx holds which: 1 double jump, 2 sword)
enum { SFX_JUMP, SFX_BANANA, SFX_STOMP, SFX_HURT, SFX_SPRING, SFX_CHECK, SFX_CLEAR, SFX_LIFE, SFX_SELECT, SFX_WALL, SFX_DASH, SFX_SWORD, SFX_JUMP2 };

typedef struct { int type, alive, dying, t, k; float x, y, vx, vy, by, fz, x0; } Ent;
typedef struct { float x, y, vx, vy; int life; uint32_t c; } Part;

static char G[MAXH][MAXW];
static unsigned char FAKE[MAXH][MAXW];   // a pretend wall: looks like ground, but you can walk through it
static int tile_raw, ride_i = -1;        // tile_raw: show pretend walls as ground (for painting). ride_i: the platform Dino stands on
static Bmp CRIT_C[NCRIT][2][8], HD_FPLAT, HD_DOOR, HD_KEYP, HD_PLAT, HD_CRACK, HD_HELMI, HD_STARI, HD_HELM;
static char BOXK[MAXH][MAXW];   // what each ? box holds: 0 bananas, 1 double jump, 2 sword
static int LW, LH, lvl;
static const Level* L;
#define MAXENT 420
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
// walking speed, top sprinting speed (built up over RUN_RAMP ticks of holding a back trigger), gravity while jump is held
#ifndef WALK_V
#define WALK_V 1.7f
#endif
#define RUN_V 3.0f
#define RUN_RAMP 90
#define G_HOLD 0.22f
#define TWIRL_T 18   // 0.3 seconds
static int slot, run_ramp, prev_run, prev_pop, twirl_t, twirl_used, snap_turn;   // slot: the power-up kept for later
static int face, on_ground, coyote, jbuf, wall_lock, wall_dir, dead_t, run_t, land_t;
static int start_x, start_y, check_x, check_y, prev_jump, prev_start, prev_down, prev_up, prev_left, prev_right;
static int ncheck;
#define PW 10
#define PH 13

static int lvl_frames; int g_no_hud;
// in_water: Dino is swimming. keyp: which of the three key pieces he holds (bits). door_msg_t: shows
// "find the key" for a while after bumping the locked door.
static int in_water, keyp, door_msg_t, key_msg_t, level_keys;
static struct { float x, y; int life; } bubbles[32];
static void add_bubble(float x, float y) {
  for (int i = 0; i < 32; i++) if (bubbles[i].life <= 0) { bubbles[i].x = x; bubbles[i].y = y; bubbles[i].life = 70 + (frame * 7 + i * 13) % 50; return; }
}
// intro_t: counts up while a level is being introduced (Dino and the counters on black, then they
// fly to their places). over_sel: which button is picked on the game over screen.
static int intro_t, over_sel, dino_sx, dino_sy;
#define INTRO_HOLD 85
#define INTRO_FLY 60
// world map: node 0 is Dino's house, node n is level n
static int cleared[64], map_node, map_from, map_t, world;
// Each world is its own map. A stop is a level (kind >= 0, the level's index), Dino's house (-1),
// the way back to the world before (-2) or the way on to the next world (-3).
typedef struct { const char* name; int n; short xy[10][2]; signed char kind[10]; } World;
#define W8(a) { a, a + 1, a + 2, a + 3, a + 4, a + 5, a + 6, a + 7 }
static const World WORLDS[7] = {
  { "JUNGLE ISLAND", 10, { {26, 146}, {62, 128}, {98, 146}, {132, 120}, {112, 86}, {148, 62}, {192, 78}, {222, 112}, {262, 92}, {300, 60} }, { -1, 0, 1, 2, 3, 4, 5, 6, 7, -3 } },
  { "CORAL COAST", 10, { {22, 62}, {56, 86}, {92, 66}, {128, 90}, {158, 122}, {198, 140}, {236, 118}, {222, 82}, {262, 54}, {300, 98} }, { -2, 8, 9, 10, 11, 12, 13, 14, 15, -3 } },
  { "FROSTY FOREST", 10, { {22, 100}, {54, 132}, {92, 148}, {126, 120}, {150, 86}, {120, 54}, {168, 40}, {212, 62}, {250, 98}, {296, 130} }, { -2, 16, 17, 18, 19, 20, 21, 22, 23, -3 } },
  { "SUNNY DUNES", 10, { {24, 146}, {58, 122}, {94, 140}, {130, 116}, {166, 136}, {200, 108}, {172, 72}, {210, 48}, {252, 70}, {296, 104} }, { -2, 24, 25, 26, 27, 28, 29, 30, 31, -3 } },
  { "WILD JUNGLE", 10, { {22, 56}, {58, 78}, {46, 116}, {84, 140}, {120, 116}, {156, 140}, {192, 116}, {226, 140}, {262, 112}, {300, 76} }, { -2, 32, 33, 34, 35, 36, 37, 38, 39, -3 } },
  { "SNOWFLAKE PEAKS", 10, { {24, 140}, {60, 118}, {96, 136}, {124, 104}, {100, 70}, {140, 48}, {184, 64}, {218, 94}, {256, 72}, {296, 44} }, { -2, 40, 41, 42, 43, 44, 45, 46, 47, -3 } },
  { "CLOUD CASTLE", 9, { {24, 146}, {64, 128}, {104, 144}, {140, 118}, {120, 84}, {156, 60}, {198, 78}, {236, 104}, {280, 70} }, { -2, 48, 49, 50, 51, 52, 53, 54, 55 } },
};
#define NWORLDS 7
#define SNOW_WORLD (world == 2 || world == 5)
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
  char b[128] = { 0 };
  // a save from before the worlds were rebuilt has fewer levels in it: start afresh then
  if (fgets(b, sizeof b, f) && (int)strcspn(b, "\r\n") >= NLEVELS) for (int i = 1; i <= NLEVELS && b[i - 1]; i++) cleared[i] = b[i - 1] == '1';
  else { fclose(f); return; }
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
static int solid_c(char c) { return c == '#' || c == 'X' || c == '?' || c == 'u' || c == 'I' || c == 'k' || c == 'D'; }
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

// ============================================================ level builder
// A level is built from a long run of set pieces picked (the same way every time) to suit its
// world and how far into the game it is. Three key pieces are hidden along the way and a locked
// door stands before the flag. The last level of each world has a boss arena instead of a door.
static uint32_t gen_seed;
static int grnd(int n) { gen_seed = gen_seed * 1664525u + 1013904223u; return (int)((gen_seed >> 10) % (uint32_t)n); }
static Ent* add_ent(int type, int tx, int ty) {
  if (nents >= MAXENT) return NULL;
  Ent* e = &ents[nents++];
  memset(e, 0, sizeof *e);
  e->type = type; e->alive = 1;
  e->x = e->x0 = tx * T + 2; e->y = (type == E_BEETLE || type == E_FROG || type == E_SPINY || type == E_CRAB || type == E_ROLL || type == E_PENGUIN || type == E_MOLE) ? ty * T + 6 : (type == E_PLANT || type == E_SNOWMAN) ? ty * T + 2 : ty * T; e->by = e->y;
  e->vx = type == E_SPINY ? -0.4f : -0.5f; e->t = tx * 13;
  return e;
}
static const char* const BOSS_NAME[7] = { "KING BEETLE", "FROST KING", "QUEEN BEE", "JUNGLE KING", "SNOW QUEEN", "KING CROAK", "BAT BARON" };
#define BOSS_BEE(k) ((k) == 2 || (k) == 4 || (k) == 6)
#define BOSS_COL(k) ((k) == 0 ? 2 : (k) == 1 ? 1 : 0)   // beetle3d colour: red, icy, green
static int boss_kind(int n) { static const signed char K[7] = { 0, 5, 1, 2, 3, 4, 6 }; return n % 8 == 7 ? K[n / 8] : -1; }

static int GX, GY, g_world, g_n, g_top, g_bot;   // GX: next free column. GY: the top row of the ground there
static char GC;                                   // what the ground is made of
static float g_diff;                              // 0 at the start of the game .. 1 at the end
static void SET(int x, int y, char c) { if (x >= 0 && x < MAXW && y >= 0 && y < LH) G[y][x] = c; }
static char GET(int x, int y) { return (x >= 0 && x < MAXW && y >= 0 && y < LH) ? G[y][x] : 'X'; }
static void colm(int x, int top) { for (int y = top < 0 ? 0 : top; y < LH; y++) SET(x, y, GC); }
static void flat(int w) { for (int i = 0; i < w; i++) colm(GX + i, GY); GX += w; }
static void coin(int x, int y) { if (GET(x, y) == ' ' || GET(x, y) == 'w') SET(x, y, 'o'); }
static void coin_row(int x, int y, int n) { for (int i = 0; i < n; i++) coin(x + i, y); }
static void coin_arc(int x, int y, int n) { for (int i = 0; i < n; i++) coin(x + i, y - (int)lroundf(sinf(3.14159f * (i + 0.5f) / n) * 2)); }
static void box(int x, int y, int kind) { SET(x, y, '?'); if (x >= 0 && x < MAXW && y >= 0 && y < MAXH) BOXK[y][x] = (char)kind; }
static void key_at(int x, int y, int piece) { SET(x, y, 'K'); if (x >= 0 && x < MAXW && y >= 0 && y < MAXH) BOXK[y][x] = (char)piece; }
static Ent* plat_at(int x, int y, int kind, int travel) {
  Ent* e = add_ent(E_PLAT, x, y);
  if (e) { e->x = e->x0 = x * T; e->y = e->by = y * T; e->vx = travel * T; e->k = kind; e->t = 0; }
  return e;
}
static int cave_theme(void) { int th = L->theme; return th == 1 || th == 2 || th == 6; }
static int snow_theme(void) { int th = L->theme; return th >= 5 && th <= 7; }
// the creatures each world uses, on the ground and in the air
static const signed char POOL_G[7][6] = {
  { E_BEETLE, E_FROG, E_BEETLE, E_PLANT, E_BEETLE, E_FROG },          // Jungle Island
  { E_CRAB, E_CRAB, E_FROG, E_CRAB, E_PLANT, E_BEETLE },              // Coral Coast
  { E_PENGUIN, E_SNOWMAN, E_BEETLE, E_PENGUIN, E_SNOWMAN, E_BEETLE }, // Frosty Forest
  { E_MOLE, E_ROLL, E_MOLE, E_SPINY, E_PLANT, E_ROLL },               // Sunny Dunes
  { E_ROLL, E_FROG, E_PLANT, E_BEETLE, E_ROLL, E_SPINY },             // Wild Jungle
  { E_PENGUIN, E_SNOWMAN, E_SPINY, E_ROLL, E_SNOWMAN, E_PENGUIN },    // Snowflake Peaks
  { E_SPINY, E_ROLL, E_BEETLE, E_SPINY, E_ROLL, E_FROG },             // Cloud Castle
};
static const signed char POOL_A[7][3] = {
  { E_BEE, E_BEE, E_BIRD }, { E_BIRD, E_BIRD, E_BEE }, { E_BEE, E_BIRD, E_BIRD }, { E_BEE, E_BIRD, E_BEE },
  { E_BEE, E_BIRD, E_BIRD }, { E_BIRD, E_BAT, E_BIRD }, { E_BIRD, E_BAT, E_BEE } };
static void foe_g(int x) {   // a creature standing on the ground at column x
  int t = POOL_G[g_world][grnd(6)];
  if (g_n < 2 && t != E_BEETLE) t = grnd(3) ? E_BEETLE : E_FROG;   // the first levels keep it gentle
  add_ent(t, x, GY - 1);
}
static void foe_a(int x) {   // a creature in the air above column x
  int t = cave_theme() ? E_BAT : POOL_A[g_world][grnd(3)];
  if (g_n < 2) t = E_BEE;
  add_ent(t, x, t == E_BIRD || t == E_BAT ? GY - 7 : GY - 4);
}
static int power_kind(void) {   // which power-up a box holds: wings, sword, helmet or star
  static const char P[7][4] = { { 1, 2, 1, 2 }, { 1, 2, 3, 1 }, { 2, 3, 1, 2 }, { 3, 4, 2, 1 }, { 4, 2, 3, 1 }, { 4, 3, 2, 1 }, { 4, 1, 3, 2 } };
  return P[g_world][grnd(4)];
}
static void ground_step(int d) { GY += d; if (GY < g_top) GY = g_top; if (GY > g_bot) GY = g_bot; }

// ---- a key piece, hidden one of a few ways
static void key_piece(int piece) {
  int v = grnd(L->theme == 9 ? 2 : 6);
  if (L->theme == 9) {
    if (v == 0) {   // a rock with a hollow in it, open at the top
      flat(3);
      int x = GX;
      for (int i = 0; i < 5; i++) colm(x + i, GY);
      for (int y = GY - 5; y < GY; y++) for (int i = 0; i < 5; i++) SET(x + i, y, 'X');
      for (int y = GY - 4; y < GY; y++) for (int i = 1; i < 4; i++) SET(x + i, y, ' ');
      SET(x + 2, GY - 5, ' ');
      key_at(x + 2, GY - 1, piece); coin(x + 1, GY - 1); coin(x + 3, GY - 1);
      add_ent(E_JELLY, x + 2, GY - 9);
      GX += 5; flat(3);
    } else {        // high up between two pillars of rock, past a fish
      flat(3);
      int x = GX;
      for (int i = 0; i < 9; i++) colm(x + i, GY);
      for (int y = GY - 9; y < GY; y++) { SET(x + 1, y, 'X'); SET(x + 7, y, 'X'); }
      key_at(x + 4, GY - 10, piece); coin_row(x + 3, GY - 8, 3);
      add_ent(E_FISH, x + 4, GY - 4);
      GX += 9; flat(2);
    }
    return;
  }
  if (v == 0 && GY - 9 >= 1) {   // up high: bounce up off a spring to a plank
    flat(9);
    SET(GX - 7, GY - 1, 's');
    for (int i = -6; i <= -3; i++) SET(GX + i, GY - 7, '=');
    key_at(GX - 4, GY - 8, piece); coin(GX - 6, GY - 8); coin(GX - 3, GY - 8);
  } else if (v == 1) {           // inside a hill, through a pretend wall (coins lead the way in)
    flat(3);
    coin(GX - 2, GY - 1); coin(GX - 1, GY - 1);
    int x = GX;
    for (int i = 0; i < 9; i++) colm(x + i, GY - 2);
    for (int i = 1; i < 8; i++) SET(x + i, GY - 1, ' ');
    SET(x, GY - 1, GC); FAKE[GY - 1][x] = 1;
    coin_row(x + 1, GY - 1, 5); key_at(x + 7, GY - 1, piece);
    GX += 9; flat(3);
  } else if (v == 2) {           // in mid-air over a pit: grab it on the way across
    flat(3);
    key_at(GX + 2, GY - 2, piece); coin(GX + 1, GY - 2); coin(GX + 3, GY - 2);
    GX += 4; flat(3);
  } else if (v == 3) {           // on top of a stone tower
    flat(2);
    int x = GX;
    static const int H[7] = { 1, 2, 3, 4, 3, 2, 1 };
    for (int i = 0; i < 7; i++) { colm(x + i, GY); for (int j = 1; j <= H[i]; j++) SET(x + i, GY - j, 'X'); }
    key_at(x + 3, GY - 5, piece);
    foe_g(x + 9);
    GX += 7; flat(4);
  } else if (v == 4 && GY + 4 < LH) {   // at the bottom of a pool
    flat(3);
    int x = GX, w = 7;
    for (int i = 0; i < w; i++) { colm(x + i, GY + 3); for (int y = GY; y < GY + 3; y++) SET(x + i, y, 'w'); }
    key_at(x + 3, GY + 2, piece); coin(x + 1, GY + 2); coin(x + 5, GY + 2);
    add_ent(E_FISH, x + 3, GY + 1);
    GX += w; flat(3);
  } else {                       // in a hut of cracked blocks, with a helmet box just before it
    flat(4);
    box(GX - 2, GY - 4, 3);
    flat(8);
    int x = GX - 5;
    SET(x, GY - 1, 'k'); SET(x + 2, GY - 1, 'k'); SET(x, GY - 2, 'k'); SET(x + 1, GY - 2, 'k'); SET(x + 2, GY - 2, 'k');
    key_at(x + 1, GY - 1, piece);
  }
}

// ---- the set pieces of a land level
enum { P_RUN, P_GAP, P_STEP, P_BRIDGE, P_SPIKES, P_BOXES, P_STONE, P_HIGH, P_ICE, P_ZIGZAG, P_MOVE, P_LIFT, P_FALL, P_PILLARS,
       P_CHIMNEY, P_TUNNEL, P_POOL, P_SECRET, P_HELMET, P_WAVE, P_BIRDS, P_PYRAMID, P_ICICLES, P_CHAIN, P_STAR, NPIECE };
static int piece_weight(int k) {
  int th = L->theme, sky = th == 3, cave = cave_theme(), sn = snow_theme(), sand = th == 8 || th == 10, wet = th == 0 || th == 4 || th == 10 || g_world == 1 || g_world == 4;
  switch (k) {
    case P_RUN: return sky ? 1 : 4;
    case P_GAP: return 4;
    case P_STEP: return 3;
    case P_BRIDGE: return sky ? 4 : 2;
    case P_SPIKES: return 2;
    case P_BOXES: return 3;
    case P_STONE: return cave ? 3 : 1;
    case P_HIGH: return 2;
    case P_ICE: return sn ? 4 : 0;
    case P_ZIGZAG: return g_n >= 1 ? (sky ? 3 : 1) : 0;
    case P_MOVE: return g_n >= 1 ? (sky ? 3 : 2) : 0;
    case P_LIFT: return g_n >= 2 ? (sky ? 2 : 1) : 0;
    case P_FALL: return g_n >= 1 ? (sky ? 3 : 2) : 0;
    case P_PILLARS: return g_n >= 2 ? (sky ? 4 : 2) : 0;
    case P_CHIMNEY: return g_n >= 1 ? 1 : 0;
    case P_TUNNEL: return cave ? 4 : sky ? 0 : 1;
    case P_POOL: return wet ? 3 : (sn ? 1 : 0);
    case P_SECRET: return 2;
    case P_HELMET: return g_n >= 24 ? 1 : 0;
    case P_WAVE: return 1 + (int)(g_diff * 3);
    case P_BIRDS: return g_world >= 1 && !cave ? 2 : 0;
    case P_PYRAMID: return sand || th == 1 ? 3 : 0;
    case P_ICICLES: return sn ? 3 : 0;
    case P_CHAIN: return g_n >= 8 ? (sky ? 3 : 1) : 0;
    case P_STAR: return g_n >= 16 ? 1 : 0;
  }
  return 0;
}
static void land_piece(int k) {
  int x;
  switch (k) {
    case P_RUN: {   // a stretch of ground with coins and a creature or two
      int w = 10 + grnd(6); x = GX; flat(w);
      coin_arc(x + 3, GY - 3, 5);
      foe_g(x + w - 3);
      if (grnd(100) < 30 + (int)(g_diff * 50)) foe_g(x + 5);
      break;
    }
    case P_GAP: {   // a gap to jump, maybe up or down a step
      int gw = 2 + grnd(2) + (g_diff > 0.35f && grnd(2));
      flat(3); coin_arc(GX - 1, GY - 3, gw + 2); GX += gw;
      if (grnd(3) == 0) ground_step(grnd(2) ? 1 : -1);
      flat(3); break;
    }
    case P_STEP: {
      flat(4);
      int d = (1 + grnd(2)) * (GY > (g_top + g_bot) / 2 ? -1 : 1);
      if (grnd(4) == 0) d = -d;
      ground_step(d); x = GX; flat(5);
      coin(x + 2, GY - 2); coin(x + 3, GY - 2);
      break;
    }
    case P_BRIDGE: {   // a log bridge over a pit, with something flying over it
      flat(2);
      int w = 8 + grnd(5);
      for (int i = 0; i < w; i++) { SET(GX + i, GY, '='); if (i % 2) coin(GX + i, GY - 2); }
      foe_a(GX + w / 2);
      GX += w; flat(2); break;
    }
    case P_SPIKES: {
      flat(4);
      int sw = 2;   // three wide is only just jumpable: keep it fair
      for (int i = 0; i < sw; i++) { colm(GX + i, GY); SET(GX + i, GY - 1, '^'); coin(GX + i, GY - 4); }
      GX += sw; flat(4); break;
    }
    case P_BOXES: {   // a row of ? boxes; one might hold a power-up
      x = GX; flat(10);
      for (int i = 3; i < 6; i++) box(x + i, GY - 4, 0);
      if (grnd(3) == 0) BOXK[GY - 4][x + 4] = (char)power_kind();
      foe_g(x + 8);
      break;
    }
    case P_STONE: {   // stone steps up and over
      flat(3); x = GX;
      for (int i = 0; i < 6; i++) { int h = i < 2 ? 1 : (i < 4 ? 2 : 1); colm(x + i, GY); for (int j = 1; j <= h; j++) SET(x + i, GY - j, 'X'); if (i == 2 || i == 3) coin(x + i, GY - 4); }
      GX += 6; flat(3); break;
    }
    case P_HIGH: {   // a spring up to a high road of planks, with creatures down below
      if (GY - 8 < 1) { flat(6); break; }
      x = GX; flat(20);
      SET(x + 2, GY - 1, 's');
      for (int i = 4; i < 17; i++) { SET(x + i, GY - 6, '='); coin(x + i, GY - 7); }
      foe_g(x + 9); foe_g(x + 14);
      if (grnd(2)) box(x + 10, GY - 10, power_kind());
      break;
    }
    case P_ICE: {   // a slippery stretch of ice
      int w = 10 + grnd(5); x = GX;
      for (int i = 0; i < w; i++) { colm(GX + i, GY); SET(GX + i, GY, 'I'); }
      GX += w; coin_row(x + 3, GY - 2, 4);
      foe_g(x + w - 2);
      break;
    }
    case P_ZIGZAG: {   // zigzag planks over a pit
      flat(2);
      for (int j = 0; j < 4; j++) { int yy = GY - 1 - (j % 2) * 2; for (int i = 0; i < 4; i++) SET(GX + j * 4 + i, yy, '='); coin(GX + j * 4 + 1, yy - 2); }
      foe_a(GX + 8);
      GX += 16; flat(2); break;
    }
    case P_MOVE: {   // a moving platform across a wide pit
      flat(2);
      plat_at(GX, GY, 0, 6);
      coin_row(GX + 2, GY - 3, 5);
      GX += 9; flat(2); break;
    }
    case P_LIFT: {   // a lift up to higher ground
      if (GY - 5 < g_top) { ground_step(3); flat(4); break; }
      flat(3);
      plat_at(GX, GY, 1, 5);
      coin(GX + 1, GY - 7); coin(GX + 1, GY - 9);
      GX += 3; GY -= 5; x = GX; flat(6);
      coin_row(x + 1, GY - 2, 3);
      break;
    }
    case P_FALL: {   // crumbly platforms over a pit: keep moving!
      flat(2);
      int n = 3 + grnd(2);
      for (int k2 = 0; k2 < n; k2++) { plat_at(GX + k2 * 4, GY, 2, 0); coin(GX + k2 * 4 + 1, GY - 2); }
      GX += n * 4 - 1; flat(3); break;
    }
    case P_PILLARS: {   // hop from pillar to pillar
      flat(2);
      int n = 3 + grnd(2);
      for (int k2 = 0; k2 < n; k2++) {
        GX += 2 + grnd(2);
        int top = GY - grnd(3); if (top < g_top) top = g_top;
        colm(GX, top); colm(GX + 1, top); coin(GX, top - 1); coin(GX + 1, top - 1);
        GX += 2;
      }
      GX += 2; flat(3); break;
    }
    case P_CHIMNEY: {   // a narrow shaft to wall-jump up
      if (GY - 6 < g_top) { flat(5); break; }
      flat(4);
      x = GX;
      colm(x, GY); colm(x + 1, GY); colm(x + 2, GY);
      for (int y = GY - 7; y <= GY - 3; y++) SET(x, y, 'X');   // a stone block hanging over the way in
      coin(x + 1, GY - 4); coin(x + 2, GY - 6);
      GX = x + 3; GY -= 6; x = GX; flat(6);
      coin_row(x + 1, GY - 2, 3);
      break;
    }
    case P_TUNNEL: {   // a low tunnel with boxes in its roof
      int w = 14 + grnd(4); x = GX; flat(w);
      for (int i = 2; i < w - 2; i++) SET(x + i, GY - 4, 'X');
      box(x + 5, GY - 4, 0); box(x + 9, GY - 4, grnd(3) ? 0 : power_kind());
      coin_row(x + 3, GY - 2, w - 6);
      foe_g(x + 7); if (g_diff > 0.3f) foe_g(x + 11);
      break;
    }
    case P_POOL: {   // a pool to swim across
      if (GY + 4 >= LH) { flat(4); break; }
      flat(3); x = GX;
      int w = 6 + grnd(5);
      for (int i = 0; i < w; i++) { colm(x + i, GY + 3); for (int y = GY; y < GY + 3; y++) SET(x + i, y, 'w'); }
      coin_row(x + 1, GY + 2, w - 2);
      if (w >= 7) add_ent(E_FISH, x + w / 2, GY + 1);
      GX += w; flat(3); break;
    }
    case P_SECRET: {   // a hill with a room hidden behind a pretend wall
      flat(3); x = GX;
      for (int i = 0; i < 10; i++) colm(x + i, GY - 2);
      for (int i = 1; i < 9; i++) SET(x + i, GY - 1, ' ');
      SET(x, GY - 1, GC); FAKE[GY - 1][x] = 1;
      coin_row(x + 1, GY - 1, 8);
      GX += 10; flat(3);
      if (grnd(2)) box(x + 4, GY - 6, power_kind());
      break;
    }
    case P_HELMET: {   // a wall of cracked blocks: only the helmet gets through. The box refills.
      flat(4); box(GX - 2, GY - 4, 3);
      flat(2); x = GX;
      for (int y = GY - 5; y < GY; y++) { SET(x, y, 'k'); SET(x + 1, y, 'k'); }
      for (int y = 0; y < GY - 5; y++) { SET(x, y, 'X'); SET(x + 1, y, 'X'); }
      colm(x, GY); colm(x + 1, GY);
      GX += 2; x = GX; flat(6); coin_row(x + 1, GY - 2, 4);
      break;
    }
    case P_WAVE: {   // lots of creatures at once, with boxes overhead
      x = GX; flat(17);
      for (int i = 0; i < 3 + (g_diff > 0.5f); i++) foe_g(x + 5 + i * 3);
      foe_a(x + 9);
      box(x + 3, GY - 4, 0); box(x + 12, GY - 4, 0);
      break;
    }
    case P_BIRDS: {   // a long bridge with birds swooping
      flat(2);
      int w = 14;
      for (int i = 0; i < w; i++) { SET(GX + i, GY, '='); if (i % 3 == 1) coin(GX + i, GY - 2); }
      add_ent(E_BIRD, GX + 4, GY - 8); add_ent(E_BIRD, GX + 11, GY - 8);
      GX += w; flat(2); break;
    }
    case P_PYRAMID: {   // stone steps up and down
      flat(2); x = GX;
      static const int H[9] = { 1, 2, 3, 4, 4, 3, 2, 1, 0 };
      for (int i = 0; i < 9; i++) { colm(x + i, GY); for (int j = 1; j <= H[i]; j++) SET(x + i, GY - j, 'X'); }
      coin_row(x + 3, GY - 6, 2);
      if (grnd(2)) box(x + 4, GY - 8, 0);
      GX += 9; flat(3);
      foe_g(GX - 2);
      break;
    }
    case P_ICICLES: {   // icicles hanging under an ice ledge
      x = GX; flat(12);
      for (int i = 3; i < 9; i++) SET(x + i, GY - 5, 'I');
      for (int i = 4; i < 9; i += 2) { Ent* e = add_ent(E_ICE, x + i, GY - 4); if (e) { e->y = e->by = (GY - 4) * T; e->vx = 0; e->t = 0; } }
      coin_row(x + 5, GY - 2, 3);
      break;
    }
    case P_CHAIN: {   // two moving platforms over a very wide pit
      flat(2); x = GX;
      plat_at(x, GY, 0, 5);
      Ent* e = plat_at(x + 8, GY - 2, 0, 4); if (e) e->t = 126;
      coin_row(x + 5, GY - 4, 4);
      GX = x + 15; flat(3); break;
    }
    case P_STAR: {   // a star box, then a crowd to throw stars at
      x = GX; flat(16);
      box(x + 2, GY - 4, 4);
      foe_g(x + 8); foe_g(x + 11); foe_g(x + 14);
      break;
    }
  }
}

// ---- the set pieces of a water level
static void water_piece(int k) {
  int x = GX;
  switch (k) {
    case 0: {   // open water with a ring of coins and fish
      int w = 14 + grnd(6);
      for (int i = 0; i < w; i++) colm(GX + i, GY + (i % 7 == 3));
      GX += w;
      int cx = x + w / 2, cy = GY - 7;
      for (int a = 0; a < 8; a++) coin(cx + (int)lroundf(cosf(a * 0.785f) * 2.2f), cy + (int)lroundf(sinf(a * 0.785f) * 2.2f));
      add_ent(E_FISH, x + 3, GY - 4); add_ent(E_FISH, x + w - 3, GY - 10);
      break;
    }
    case 1: {   // rock columns from the floor and from the top: swim over, then under
      flat(3);
      for (int k2 = 0; k2 < 3; k2++) {
        int a = GX;
        if (k2 % 2 == 0) { colm(a, GY - 8); colm(a + 1, GY - 8); for (int y = GY - 8; y < GY; y++) { SET(a, y, 'X'); SET(a + 1, y, 'X'); } }
        else { colm(a, GY); colm(a + 1, GY); for (int y = 0; y < GY - 8; y++) { SET(a, y, 'X'); SET(a + 1, y, 'X'); } }
        GX += 2; flat(4);
        coin_row(GX - 3, k2 % 2 ? GY - 4 : GY - 10, 2);
      }
      break;
    }
    case 2: {   // a narrow passage with jellyfish
      int w = 16;
      for (int i = 0; i < w; i++) { colm(GX + i, GY - 2); for (int y = 0; y < GY - 9; y++) SET(GX + i, y, 'X'); }
      GX += w;
      add_ent(E_JELLY, x + 5, GY - 6); add_ent(E_JELLY, x + 11, GY - 5);
      coin_row(x + 2, GY - 4, 12);
      break;
    }
    case 3: {   // a trench with spikes on the bottom
      if (GY + 3 >= LH) { flat(6); break; }
      flat(3); x = GX;
      for (int i = 0; i < 6; i++) { colm(x + i, GY + 2); SET(x + i, GY + 1, '^'); }
      coin_row(x, GY - 3, 6);
      GX += 6; flat(3);
      break;
    }
    case 4: {   // crabs on the sea floor and a shelf with boxes over it
      flat(14);
      add_ent(E_CRAB, x + 3, GY - 1); add_ent(E_CRAB, x + 11, GY - 1);
      for (int i = 4; i < 9; i++) SET(x + i, GY - 5, 'X');
      box(x + 5, GY - 9, 0); box(x + 7, GY - 9, grnd(2) ? 0 : 2);
      break;
    }
    case 5: {   // a school of fish
      int w = 16;
      for (int i = 0; i < w; i++) colm(GX + i, GY);
      GX += w;
      for (int f = 0; f < 3; f++) add_ent(E_FISH, x + 3 + f * 5, GY - 3 - f * 3);
      for (int i = 0; i < 5; i++) coin(x + 2 + i * 3, GY - 2 - (i % 3) * 3);
      break;
    }
    case 6: {   // the sea floor rises or drops
      flat(4);
      int d = (2 + grnd(2)) * (GY > (g_top + g_bot) / 2 ? -1 : 1);
      ground_step(d); x = GX; flat(6); coin_row(x + 1, GY - 2, 4);
      if (grnd(2)) add_ent(E_JELLY, x + 3, GY - 7);
      break;
    }
    default: {  // a rock arch with coins underneath
      int w = 12;
      for (int i = 0; i < w; i++) colm(GX + i, GY);
      for (int i = 2; i < w - 2; i++) SET(GX + i, GY - 6, 'X');
      GX += w;
      coin_row(x + 3, GY - 3, 6);
      add_ent(E_CRAB, x + 6, GY - 1);
      break;
    }
  }
}

static void boss_arena(int bk) {
  int x = GX;
  for (int i = 0; i < 38; i++) colm(x + i, GY);
  SET(x + 2, GY - 1, 'c'); box(x + 5, GY - 4, 2);
  arena_x0 = (x + 9) * T; arena_x1 = (x + 34) * T;
  Ent* e = add_ent(E_BOSS, x + 26, GY - 1);
  if (e) {
    boss_i = (int)(e - ents); e->k = bk;
    e->x = e->x0 = (x + 26) * T; e->y = e->by = BOSS_BEE(bk) ? GY * T - 84 : GY * T - 26;
    boss_max = boss_hp = bk >= 5 ? 6 : bk >= 3 ? bk + 2 : 3 + bk;
  }
  GX += 38;
}

static void build_level(int n) {
  boss_i = -1; boss_awake = boss_dead = 0; g_boss_music = 0;
  int water = L->theme == 9, bk = boss_kind(n);
  g_world = n / 8; g_n = n; g_diff = n / 55.0f;
  LH = water ? 20 : 20;
  g_top = water ? 9 : 7; g_bot = water ? 17 : 16;
  GC = cave_theme() ? 'X' : '#';
  gen_seed = 2654435761u ^ (uint32_t)(n * 7919 + 17);
  GX = 0; GY = water ? 16 : 14;
  // the start, with any signs
  int start_w = 10 + (L->signs[0] ? 6 : 0) + (L->signs[1] ? 5 : 0) + (L->signs[2] ? 5 : 0);
  flat(start_w);
  start_x = 3 * T + 3; start_y = (GY - 1) * T + T - PH;
  for (int i = 0; i < 3 && L->signs[i]; i++) { sign_x[i] = 7 + i * 5; sign_y[i] = GY - 1; nsigns = i + 1; }
  int len = bk >= 0 ? 300 : water ? 380 + (int)(g_diff * 50) : 470 + (int)(g_diff * 70);
  level_keys = bk < 0;
  int last_cp = GX, keys_done = 0, last = -1, last2 = -1;
  while (GX < len) {
    if (GX - last_cp > 80) {   // a checkpoint every so often
      int x = GX; flat(6); SET(x + 2, GY - 1, 'c'); last_cp = GX;
      continue;
    }
    if (level_keys && keys_done < 3 && GX > len * (keys_done * 0.27f + 0.2f)) { key_piece(keys_done++); continue; }
    if (water) { int k = grnd(8); if (k == last) k = (k + 3) % 8; last = k; water_piece(k); continue; }
    int tot = 0, k = 0;
    for (int i = 0; i < NPIECE; i++) if (i != last && i != last2) tot += piece_weight(i);
    int r = grnd(tot > 0 ? tot : 1);
    for (k = 0; k < NPIECE; k++) { if (k == last || k == last2) continue; r -= piece_weight(k); if (r < 0) break; }
    if (k >= NPIECE) k = P_RUN;
    last2 = last; last = k;
    land_piece(k);
  }
  // the end: bring the ground back to a comfortable height
  while (GY < (water ? 15 : 13)) { flat(3); ground_step(1); }
  while (GY > (water ? 17 : 15)) { flat(3); ground_step(-1); }
  flat(4);
  if (bk >= 0) boss_arena(bk);
  else {   // the locked door in a wall that goes all the way up
    int x = GX;
    for (int y = 0; y < GY - 3; y++) SET(x, y, 'X');
    for (int y = GY - 3; y < GY; y++) SET(x, y, 'D');
    colm(x, GY); GX++;
  }
  int x = GX; flat(12);
  goal_x = x + 6; goal_y = GY - 1;
  LW = GX;
}

static void load_level(int n) {
  lvl = n; L = &LEVELS[n];
  memset(G, ' ', sizeof G); memset(BOXK, 0, sizeof BOXK); memset(FAKE, 0, sizeof FAKE); ride_i = -1;
  nents = 0; nsigns = 0; ncheck = 0;
  memset(parts, 0, sizeof parts); memset(bubbles, 0, sizeof bubbles);
  keyp = 0; door_msg_t = key_msg_t = 0;
  build_level(n);
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
    if (e->type == E_PLAT) { if (e->k == 2) { e->alive = 1; e->y = e->by; e->vy = 0; e->fz = 0; } continue; }
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

static void open_door(int tx) {
  for (int ty = 0; ty < LH; ty++) if (G[ty][tx] == 'D') { G[ty][tx] = 'd'; spawn_parts(tx * T + 8, ty * T + 8, 8, 0xFFFFD84A, 1.8f, 0.4f); }
  plat_sfx(SFX_LIFE); plat_sfx(SFX_CHECK);
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
  {   // is Dino in water? (water levels are water all through)
    int cx = (int)floorf((px + PW / 2) / T), cy = (int)floorf((py + PH / 2) / T);
    in_water = L->theme == 9 || tile(cx, cy) == 'w';
  }
  // sprinting: hold a back trigger and Dino speeds up bit by bit, reaching top speed after 1.5 seconds
  if (in->run && dir && !in_water) { if (run_ramp < RUN_RAMP) run_ramp++; }
  else if (on_ground || in_water) { run_ramp -= 3; if (run_ramp < 0) run_ramp = 0; }
  float acc = on_ice ? 0.07f : on_ground ? 0.3f : 0.2f, maxv = WALK_V + (RUN_V - WALK_V) * run_ramp / (float)RUN_RAMP;
  if (in_water) { acc = on_ground ? 0.2f : 0.13f; maxv = 1.5f; }
  if (atk_t > 6 && power != 2) maxv = 4.2f;
  // turning round while moving: snap straight round, keeping the speed
  if (dir && on_ground && !on_ice && !in_water && pvx * dir < -0.4f) { pvx = -pvx; snap_turn = 1; }
  if (dir) { pvx += dir * acc; face = dir; if (pvx > maxv) pvx = maxv; if (pvx < -maxv) pvx = -maxv; }
  else if (wall_lock <= 0) { float f = in_water ? 0.05f : on_ice ? 0.02f : on_ground ? 0.3f : 0.08f; if (pvx > f) pvx -= f; else if (pvx < -f) pvx += f; else pvx = 0; }

  if (jp) jbuf = 7; else if (jbuf > 0) jbuf--;
  if (on_ground) { coyote = 6; air_jumps = 1; twirl_used = 0; twirl_t = 0; } else if (coyote > 0) coyote--;
  // the twirl: a fresh press of L or R in the air spins Dino round and holds him up for half a second, once per jump
  if (in->run && !prev_run && !on_ground && !in_water && !twirl_used && !atk_t) { twirl_t = TWIRL_T; twirl_used = 1; if (pvy > -1) pvy = -1.0f; plat_sfx(SFX_JUMP2); spawn_parts(px + PW / 2, py + PH / 2, 8, 0xFFFFFFFF, 1.4f, 0.0f); }
  prev_run = in->run;
  // the slot: drop the kept power-up from the top of the screen
  if (in->pop && !prev_pop && slot && nents < MAXENT) {
    Ent* e = &ents[nents++]; memset(e, 0, sizeof *e);
    e->type = E_POW; e->alive = 1; e->k = slot; e->vx = 0; e->t = 17; e->x = px; e->y = e->by = camy - 14; slot = 0;
    plat_sfx(SFX_CHECK);
  }
  prev_pop = in->pop;

  // walls next to the player
  wall_dir = 0;
  if (!on_ground && !in_water) {
    if (box_solid(px + PW, py + 2, 1, PH - 4)) wall_dir = 1;
    else if (box_solid(px - 1, py + 2, 1, PH - 4)) wall_dir = -1;
  }

  if (in_water) {
    // swimming: each press of jump is a stroke up; at the top of a pool it leaps out
    if (jp) {
      int surf = L->theme != 9 && tile((int)floorf((px + PW / 2) / T), (int)floorf((py - 4) / T)) != 'w';
      pvy = surf ? -5.2f : -2.5f; jbuf = 0; coyote = 0; on_ground = 0;
      plat_sfx(SFX_JUMP2);
      add_bubble(px + PW / 2 + face * 4, py + 2); add_bubble(px + PW / 2, py + 6);
    }
  } else if (jbuf && coyote) {
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
  float g = ((in->jump || spring_up) && pvy < 0) ? G_HOLD : 0.55f;
  if (in_water) { g = in->down ? 0.16f : 0.085f; if (pvy < -2.6f && !(pvy < -4)) pvy += 0.06f; }
  pvy += g;
  float maxfall = in_water ? (in->down ? 2.4f : 1.25f) : 5.5f;
  if (in_water && frame % 47 == 0) add_bubble(px + PW / 2 + face * 3, py + 3);
  if (wall_dir && dir == wall_dir && pvy > 0) { maxfall = 1.1f; if (frame % 6 == 0) spawn_parts(px + (wall_dir > 0 ? PW : 0), py + 4, 1, 0xFFE8E0C8, 0.4f, 0); }
  if (twirl_t > 0) { twirl_t--; if (pvy > 0.25f) pvy = 0.25f; }   // twirling: hardly falls at all
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
  ride_i = -1;   // let go of the platform every tick, so a jump really leaves it
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
  if (L->theme == 9 && py < 3) { py = 3; if (pvy < 0) pvy = 0; }   // the top of a water level
  if (on_ground && !was_ground) { land_t = 6; spawn_parts(px + PW / 2, py + PH, 3, 0xFFE8E0C8, 0.6f, 0.2f); }
  if (land_t) land_t--;

  // tiles the player touches
  int x0 = (int)floorf(px / T), x1 = (int)floorf((px + PW - 0.01f) / T);
  int y0 = (int)floorf(py / T), y1 = (int)floorf((py + PH - 0.01f) / T);
  for (int ty = y0; ty <= y1; ty++) for (int tx = x0; tx <= x1; tx++) {
    char c = tile(tx, ty);
    if (c == 'o') collect(tx, ty);
    else if (c == 'K') {
      G[ty][tx] = ' '; keyp |= 1 << BOXK[ty][tx];
      plat_sfx(SFX_LIFE); spawn_parts(tx * T + 8, ty * T + 8, 16, 0xFFFFD84A, 2.2f, 0.6f);
      if (keyp == 7) key_msg_t = 150;
    }
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
  // the locked door: it opens when Dino walks up to it holding the whole key
  {
    int ty0 = (int)floorf((py + 1) / T), ty1 = (int)floorf((py + PH - 1) / T);
    for (int side = -1; side <= 1; side += 2) {
      int tx = (int)floorf((side > 0 ? px + PW + 1.5f : px - 1.5f) / T);
      for (int ty = ty0; ty <= ty1; ty++) if (tile(tx, ty) == 'D') {
        if (keyp == 7) open_door(tx);
        else if (!door_msg_t) { door_msg_t = 150; plat_sfx(SFX_WALL); }
      }
    }
  }
  if (door_msg_t) door_msg_t--;
  if (key_msg_t) key_msg_t--;
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

// walk along the ground at spd, turning round at walls and at the edge of a drop
static void walker_step(Ent* e, float spd, float ew, float eh) {
  float v = e->vx > 0 ? spd : -spd, nx = e->x + v;
  int ahead = v < 0 ? (int)floorf(nx / T) : (int)floorf((nx + ew) / T);
  int below = (int)floorf((e->y + eh + 1) / T);
  char fl = tile(ahead, below);
  if (box_solid(nx, e->y, ew, eh) || !(solid_c(fl) || fl == '=')) e->vx = -e->vx; else e->x = nx;
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
        if (power && power != e->k) slot = power;   // the one he had goes into the slot
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
      if (e->k == 2) {   // a crumbly platform: shakes once Dino stands on it, then drops
        if (e->fz == 0 && ride_i == i) e->fz = 1;
        if (e->fz > 0) { e->fz++; if (e->fz > 34) { e->vy += 0.2f; if (e->vy > 4.5f) e->vy = 4.5f; e->y += e->vy; } }
        if (e->y > LH * T + 60) { e->alive = 0; if (ride_i == i) ride_i = -1; continue; }
      } else if (e->k) e->y = e->by - e->vx * s2; else e->x = e->x0 + e->vx * s2;
      if (ride_i == i && !dead_t && state == ST_PLAY) {   // carry Dino along
        if (!box_solid(px + e->x - ox, py, PW, PH)) px += e->x - ox;
        if (!box_solid(px, e->y - PH, PW, PH)) py = e->y - PH;
      }
      continue;
    }
    if (e->type == E_SHOT) {
      e->x += e->vx; if (e->k == 0) e->y += e->vy;
      if (e->k >= 2) { e->vy += 0.14f; if (e->vy > 4) e->vy = 4; e->y += e->vy; }
      int gone = e->t > 120 || box_solid(e->x, e->y + 1, 6, 4) || e->y > LH * T + 20;
      if (e->k == 1 && !gone) {   // Dino's star bounces along the ground
        e->vy += 0.25f; e->y += e->vy;
        if (box_solid(e->x, e->y, 6, 6)) { e->y -= e->vy; e->vy = e->vy > 0 ? -2.8f : 0.5f; }
        if (frame % 3 == 0) spawn_parts(e->x + 3, e->y + 3, 1, 0xFFFFF2A0, 0.4f, 0);
        for (int j = 0; j < nents && !gone; j++) {
          Ent* o = &ents[j];
          if (!o->alive || o->dying) continue;
          if (IS_FOE(o->type) && !(o->type == E_MOLE && o->k == 0) && e->x + 6 > o->x && e->x < o->x + 12 && e->y + 6 > o->y && e->y < o->y + 14) { o->dying = 1; gone = 1; plat_sfx(SFX_STOMP); spawn_parts(o->x + 6, o->y + 4, 10, 0xFFFFE27A, 2.0f, 0.5f); }
          else if (o->type == E_BOSS && boss_awake && e->x + 6 > o->x && e->x < o->x + 30 && e->y + 6 > o->y && e->y < o->y + 24) { if (!boss_inv) boss_damage(o); gone = 1; }
        }
      } else if (e->k != 1 && !gone && !dead_t && state == ST_PLAY) {
        float ax, ay, aw, ah;
        if (attack_box(&ax, &ay, &aw, &ah) && ax + aw > e->x && ax < e->x + 6 && ay + ah > e->y && ay < e->y + 6) gone = 1;
        else if (px + PW > e->x + 1 && px < e->x + 5 && py + PH > e->y + 1 && py < e->y + 5) { hurt(); gone = 1; }
      }
      if (gone) { e->alive = 0; spawn_parts(e->x + 3, e->y + 3, 5, e->k == 1 ? 0xFFFFF2A0 : e->k == 3 ? 0xFFFFFFFF : 0xFF6A4A2A, 1.2f, 0.2f); }
      continue;
    }
    if (e->type == E_PLANT || e->type == E_SNOWMAN) eh = 14;
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
    } else if (e->type == E_CRAB) {
      // scuttles in quick bursts, then stops to snap its claws and turns to face Dino
      int ph = e->t % 110;
      if (ph < 64) walker_step(e, 1.15f, ew, eh);
      else if (ph == 64 && fabsf(px - e->x) < 160) e->vx = px > e->x ? 0.5f : -0.5f;
    } else if (e->type == E_FISH) {
      // swims to and fro in a gentle wave; darts at Dino when he comes close in front of it
      float dx = px - e->x, dy = py - e->y;
      if (e->k == 0) {
        float nx = e->x + (e->vx > 0 ? 0.7f : -0.7f);
        if (box_solid(nx, e->y, ew, eh) || fabsf(nx - e->x0) > 72) e->vx = -e->vx; else e->x = nx;
        float ny = e->by + sinf(e->t * 0.05f) * 7;
        if (!box_solid(e->x, ny, ew, eh)) e->y = ny;
        if (!dead_t && fabsf(dx) < 96 && fabsf(dy) < 28 && (dx > 0) == (e->vx > 0) && e->t > 40) { e->k = 1; e->vy = 46; }
      } else {
        float nx = e->x + (e->vx > 0 ? 2.1f : -2.1f);
        if (box_solid(nx, e->y, ew, eh) || --e->vy <= 0) { e->k = 0; e->t = 0; e->x0 = e->x; } else e->x = nx;
      }
    } else if (e->type == E_JELLY) {
      // pulses upward, then sinks slowly, drifting towards Dino
      int c = e->t % 110;
      float vy = c < 22 ? -0.85f : 0.3f, vx = px > e->x ? 0.22f : -0.22f;
      if (!box_solid(e->x + vx, e->y, ew, eh) && fabsf(e->x + vx - e->x0) < 90) e->x += vx;
      if (!box_solid(e->x, e->y + vy, ew, eh) && e->y + vy > 4 && e->y + vy < e->by + 40 && e->y + vy > e->by - 60) e->y += vy;
      e->vx = vx;
    } else if (e->type == E_ROLL) {
      // ambles along; when it spots Dino it curls into a ball and comes rolling at him
      if (e->k == 0) {
        walker_step(e, 0.35f, ew, eh);
        if (!dead_t && e->t > 70 && fabsf(px - e->x) < 130 && fabsf(py - e->y) < 30) { e->k = 1; e->t = 0; e->vx = px > e->x ? 0.5f : -0.5f; }
      } else if (e->k == 1) { if (e->t > 26) { e->k = 2; e->t = 0; } }
      else { walker_step(e, 2.5f, ew, eh); if (e->t > 170) { e->k = 0; e->t = 0; } }
    } else if (e->type == E_PENGUIN) {
      // waddles about, then flops on its belly and slides at Dino
      if (e->k == 0) {
        walker_step(e, 0.4f, ew, eh);
        if (!dead_t && e->t > 100 && fabsf(px - e->x) < 150 && fabsf(py - e->y) < 30) { e->k = 1; e->t = 0; e->vx = px > e->x ? 0.5f : -0.5f; }
      } else { if (e->t < 85) walker_step(e, 2.3f, ew, eh); if (e->t > 110) { e->k = 0; e->t = 0; } }
    } else if (e->type == E_MOLE) {
      // waits down its hole; pops up near Dino, throws a clod of earth at him and ducks back down
      float dx = px - e->x;
      e->vx = dx > 0 ? 0.5f : -0.5f;
      if (e->k == 0) { if (!dead_t && e->t > 80 && fabsf(dx) < 140 && fabsf(dx) > 18 && fabsf(py - e->y) < 80) { e->k = 1; e->t = 0; } }
      else {
        if (e->t == 32 && !dead_t) { float vx = clampf(dx / 46, -2.4f, 2.4f); add_shot(e->x + 3, e->y - 6, vx, -3.3f, 2); plat_sfx(SFX_WALL); }
        if (e->t > 76) { e->k = 0; e->t = 0; }
      }
    } else if (e->type == E_BIRD) {
      // glides to and fro up high; dives at Dino when he passes underneath, then flies back up
      if (e->k == 0) {
        e->x += e->vx > 0 ? 0.8f : -0.8f;
        if (fabsf(e->x - e->x0) > 70) { e->x -= e->vx > 0 ? 0.8f : -0.8f; e->vx = -e->vx; }
        e->y = e->by + sinf(e->t * 0.06f) * 5;
        float dx = px - e->x, dy = py - e->y;
        if (!dead_t && e->t > 80 && fabsf(dx) < 56 && dy > 16 && dy < 150) { float d = sqrtf(dx * dx + dy * dy); e->k = 1; e->t = 0; e->vx = dx / d * 2.6f; e->vy = dy / d * 2.6f; if (fabsf(e->vx) < 0.1f) e->vx = 0.1f; }
      } else if (e->k == 1) {
        e->x += e->vx; e->y += e->vy;
        if (box_solid(e->x, e->y, ew, eh) || e->t > 70) { e->k = 2; e->t = 0; }
      } else {
        e->y -= 1.1f; e->x += e->vx > 0 ? 0.4f : -0.4f;
        if (e->y <= e->by) { e->y = e->by; e->k = 0; e->t = 0; e->x0 = e->x; e->vx = e->vx > 0 ? 0.5f : -0.5f; }
      }
    } else if (e->type == E_SNOWMAN) {
      // stands its ground and lobs snowballs at Dino
      e->vx = px > e->x ? 0.5f : -0.5f;
      float dx = px - e->x;
      if (e->t % 130 == 100 && !dead_t && fabsf(dx) < 175 && fabsf(py - e->y) < 90) { e->k = 14; add_shot(e->x + 3, e->y - 10, clampf(dx / 50, -2.6f, 2.6f), -3.2f, 3); plat_sfx(SFX_WALL); }
      if (e->k > 0) e->k--;
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
    if (e->type == E_MOLE && e->k == 0) continue;   // safe down its hole
    if (!dead_t && state == ST_PLAY) {
      float ex = e->x + 1, ey = e->y + (e->type == E_BEETLE ? 0 : 2);
      int hard = e->type == E_SPINY || (e->type == E_ROLL && e->k > 0);   // spikes, or a rolled-up shell
      float ax, ay, aw, ah;
      if (attack_box(&ax, &ay, &aw, &ah) && ax + aw > ex && ax < ex + ew - 2 && ay + ah > ey && ay < ey + eh) {
        if (hard && power != 2 && power != 3) {   // a bare head bounces off spikes and shells
          pvx = -face * 2.6f; pvy = -2.0f; atk_t = 0; wall_lock = 10; plat_sfx(SFX_WALL);
          continue;
        }
        e->dying = 1; plat_sfx(SFX_STOMP);
        spawn_parts(e->x + 6, e->y + 4, 10, e->type == E_BEETLE ? 0xFF3FA34D : 0xFFF7D038, 2.0f, 0.5f);
        continue;
      }
      if (px + PW > ex && px < ex + ew - 2 && py + PH > ey && py < ey + eh) {
        if (pvy > 0.5f && py + PH - ey < 9 && e->type == E_ROLL && e->k > 0) { pvy = -4.6f; plat_sfx(SFX_WALL); }   // bounce off the shell
        else if (pvy > 0.5f && py + PH - ey < 9 && e->type != E_SPINY && e->type != E_JELLY) {
          e->dying = 1; pvy = -4.6f; plat_sfx(SFX_STOMP);
          spawn_parts(e->x + 6, e->y + 4, 8, e->type == E_BEETLE ? 0xFF3FA34D : 0xFFF7D038, 1.5f, 0.3f);
        } else hurt();
      }
    }
  }
}

static void update_parts(void) {
  for (int i = 0; i < 32; i++) if (bubbles[i].life > 0) {
    bubbles[i].life--; bubbles[i].y -= 0.55f; bubbles[i].x += sinf((bubbles[i].life + i * 7) * 0.15f) * 0.25f;
    int tx = (int)floorf(bubbles[i].x / T), ty = (int)floorf(bubbles[i].y / T);
    if (L->theme != 9 && tile(tx, ty) != 'w') bubbles[i].life = 0;
    if (bubbles[i].y < 2 || solid_c(tile(tx, ty))) bubbles[i].life = 0;
  }
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
  else if (type == E_BAT) bat3d(b, fr == 7 ? -1.5f : sinf(fr * 1.0472f), yaw + dying * 0.3f, dying, S, crown);
  else if (type == E_CRAB) crab3d(b, fr * 1.0472f, fr >= 6 ? (float)(fr - 6) : 0.15f, yaw, dying, S);
  else if (type == E_FISH) fish3d(b, fr * 0.7854f, yaw + dying * 0.4f, dying, S);
  else if (type == E_JELLY) { static const float P[4] = { 0.35f, 0.8f, 1, 0.6f }; jelly3d(b, fr < 4 ? P[fr] : 0, fr * 0.9f, yaw, dying, S); }
  else if (type == E_ROLL) roller3d(b, fr, yaw, dying, S);
  else if (type == E_PENGUIN) penguin3d(b, fr * 1.5708f, fr >= 4, yaw, dying, S);
  else if (type == E_MOLE) mole3d(b, fr == 7 ? 0 : fr == 6 ? 1 : fr / 5.0f, fr == 6, yaw, dying, S);
  else if (type == E_BIRD) bird3d(b, fr == 7 ? 0.3f : sinf(fr * 1.0472f), fr == 6, yaw + dying * 0.3f, dying, S, crown);
  else snowman3d(b, fr == 4 ? 0 : sinf(fr * 1.5708f) * 0.5f, fr == 4, yaw, dying, S);
}
// the frame of a newer creature's model to show this tick
static int critter_frame(const Ent* e) {
  switch (e->type) {
    case E_FROG: return e->vy != 0 ? 7 : (e->t / 10) % 4;
    case E_SPINY: return e->t % 8;
    case E_PLANT: { int c = e->t % 130; return c > 45 && c < 75 ? 3 : (e->t / 12) % 2; }
    case E_BAT: return e->k == 0 ? 7 : (e->t / 2) % 6;
    case E_CRAB: return (e->t % 110) < 64 ? (e->t / 4) % 6 : 6 + (e->t / 8) % 2;
    case E_FISH: return (e->t / (e->k ? 2 : 4)) % 8;
    case E_JELLY: { int c = e->t % 110; return c < 22 ? c * 4 / 22 : 4 + (e->t / 10) % 4; }
    case E_ROLL: return e->k == 0 ? (e->t / 8) % 4 : e->k == 1 ? 4 : 5 + (e->t / 3) % 3;
    case E_PENGUIN: return e->k ? 4 : (e->t / 8) % 4;
    case E_MOLE: {
      if (e->k == 0) return 7;
      if (e->t < 12) return e->t / 2;
      if (e->t > 64) { int f = (76 - e->t) / 2; return f < 0 ? 7 : f > 5 ? 5 : f; }
      return e->t >= 26 && e->t < 42 ? 6 : 5;
    }
    case E_BIRD: return e->k == 1 ? 6 : (e->t / (e->k ? 2 : 3)) % 6;
    default: return e->k > 0 ? 4 : (e->t / 15) % 4;   // the snowman
  }
}
static void key_part(float x, float y, int part, uint32_t col, float sc) {   // one of the three pieces of the key
  if (part == 0) { fb_dot(x, y, 7.5f * sc, OUTLINE, 1); fb_dot(x, y, 6 * sc, col, 1); fb_dot(x, y, 2.6f * sc, OUTLINE, 1); }
  else if (part == 1) { fb_capsule(x - 7 * sc, y, x + 7 * sc, y, 3.4f * sc, OUTLINE); fb_capsule(x - 7 * sc, y, x + 7 * sc, y, 2 * sc, col); }
  else { fb_capsule(x - 5 * sc, y, x + 4 * sc, y, 3.4f * sc, OUTLINE); fb_capsule(x + 1 * sc, y, x + 1 * sc, y + 7 * sc, 3 * sc, OUTLINE); fb_capsule(x - 4 * sc, y, x - 4 * sc, y + 5 * sc, 3 * sc, OUTLINE);
         fb_capsule(x - 5 * sc, y, x + 4 * sc, y, 2 * sc, col); fb_capsule(x + 1 * sc, y, x + 1 * sc, y + 7 * sc, 1.6f * sc, col); fb_capsule(x - 4 * sc, y, x - 4 * sc, y + 5 * sc, 1.6f * sc, col); }
}
static void make_extra_art(void) {
  if (HD_FPLAT.px) return;
  HD_FPLAT = bmp_new(96, 22); HD_DOOR = bmp_new(40, 98);
  for (int y = 0; y < 22; y++) for (int x = 0; x < 96; x++) {   // a crumbly stone platform
    float fx = x + 0.5f, fy = y + 0.5f, dd = sd_rrect(fx, fy, 48, 10, 47, 9, 4);
    if (dd > 1) continue;
    uint32_t* d = &HD_FPLAT.px[y * 96 + x];
    uint32_t c = fy < 6 ? mixc(0xFFFFE2C8, 0xFFE0A07A, fy / 6) : mixc(0xFFC07A5A, 0xFF8A4A3A, (fy - 6) / 13);
    float cr = minf(sd_seg(fx, fy, 30, 6, 36, 20), minf(sd_seg(fx, fy, 62, 4, 58, 13), sd_seg(fx, fy, 58, 13, 66, 20)));
    if (cr < 0.8f && fy > 5) c = 0xFF5A2A22;
    over(d, OUTLINE, cov(dd)); over(d, c, cov(dd + 2));
  }
  for (int y = 0; y < 98; y++) for (int x = 0; x < 40; x++) {   // a big wooden door with iron bands and a keyhole
    float fx = x + 0.5f, fy = y + 0.5f, dd = sd_rrect(fx, fy, 20, 52, 19, 45, 3);
    float arch = maxf(sd_circle(fx, fy, 20, 22, 19.5f), fy - 22);
    dd = minf(dd, arch);
    if (dd > 1) continue;
    uint32_t* d = &HD_DOOR.px[y * 40 + x];
    int plank = (int)(fx / 8.5f);
    uint32_t c = mixc(0xFFB8743A, 0xFF7A4422, clampf(fy / 98 + (plank & 1) * 0.12f, 0, 1));
    if (fmodf(fx, 8.5f) < 1.2f) c = 0xFF4A2A14;
    if (fabsf(fy - 34) < 3.5f || fabsf(fy - 76) < 3.5f) c = mixc(0xFF8A90A0, 0xFF4A5060, fabsf(fy - 34) < 3.5f ? (fy - 30.5f) / 7 : (fy - 72.5f) / 7);
    float kh = minf(sd_circle(fx, fy, 20, 52, 3.4f), sd_rrect(fx, fy, 20, 58, 1.6f, 5, 1));
    float plate = sd_rrect(fx, fy, 20, 55, 6.5f, 10, 3);
    if (plate < 0) c = mixc(0xFFFFE27A, 0xFFE0A21C, (fy - 45) / 20);
    if (kh < 0) c = 0xFF1E1A22;
    over(d, OUTLINE, cov(dd)); over(d, c, cov(dd + 2.2f));
  }
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
      else if (c == 'K') {   // a glowing piece of the key
        float bob = sinf((frame + tx * 11) * 0.09f) * 3;
        fb_dot(X + 16, Y + 16 + bob, 14 + sinf(frame * 0.15f) * 2, 0xFFFFF2A0, 0.3f);
        key_part(X + 16, Y + 15 + bob, BOXK[ty][tx], 0xFFFFD84A, 1.4f);
        if ((frame / 6 + tx) % 9 == 0) fb_dot(X + 6 + (frame * 7 + tx) % 20, Y + 4 + (frame * 3) % 22, 2, 0xFFFFFFFF, 0.9f);
      }
      else if (c == 'D' && tile(tx, ty - 1) != 'D') hblit(&HD_DOOR, X - 4, Y - 2, 0);
      else if (c == 'd' && tile(tx, ty - 1) != 'd') { for (int v = 0; v < 96; v++) for (int u = 4; u < 28; u++) fb_over(X + u, Y + v, 0xFF1E1A22, v < 20 ? cov(sd_circle(u + 0.5f, v + 0.5f, 16, 20, 12)) : 0.9f); }
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
    if (e->type == E_PLAT) {
      if (e->k == 2) hblit(&HD_FPLAT, (int)ex + (e->fz > 0 && e->fz < 34 ? ((frame / 2) % 2 ? 2 : -2) : 0), (int)ey, 0);
      else hblit(&HD_PLAT, (int)ex, (int)ey, 0);
      continue;
    }
    if (e->type == E_SHOT) {
      if (e->k == 2) { fb_dot(ex + 6, ey + 6, 7, OUTLINE, 1); fb_dot(ex + 6, ey + 6, 5.4f, 0xFF8A5A30, 1); fb_dot(ex + 4.5f, ey + 4.5f, 2, 0xFFB8885A, 1); }
      else if (e->k == 3) { fb_dot(ex + 6, ey + 6, 7.5f, 0xFF8FAACB, 1); fb_dot(ex + 6, ey + 6, 6, 0xFFFFFFFF, 1); fb_dot(ex + 4.5f, ey + 4.5f, 2, 0xFFFFFFFF, 1); }
      else if (e->k) { fb_dot(ex + 6, ey + 6, 8, OUTLINE, 1); fb_dot(ex + 6, ey + 6, 6.4f, 0xFFF7D038, 1); fb_dot(ex + 4.5f, ey + 4.5f, 2.6f, 0xFFFFFBE0, 1); }
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
    if (e->type >= E_FROG && e->type <= E_SNOWMAN) {
      const Bmp* pic = &ESCR;
      int ti = e->type - E_FROG, fr = critter_frame(e), eh2 = (e->type == E_PLANT || e->type == E_SNOWMAN) ? 28 : 20;
      int flyer = e->type == E_BAT || e->type == E_FISH || e->type == E_JELLY || e->type == E_BIRD;
      if (steady) { Bmp* c = &CRIT_C[ti][dir][fr]; if (!c->px) critter3d(c, e->type, fr, dir ? 1.15f : -1.15f, 0, 1, 0); pic = c; }
      else { RT_SS = 1; critter3d(&ESCR, e->type, fr, e->fz * 1.15f, e->dying, 1, 0); RT_SS = 2; }
      if (flyer) hblit(pic, (int)ex + 12 - 26, (int)ey + 10 - 28 + e->dying * e->dying / 3, 0);
      else { if (!e->dying && e->type != E_MOLE) fb_shadow(ex + 12, ey + eh2 + 1, 12, 2.5f, 0.18f); hblit(pic, (int)ex + 12 - 26, (int)ey + eh2 - 38, 0); }
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
    if (power == 4 && show && !dead_t) {   // the star power: a little star that floats over his head
      float sx = X - 16 + (flip ? -4 : 4), sy = Y - 82 + sinf(frame * 0.12f) * 3;
      fb_dot(sx + 16, sy + 16, 15, 0xFFFFF2A0, 0.25f + 0.1f * sinf(frame * 0.2f));
      hblit(&HD_STARI, (int)sx, (int)sy, 0);
      if (frame % 9 == 0) spawn_parts(px + PW / 2 + (frame % 5 - 2) * 2, py - 12, 1, 0xFFFFF2A0, 0.4f, 0.2f);
    }
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
  // water in pools: see-through blue in front, with a bright wavy top
  if (L->theme != 9) for (int ty = ty0; ty <= ty0 + FB_H / HT + 1; ty++) for (int tx = tx0; tx <= tx0 + FB_W / HT + 1; tx++) {
    if (tile(tx, ty) != 'w') continue;
    int X = tx * HT - CX, Y = ty * HT - CY, top = tile(tx, ty - 1) != 'w';
    for (int v = 0; v < HT; v++) for (int u = 0; u < HT; u++) {
      float wave = top ? 3 + sinf((tx * HT + u) * 0.12f + frame * 0.08f) * 2 : -1;
      if (v < wave) continue;
      uint32_t c = v < wave + 2.2f ? 0xFFE8FBFF : mixc(0xFF4FC2E8, 0xFF1E6AAA, clampf((ty * HT + v - (ty * HT)) / 64.0f + 0.2f, 0, 1));
      fb_over(X + u, Y + v, c, v < wave + 2.2f ? 0.75f : 0.42f);
    }
  }
  for (int i = 0; i < 32; i++) if (bubbles[i].life > 0) {
    float bx = bubbles[i].x * 2 - CX, by = bubbles[i].y * 2 - CY, r = 2.2f + (i % 3) * 0.8f;
    fb_dot(bx, by, r + 1, 0xFFFFFFFF, 0.55f); fb_dot(bx, by, r, 0xFF9ADCF5, 0.35f); fb_dot(bx - r * 0.35f, by - r * 0.35f, r * 0.35f, 0xFFFFFFFF, 0.9f);
  }
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

static const Bmp* pow_icon(int k) { return k == 2 ? &HD_SWORDI : k == 3 ? &HD_HELMI : k == 4 ? &HD_STARI : &HD_WINGI; }
int g_pad_screen;   // the platform sets this when it shows game_draw_pad on a second screen
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
  if (level_keys) {   // the three pieces of the key; together they make one key
    for (int k = 0; k < 3; k++) {
      static const float KX[3] = { 18, 34, 49 };
      key_part(KX[k], 50, k, (keyp >> k) & 1 ? 0xFFFFD84A : 0xFF5A5468, 1.0f);
    }
    if (keyp == 7 && (frame / 20) % 2) fb_dot(34, 50, 26, 0xFFFFF2A0, 0.15f);
  }
  if (door_msg_t) text_c("FIND ALL 3 KEY PIECES TO OPEN THE DOOR", 32, 1, 0xFFFFF4C2);
  else if (key_msg_t) text_c("YOU MADE THE KEY! NOW OPEN THE DOOR", 32, 1, 0xFFFFD84A);
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
  if (slot && !g_pad_screen) {   // the kept power-up (on the Wii U it sits on the GamePad instead)
    fill(SCR_W - 104, 1, 18, 18, 0xFFF7D038); fill(SCR_W - 103, 2, 16, 16, 0xFF1B1020);
    make_extra_art();
    hblit(pow_icon(slot), (SCR_W - 103) * 2, 4, 0);
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
// Each world's map: the land follows the path, with trees scattered on it and one landmark.
static uint32_t map_px(int x, int y) { return (unsigned)x < SCR_W && (unsigned)y < SCR_H ? FB[y * 2 * FB_W + x * 2] : 0; }
static void map_land(int kind) {   // 0 jungle, 1 snow, 2 desert, 3 clouds, 4 sandy coast
  int snowy = kind == 1;
  const World* w = &WORLDS[world];
  uint32_t rim = snowy ? 0xFFBFE0F5 : 0xFFF0D9A0, land = snowy ? 0xFFF4FAFF : 0xFF4FAE52, shade = snowy ? 0xFFDCEBF7 : 0xFF409A46;
  if (kind == 2) { rim = 0xFFFFF0C0; land = 0xFFEBCB82; shade = 0xFFD9B46A; }
  if (kind == 3) { rim = 0xFFD6EAFB; land = 0xFFFFFFFF; shade = 0xFFEAF2FC; }
  if (kind == 4) { rim = 0xFFFFF6D8; land = 0xFFF2DA9A; shade = 0xFFE4C680; }
  for (int pass = 0; pass < 2; pass++) {
    for (int n = 0; n < w->n; n++) {
      disc(w->xy[n][0], w->xy[n][1], 28 + 5 * !pass, pass ? land : rim);
      if (n + 1 < w->n) disc((w->xy[n][0] + w->xy[n + 1][0]) / 2, (w->xy[n][1] + w->xy[n + 1][1]) / 2 + 6, 30 + 5 * !pass, pass ? land : rim);
    }
    for (int k = 0; k < 4; k++) { uint32_t h = hash2(k, 31 + world); disc(50 + (int)(h % 220), 50 + (int)((h >> 9) % 90), 20 + 5 * !pass, pass ? land : rim); }
  }
  for (int y = 0; y < SCR_H; y++) for (int x = 0; x < SCR_W; x++)
    if (map_px(x, y) == land && hash2(x, y + world * 50) % 10 == 0) put(x, y, shade);
  // the landmark sits by the boss level of the world
  int bi = w->kind[w->n - 1] == -3 ? w->n - 2 : w->n - 1;
  int lx = w->xy[bi][0], ly = w->xy[bi][1];
  if (kind == 4) {   // a lighthouse
    lx += 22; ly -= 36;
    for (int j = 0; j < 30; j++) fill(lx - 4 + j / 10, ly + j, 9 - (j / 10) * 2 + j / 5, 1, (j / 5) % 2 ? 0xFFFFFFFF : 0xFFE0384A);
    fill(lx - 3, ly - 6, 7, 6, 0xFF2B1A12); fill(lx - 2, ly - 5, 5, 4, (frame / 20) % 2 ? 0xFFFFF27A : 0xFFFFD84A);
    if ((frame / 20) % 2) for (int i = 0; i < 14; i++) put(lx + 4 + i, ly - 4 + i / 5, 0xFFFFF7C8);
  } else if (kind == 2) {   // a pyramid
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
    else if (kind == 4) { for (int j = 0; j < 8; j++) put(x + j / 4, y - j, 0xFF8A5A30); for (int f = -1; f <= 1; f += 2) { fill(x + 2, y - 9, 5 * (f > 0), 2, 0xFF2E9A4A); fill(x - 3 + (f > 0 ? 5 : 0), y - 8, 4, 2, 0xFF2E9A4A); } fill(x - 1, y - 10, 5, 2, 0xFF3FB85A); }
    else if (kind == 2) { fill(x - 1, y - 6, 3, 11, 0xFF4F9A52); fill(x - 5, y - 3, 4, 2, 0xFF4F9A52); fill(x - 5, y - 6, 2, 4, 0xFF4F9A52); fill(x + 2, y - 1, 4, 2, 0xFF4F9A52); fill(x + 4, y - 5, 2, 5, 0xFF4F9A52); }
    else if (snowy) pine(x, y, green);
    else { fill(x - 1, y, 3, 6, 0xFF6B3F1D); disc(x, y - 3, 6, t % 3 ? 0xFF2E7D32 : 0xFF3F9A3A); disc(x - 2, y - 5, 2, 0xFF5FC25A); }
  }
  if (world == 0) {   // Dino's house by the first stop
    int hx = w->xy[0][0], hy = w->xy[0][1] - 4;
    fill(hx - 12, hy - 4, 24, 14, 0xFFE8D5B0); fill(hx - 3, hy + 2, 6, 8, 0xFF6B3F1D);
    for (int j = 0; j < 11; j++) fill(hx - j - 4, hy - 14 + j, 2 * j + 8, 1, 0xFF4C8C2B);
  } else boat(w->xy[0][0] - 16, w->xy[0][1] + 8 + (frame / 30) % 2);
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
  if (world == 1) { sea0 = 0xFF1FA0C0; sea1 = 0xFF6FE0E8; wave = 0xFFE0FFFF; }
  if (world == 2) { sea0 = 0xFF3F7FB4; sea1 = 0xFF86BFE2; wave = 0xFFD6ECFA; }
  if (world == 5) { sea0 = 0xFF0B1030; sea1 = 0xFF24346E; wave = 0xFF4A5C94; }
  for (int y = 0; y < SCR_H; y++) {
    fill(0, y, SCR_W, 1, blend(sea0, sea1, y / (float)SCR_H));
  }
  for (int i = 0; i < 50; i++) {
    uint32_t h = hash2(i, 5);
    int x = (int)(h % SCR_W + frame / 6) % SCR_W, y = (h >> 9) % SCR_H;
    if ((frame / 15 + i) % 4) fill(x, y, 3, 1, wave);
  }
  { static const char KIND[7] = { 0, 4, 1, 2, 0, 1, 3 }; map_land(KIND[world]); }
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

static uint32_t* title_bg;   // the title's sky, trees and hill never change, so they are painted once and kept
void game_draw(uint32_t* fb) {
  FB = fb;
  char buf[64];
  if (state == ST_TITLE) {
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
    if (!g_no_hud) text_c("MOVE: STICK   JUMP: A OR B   HEADBUTT: X OR Y   PAUSE: +", 161, 1, 0xFFFFF4C2);
    if (!g_no_hud) text_c("SPRINT: HOLD L OR R   TWIRL: L OR R IN THE AIR   USE SLOT: -", 171, 1, 0xFFFFF4C2);
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

// ============================================================ the GamePad screen
// a picture drawn s times bigger (s whole pixels per pixel)
static void hblit_big(const Bmp* b, int X, int Y, int s) {
  for (int y = 0; y < b->h * s; y++) {
    int fy = Y + y; if ((unsigned)fy >= FB_H) continue;
    const uint32_t* src = b->px + (y / s) * b->w;
    for (int x = 0; x < b->w * s; x++) {
      int fx = X + x; if ((unsigned)fx >= FB_W) continue;
      uint32_t c = src[x / s], a = c >> 24;
      if (!a) continue;
      uint32_t* d = &FB[fy * FB_W + fx];
      *d = a == 255 ? c : lerpc(*d, c, a + (a >> 7));
    }
  }
}
// the power-up slot, in the middle of the GamePad
#define PAD_SX 250
#define PAD_SY 96
#define PAD_SW 140
int game_pad_slot_hit(int x, int y) { return x >= PAD_SX - 20 && x < PAD_SX + PAD_SW + 20 && y >= PAD_SY - 20 && y < PAD_SY + PAD_SW + 20; }
void game_draw_pad(uint32_t* fb) {
  uint32_t* keep = FB; FB = fb;
  char buf[64];
  int in_level = L && (state == ST_PLAY || state == ST_PAUSE || state == ST_CLEAR || (state == ST_OVER && state_t < 40));
  if (in_level) hd_background();
  else if (title_bg) memcpy(FB, title_bg, FB_W * FB_H * 4);
  else for (int i = 0; i < FB_W * FB_H; i++) FB[i] = 0xFF7EC8F0;
  make_extra_art();
  // coins, top left
  darken(4, 4, 74, 30, 0.35f);
  hblit_big(&HD_COIN[(frame / 3) % COIN_FRAMES], 12, 14, 2);
  snprintf(buf, sizeof buf, "%02d", bananas); text(buf, 36, 9, 4, 0xFFFFF4C2);
  // lives, top right
  darken(SCR_W - 80, 4, 76, 30, 0.35f);
  static Bmp pad_dino;
  if (!pad_dino.px) { DinoPose p; dino_idle_pose(&p, 50); dino3d(&pad_dino, &p, 0.5f, 0.9f); }
  hblit(&pad_dino, (SCR_W - 72) * 2 - (int)(36 * 0.9f), 66 - (int)(64 * 0.9f), 0);
  snprintf(buf, sizeof buf, "x%d", lives < 0 ? 0 : lives); text(buf, SCR_W - 62, 9, 4, 0xFFFFFFFF);
  // which level (or world) you are in, along the bottom
  const char* nm = in_level ? L->name : state == ST_MAP ? WORLDS[world].name : "AMAZING DINO";
  while ((*nm >= '0' && *nm <= '9') || *nm == ' ') nm++;   // drop the level number at the front
  int sc = text_w(nm, 3) <= SCR_W - 12 ? 3 : 2;
  darken(0, 146, SCR_W, 34, 0.35f);
  if (in_level) { snprintf(buf, sizeof buf, "WORLD %d-%d", lvl / 8 + 1, lvl % 8 + 1); text_c(buf, 148, 1, 0xFFFFF4C2); }
  text_c(nm, 158, sc, 0xFFFFFFFF);
  // the slot
  {
    int X = PAD_SX, Y = PAD_SY, W = PAD_SW;
    for (int y = Y - 6; y < Y + W + 6; y++) for (int x = X - 6; x < X + W + 6; x++) {
      int edge = x < X || x >= X + W || y < Y || y >= Y + W;
      if ((x < X - 2 || x >= X + W + 2) && (y < Y - 2 || y >= Y + W + 2)) continue;   // rounded-ish corners
      FB[y * FB_W + x] = edge ? (slot ? 0xFFF7D038 : 0xFFE8E0C8) : lerpc(FB[y * FB_W + x], 0xFF1B1020, 225);
    }
    if (slot) {
      float bob = sinf(frame * 0.08f) * 4;
      hblit_big(pow_icon(slot), X + W / 2 - 48, Y + W / 2 - 48 + (int)bob, 3);
      text_c("TAP TO USE", (Y + W) / 2 + 8, 1, 0xFFFFF4C2);
    } else text_c("EMPTY", (Y + W / 2) / 2 - 3, 2, 0xFF9A94A8);
    text_c("POWER-UP SLOT", Y / 2 - 9, 1, 0xFFFFFFFF);
  }
  if (state == ST_PAUSE) text_c("PAUSED", 30, 2, 0xFFFFFFFF);
  FB = keep;
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
enum { SONG_TITLE, SONG_MAP, SONG_GRASS, SONG_WILD, SONG_CAVE, SONG_FALLS, SONG_SNOW, SONG_ICE, SONG_NIGHT, SONG_DESERT, SONG_CASTLE, SONG_BOSS, SONG_WATER, SONG_BEACH, NSONGS };
// the built-in band only has tunes up to the boss; the recorded music has them all
static const Song SONGS[SONG_BOSS + 1] = {
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
    if (L->theme == 9) return SONG_WATER;
    if (lvl >= 48) return SONG_CASTLE;   // the whole Cloud Castle world
    switch (L->theme) {
      case 1: case 2: return SONG_CAVE;
      case 8: return SONG_DESERT;
      case 10: return SONG_BEACH;
      case 3: return SONG_FALLS;
      case 4: return lvl >= 32 ? SONG_WILD : SONG_FALLS;
      case 5: return SONG_SNOW;
      case 6: return SONG_ICE;
      case 7: return SONG_NIGHT;
      default: return lvl >= 32 ? SONG_WILD : SONG_GRASS;
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
  if (want == SONG_WATER) want = SONG_FALLS;
  if (want == SONG_BEACH) want = SONG_GRASS;
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
