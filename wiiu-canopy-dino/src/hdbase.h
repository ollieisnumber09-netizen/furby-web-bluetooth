// hd.h: the smooth "3D-style" art. Nothing is loaded from files: every picture is worked out
// by code (shapes with soft shading and outlines) into bitmaps when the game or a level starts,
// and after that drawing is just copying bitmaps. Sizes here are in screen pixels (FB_W x FB_H),
// which is twice the game's own units.
#include <stdlib.h>

#define HT 32               // one tile on screen
#define OUTLINE 0xFF1E1A22

typedef struct { int w, h; uint32_t* px; } Bmp;
static Bmp bmp_new(int w, int h) { Bmp b = { w, h, (uint32_t*)calloc((size_t)w * h, 4) }; return b; }

static inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
static inline float minf(float a, float b) { return a < b ? a : b; }
static inline float maxf(float a, float b) { return a > b ? a : b; }
static inline uint32_t mixc(uint32_t a, uint32_t b, float t) {
  t = clampf(t, 0, 1);
  int ar = (a >> 16) & 255, ag = (a >> 8) & 255, ab = a & 255;
  int br = (b >> 16) & 255, bg = (b >> 8) & 255, bb = b & 255;
  return 0xFF000000 | ((int)(ar + (br - ar) * t) << 16) | ((int)(ag + (bg - ag) * t) << 8) | (int)(ab + (bb - ab) * t);
}
// put colour c over pixel *d with coverage a (0..1); *d keeps a proper alpha
static inline void over(uint32_t* d, uint32_t c, float a) {
  if (a <= 0) return;
  if (a > 1) a = 1;
  float da = (*d >> 24) / 255.0f, oa = a + da * (1 - a);
  if (oa <= 0) return;
  float k = da * (1 - a);
  int r = (int)((((c >> 16) & 255) * a + ((*d >> 16) & 255) * k) / oa);
  int g = (int)((((c >> 8) & 255) * a + ((*d >> 8) & 255) * k) / oa);
  int b = (int)(((c & 255) * a + (*d & 255) * k) / oa);
  *d = ((uint32_t)(oa * 255 + 0.5f) << 24) | (r << 16) | (g << 8) | b;
}
static inline float cov(float d) { return clampf(0.5f - d, 0, 1); }   // distance to edge -> coverage

static inline uint32_t lerpc(uint32_t a, uint32_t b, int t) {   // t 0..256, whole-number maths
  uint32_t rb = (((a & 0xFF00FF) * (256 - t) + (b & 0xFF00FF) * t) >> 8) & 0xFF00FF;
  uint32_t g = (((a & 0x00FF00) * (256 - t) + (b & 0x00FF00) * t) >> 8) & 0x00FF00;
  return 0xFF000000 | rb | g;
}
// distances to shapes (negative inside)
static inline float sd_circle(float x, float y, float cx, float cy, float r) { return sqrtf((x - cx) * (x - cx) + (y - cy) * (y - cy)) - r; }
static inline float sd_ellipse(float x, float y, float cx, float cy, float rx, float ry) {
  float k = sqrtf((x - cx) * (x - cx) / (rx * rx) + (y - cy) * (y - cy) / (ry * ry));
  return (k - 1) * minf(rx, ry);
}
static inline float sd_seg(float x, float y, float ax, float ay, float bx, float by) {
  float px = x - ax, py = y - ay, dx = bx - ax, dy = by - ay;
  float l = dx * dx + dy * dy, t = l > 0 ? clampf((px * dx + py * dy) / l, 0, 1) : 0;
  float ex = px - dx * t, ey = py - dy * t;
  return sqrtf(ex * ex + ey * ey);
}
static inline float sd_tri(float x, float y, float ax, float ay, float bx, float by, float cx, float cy) {
  float d = minf(sd_seg(x, y, ax, ay, bx, by), minf(sd_seg(x, y, bx, by, cx, cy), sd_seg(x, y, cx, cy, ax, ay)));
  float s1 = (bx - ax) * (y - ay) - (by - ay) * (x - ax);
  float s2 = (cx - bx) * (y - by) - (cy - by) * (x - bx);
  float s3 = (ax - cx) * (y - cy) - (ay - cy) * (x - cx);
  int in = (s1 >= 0 && s2 >= 0 && s3 >= 0) || (s1 <= 0 && s2 <= 0 && s3 <= 0);
  return in ? -d : d;
}
static inline float sd_rrect(float x, float y, float cx, float cy, float hw, float hh, float r) {
  float qx = fabsf(x - cx) - (hw - r), qy = fabsf(y - cy) - (hh - r);
  float ox = maxf(qx, 0), oy = maxf(qy, 0);
  return sqrtf(ox * ox + oy * oy) + minf(maxf(qx, qy), 0) - r;
}
static inline float hashf(int x, int y) { return (hash2(x, y) & 0xFFFF) / 65535.0f; }
static float noise2(float x, float y) {   // smooth noise 0..1
  int xi = (int)floorf(x), yi = (int)floorf(y);
  float fx = x - xi, fy = y - yi;
  fx = fx * fx * (3 - 2 * fx); fy = fy * fy * (3 - 2 * fy);
  float a = hashf(xi, yi), b = hashf(xi + 1, yi), c = hashf(xi, yi + 1), d = hashf(xi + 1, yi + 1);
  return a + (b - a) * fx + (c - a) * fy + (a - b - c + d) * fx * fy;
}

// copy a bitmap to the screen, with see-through and soft edges
static void hblit(const Bmp* b, int X, int Y, int flip) {
  for (int j = 0; j < b->h; j++) {
    int y = Y + j;
    if ((unsigned)y >= FB_H) continue;
    const uint32_t* src = b->px + j * b->w;
    uint32_t* dst = FB + y * FB_W;
    for (int i = 0; i < b->w; i++) {
      int x = X + i;
      if ((unsigned)x >= FB_W) continue;
      uint32_t c = src[flip ? b->w - 1 - i : i];
      uint32_t a = c >> 24;
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


// soft drawing straight onto the screen
static inline void fb_over(int x, int y, uint32_t c, float a) {
  if (a <= 0 || (unsigned)x >= FB_W || (unsigned)y >= FB_H) return;
  uint32_t* d = &FB[y * FB_W + x];
  *d = mixc(*d, c, a);
}
static void fb_dot(float cx, float cy, float r, uint32_t c, float alpha) {
  for (int y = (int)(cy - r - 1); y <= (int)(cy + r + 1); y++) for (int x = (int)(cx - r - 1); x <= (int)(cx + r + 1); x++)
    fb_over(x, y, c, cov(sd_circle(x + 0.5f, y + 0.5f, cx, cy, r)) * alpha);
}
static void fb_capsule(float ax, float ay, float bx, float by, float r, uint32_t c) {
  int x0 = (int)(minf(ax, bx) - r - 1), x1 = (int)(maxf(ax, bx) + r + 1), y0 = (int)(minf(ay, by) - r - 1), y1 = (int)(maxf(ay, by) + r + 1);
  for (int y = y0; y <= y1; y++) for (int x = x0; x <= x1; x++) fb_over(x, y, c, cov(sd_seg(x + 0.5f, y + 0.5f, ax, ay, bx, by) - r));
}
