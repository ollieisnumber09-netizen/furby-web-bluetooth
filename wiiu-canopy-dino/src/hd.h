static Bmp make_banana(void) {
  Bmp b = bmp_new(32, 32);
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    float d1 = sd_circle(fx, fy, 12, 9, 15.5f), d2 = -sd_circle(fx, fy, 7.5f, 4.5f, 14.5f);
    float dd = maxf(d1, d2);
    if (dd > 3) continue;
    uint32_t* d = &b.px[y * b.w + x];
    over(d, OUTLINE, cov(dd - 1.6f));
    float s = clampf(d1 / -7.0f, 0, 1);   // 0 at the outer edge
    uint32_t c = mixc(0xFFE9A21C, 0xFFFFF07A, s);
    if (fx > 24.5f && fy < 9) c = 0xFF7A5230;
    if (fx < 7.5f && fy > 21.5f) c = 0xFF7A5230;
    over(d, c, cov(dd));
  }
  return b;
}
static Bmp make_box(int used) {
  Bmp b = bmp_new(32, 32);
  uint32_t lite = used ? 0xFFB98A5A : 0xFFFFE27A, mid = used ? 0xFF94673C : 0xFFF2B02E, dark = used ? 0xFF6B4524 : 0xFFB9741A;
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    float dd = sd_rrect(fx, fy, 16, 16, 16, 16, 5);
    if (dd > 0.5f) continue;
    uint32_t* d = &b.px[y * b.w + x];
    over(d, OUTLINE, cov(dd));
    float di = sd_rrect(fx, fy, 16, 16, 14.2f, 14.2f, 4);
    if (di < 1) {
      uint32_t c = mixc(lite, mid, (fx + fy) / 40);
      if (di > -3.2f) c = (fx + fy < 32) ? mixc(lite, 0xFFFFFFFF, 0.35f) : dark;   // bevel
      over(d, c, cov(di));
      float bolt = minf(minf(sd_circle(fx, fy, 6.5f, 6.5f, 1.2f), sd_circle(fx, fy, 25.5f, 6.5f, 1.2f)), minf(sd_circle(fx, fy, 6.5f, 25.5f, 1.2f), sd_circle(fx, fy, 25.5f, 25.5f, 1.2f)));
      over(d, dark, cov(bolt));
      if (!used) {
        for (int pass = 0; pass < 2; pass++) {
          float qx = fx - (pass ? 0 : 1.2f), qy = fy - (pass ? 0 : 1.2f);
          float ang = atan2f(qy - 12.5f, qx - 16);
          float ring = fabsf(sd_circle(qx, qy, 16, 12.5f, 4.3f)) - 1.5f;
          if (ang > 1.6f || ang < -2.9f) ring = 99;   // open on the lower left
          float q = minf(ring, minf(sd_seg(qx, qy, 16, 16.8f, 16, 19) - 1.5f, sd_circle(qx, qy, 16, 23.3f, 1.8f)));
          over(d, pass ? 0xFFFFFFFF : dark, cov(q));
        }
      }
    }
  }
  return b;
}
static Bmp make_spring(void) {
  Bmp b = bmp_new(32, 32);
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    float dbase = sd_rrect(fx, fy, 16, 27.5f, 11, 4, 2);
    float dcoil = minf(minf(sd_seg(fx, fy, 9, 23, 23, 20), sd_seg(fx, fy, 23, 20, 9, 17)), sd_seg(fx, fy, 9, 17, 23, 14)) - 1.4f;
    float dcap = sd_ellipse(fx, fy, 16, 11, 14.5f, 6);
    float dall = minf(dbase, minf(dcoil, dcap));
    if (dall > 3) continue;
    uint32_t* d = &b.px[y * b.w + x];
    over(d, OUTLINE, cov(dall - 1.6f));
    over(d, mixc(0xFFB6BAC6, 0xFF6C7080, (fy - 23) / 8), cov(dbase));
    over(d, mixc(0xFFE8ECF4, 0xFF8C8F9C, (fx - 9) / 14), cov(dcoil));
    if (dcap < 2) over(d, OUTLINE, cov(dcap - 1.4f));
    if (dcap < 1) {
      uint32_t c = mixc(0xFFFF8A7A, 0xFFC8322C, (fy - 5) / 12);
      if (sd_circle(fx, fy, 9, 10, 2.6f) < 0 || sd_circle(fx, fy, 17, 8.5f, 2.8f) < 0 || sd_circle(fx, fy, 24.5f, 11, 2.3f) < 0) c = 0xFFFFFFFF;
      over(d, c, cov(dcap));
    }
  }
  return b;
}

static Bmp make_sign(void) {
  Bmp b = bmp_new(32, 32);
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    float dpost = sd_rrect(fx, fy, 16, 24, 2.5f, 9, 1), dbrd = sd_rrect(fx, fy, 16, 11, 14, 9, 3.5f);
    float dall = minf(dpost, dbrd);
    if (dall > 3) continue;
    uint32_t* d = &b.px[y * b.w + x];
    over(d, OUTLINE, cov(dall - 1.6f));
    over(d, mixc(0xFF9A6233, 0xFF6B4220, (fx - 13) / 6), cov(dpost));
    if (dbrd < 1) {
      uint32_t c = mixc(0xFFF2C784, 0xFFC08A4A, (fy - 2) / 18);
      if (dbrd > -2) c = fx + fy < 27 ? 0xFFFFE2A8 : 0xFF9A6233;
      for (int k = 0; k < 3; k++) if (sd_seg(fx, fy, 8, 7 + k * 4, k == 2 ? 18 : 24, 7 + k * 4) < 0.9f) c = 0xFF7A5230;
      over(d, c, cov(dbrd));
    }
  }
  return b;
}
static Bmp make_icicle(void) {
  Bmp b = bmp_new(24, 32);
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    float dd = sd_tri(fx, fy, 3, 0, 21, 0, 12, 27) - 1.2f;
    if (dd > 3) continue;
    uint32_t* d = &b.px[y * b.w + x];
    over(d, 0xFF3F7FB8, cov(dd - 1.3f));
    uint32_t c = mixc(0xFFF4FCFF, 0xFF8CC8EC, clampf((fx - 5) / 13, 0, 1));
    if (fabsf(fx - 9 - (fy * 0.1f)) < 1.2f && fy < 18) c = 0xFFFFFFFF;
    over(d, c, cov(dd));
  }
  return b;
}
static Bmp make_wingi(void) {
  Bmp b = bmp_new(32, 32);
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    float dd = minf(minf(sd_ellipse(fx, fy, 13, 11, 10, 6), sd_ellipse(fx, fy, 16, 17, 10, 5)), sd_ellipse(fx, fy, 19, 22, 8, 4));
    dd = minf(dd, sd_circle(fx, fy, 23, 12, 5));
    if (dd > 3) continue;
    uint32_t* d = &b.px[y * b.w + x];
    over(d, OUTLINE, cov(dd - 1.6f));
    uint32_t c = mixc(0xFFFFFFFF, 0xFFB9D4F2, clampf((fy - 6) / 20, 0, 1));
    if (fabsf(sd_ellipse(fx, fy, 16, 17, 10, 5)) < 0.6f && fx < 18) c = 0xFF8FB0D8;
    over(d, c, cov(dd));
  }
  return b;
}
static Bmp make_swordi(void) {
  Bmp b = bmp_new(32, 32);
  for (int y = 0; y < b.h; y++) for (int x = 0; x < b.w; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    float dbl = sd_seg(fx, fy, 12, 20, 26, 6) - 2.6f, dgd = sd_seg(fx, fy, 8, 17, 15, 24) - 1.8f, dhd = sd_seg(fx, fy, 10, 22, 6, 26) - 1.9f;
    float dall = minf(dbl, minf(dgd, dhd));
    if (dall > 3) continue;
    uint32_t* d = &b.px[y * b.w + x];
    over(d, OUTLINE, cov(dall - 1.6f));
    over(d, 0xFF8B5A2B, cov(dhd));
    over(d, (fx - fy) > -3.5f ? 0xFFF4F7FC : 0xFFAEB6C8, cov(dbl));
    over(d, 0xFFF2B02E, cov(dgd));
  }
  return b;
}
static Bmp HD_BANANA, HD_BOX[2], HD_SPRING, HD_SIGN, HD_ICICLE, HD_WINGI, HD_SWORDI;
static int hd_ready;
static void hd_init(void) {
  if (hd_ready) return;
  hd_ready = 1;
  HD_BANANA = make_banana();
  HD_BOX[0] = make_box(0); HD_BOX[1] = make_box(1);
  HD_SPRING = make_spring();
  HD_SIGN = make_sign(); HD_ICICLE = make_icicle(); HD_WINGI = make_wingi(); HD_SWORDI = make_swordi();
}

// ------------------------------------------------------------ the level picture
// All the ground, stone, ice, planks, spikes and little decorations of a level are painted once
// into one big picture when the level loads. To keep loading quick, the earth, brick and ice
// patterns are worked out once as repeating textures (512x256) and copied from there; only the
// edges of things (grass caps, rounded corners, shadows) are worked out pixel by pixel.
static Bmp LVB;
static unsigned char* LV_ROW;   // for each screen row of the level picture: 0 empty, 1 has something
static int snowy(void) { return L->theme >= 5; }
#define TXW 512
#define TXH 256
static uint32_t *GTEX[2][2], *STEX[5], *ITEX;
static float CAP_TAB[32];
static float pnoise(float x, float y, int wx, int wy) {   // smooth noise that repeats every wx, wy cells
  int xi = (int)floorf(x), yi = (int)floorf(y);
  float fx = x - xi, fy = y - yi;
  fx = fx * fx * (3 - 2 * fx); fy = fy * fy * (3 - 2 * fy);
  int x0 = ((xi % wx) + wx) % wx, x1 = (x0 + 1) % wx, y0 = ((yi % wy) + wy) % wy, y1 = (y0 + 1) % wy;
  float a = hashf(x0, y0), b = hashf(x1, y0), c = hashf(x0, y1), d = hashf(x1, y1);
  return a + (b - a) * fx + (c - a) * fy + (a - b - c + d) * fx * fy;
}
static uint32_t* ground_tex(int sn, int dark) {
  if (GTEX[sn][dark]) return GTEX[sn][dark];
  uint32_t* t = GTEX[sn][dark] = (uint32_t*)malloc(TXW * TXH * 4);
  uint32_t base = dark ? (sn ? 0xFF5E6F8C : 0xFF8E5A30) : (sn ? 0xFF8FA2BE : 0xFFD79A58);
  for (int Y = 0; Y < TXH; Y++) for (int X = 0; X < TXW; X++) {
    uint32_t c = base;
    // soft layers of earth and a few pebbles
    float wob = sinf(X * 6.28318f * 4 / TXW) * 5 + sinf(X * 6.28318f / TXW + 2) * 7;
    float band = sinf((Y + wob) * 6.28318f * 11 / TXH);
    c = mixc(c, band > 0 ? 0xFFFFFFFF : 0xFF000000, fabsf(band) * 0.07f);
    c = mixc(c, 0xFF000000, (pnoise(X * 46.0f / TXW, Y * 23.0f / TXH, 46, 23) - 0.5f) * 0.14f);
    int cxp = X / 32, cyp = Y / 32;
    if (hash2(cxp, cyp * 7) % 3 == 0) {
      float px0 = cxp * 32 + 8 + hashf(cxp, cyp) * 16, py0 = cyp * 32 + 8 + hashf(cyp, cxp + 9) * 16;
      float dp = sd_ellipse(X + 0.5f, Y + 0.5f, px0, py0, 4.5f, 3.2f);
      if (dp < 1) c = mixc(c, (X + 0.5f - px0) + (Y + 0.5f - py0) < -1 ? 0xFFFFFFFF : 0xFF000000, 0.16f * cov(dp));
    }
    t[Y * TXW + X] = c;
  }
  return t;
}
static int stone_pal(void) { int th = L->theme; return th == 1 ? 1 : th == 2 ? 2 : (th == 5 || th == 7) ? 3 : th == 6 ? 4 : 0; }
static uint32_t* stone_tex(int pal) {
  if (STEX[pal]) return STEX[pal];
  static const uint32_t COL[5][3] = { { 0xFFA9AFBE, 0xFF646A7A, 0xFFD6DBE6 }, { 0xFF9A8BC0, 0xFF54487A, 0xFFC9BCE8 }, { 0xFF857869, 0xFF483E36, 0xFFAA9C8C },
                                      { 0xFF9CC0DE, 0xFF567FA6, 0xFFD2E8F7 }, { 0xFF5F8FB8, 0xFF2A527A, 0xFF8FBBDD } };
  uint32_t* t = STEX[pal] = (uint32_t*)malloc(TXW * TXH * 4);
  uint32_t base = COL[pal][0], dark = COL[pal][1], lite = COL[pal][2];
  for (int Y = 0; Y < TXH; Y++) for (int X = 0; X < TXW; X++) {
    // two bricks per tile, staggered every other row
    int u = X & 31, v = Y & 31, row = v / 16, bu = (u + (row ? 16 : 0)) & 31, bv = v & 15;
    float dd = sd_rrect(bu + 0.5f, bv + 0.5f, 16, 8, 15.5f, 7.5f, 3);
    uint32_t c = mixc(base, dark, (pnoise(X * 61.0f / TXW, Y * 31.0f / TXH, 61, 31) - 0.4f) * 0.35f);
    if (dd > -2.2f) c = (bv < 8 && bu > 1 && bu < 30) || bu < 3 ? lite : dark;
    if (bu < 3 && bv > 12) c = dark;
    if (dd > -0.3f) c = mixc(dark, 0xFF000000, 0.35f);
    t[Y * TXW + X] = c;
  }
  return t;
}
static uint32_t* ice_tex(void) {
  if (ITEX) return ITEX;
  ITEX = (uint32_t*)malloc(TXW * TXH * 4);
  for (int Y = 0; Y < TXH; Y++) for (int X = 0; X < TXW; X++) {
    uint32_t c = mixc(0xFFC6ECFC, 0xFF86C6EE, (Y & 31) / 31.0f * 0.6f + (pnoise(X * 26.0f / TXW, Y * 13.0f / TXH, 26, 13) - 0.5f) * 0.3f);
    int dg = (X + Y * 2) % 128;
    if (dg < 5) c = mixc(c, 0xFFFFFFFF, 0.75f); else if (dg > 40 && dg < 43) c = mixc(c, 0xFFFFFFFF, 0.45f);
    ITEX[Y * TXW + X] = c;
  }
  return ITEX;
}
static uint32_t ground_px(int X, int Y, int tx, int ty, int depth_t, int top, int lft, int rgt, int bot, const uint32_t* tl, const uint32_t* td, float* alpha) {
  int u = X & 31, v = Y & 31, sn = snowy();
  *alpha = 1;
  if (top && lft && u < 9 && v < 9) *alpha = cov(sqrtf((9 - u - 0.5f) * (9 - u - 0.5f) + (9 - v - 0.5f) * (9 - v - 0.5f)) - 9);
  if (top && rgt && u > 22 && v < 9) *alpha = cov(sqrtf((u + 0.5f - 23) * (u + 0.5f - 23) + (9 - v - 0.5f) * (9 - v - 0.5f)) - 9);
  int depth = v + depth_t * 32, ti = (Y & (TXH - 1)) * TXW + (X & (TXW - 1));
  uint32_t c = depth >= 110 ? td[ti] : lerpc(tl[ti], td[ti], depth * 256 / 110);
  if (lft && u < 4) c = lerpc(c, 0xFF000000, (int)((0.28f - u * 0.06f) * 256));
  if (rgt && u > 27) c = lerpc(c, 0xFF000000, (int)((0.28f - (31 - u) * 0.06f) * 256));
  if (bot && v > 27) c = lerpc(c, 0xFF000000, 77);
  if (top) {
    // a thick cap of grass (or snow) with a scalloped lower edge
    float edge = 12 + 4 * CAP_TAB[(X + 4) & 31];
    if (v < edge) {
      uint32_t g0 = sn ? 0xFFFFFFFF : 0xFFC8F27A, g1 = sn ? 0xFFEAF4FF : 0xFF6CC943, g2 = sn ? 0xFFB7CEE8 : 0xFF3B9A36;
      float k = v / edge;
      c = k < 0.45f ? mixc(g0, g1, k / 0.45f) : mixc(g1, g2, (k - 0.45f) / 0.55f);
      if (v < 2) c = lerpc(c, 0xFFFFFFFF, 115);
      if (edge - v < 1.6f) c = sn ? 0xFF8FAACB : 0xFF2A7A2E;
      else if (hash2(X, Y) & 1) c = lerpc(c, 0xFF000000, 8);
    } else if (v < edge + 5) c = lerpc(c, 0xFF000000, (int)(0.26f * (1 - (v - edge) / 5) * 256));
  }
  return c;
}
// little things that grow on top of the ground: returns alpha, writes the colour
static float decor_px(int X, int Y, int tx, int ty, uint32_t* out) {
  char below = tile(tx, ty + 1);
  if (!(below == '#' || (below == 'X' && L->theme != 1 && L->theme != 2 && L->theme != 6))) return 0;
  if (tile(tx, ty) != ' ') return 0;
  for (int i = 0; i < nsigns; i++) if (sign_x[i] == tx && sign_y[i] == ty) return 0;
  if (goal_x == tx && goal_y == ty) return 0;
  uint32_t h = hash2(tx * 3 + 1, ty * 5 + lvl);
  int kind = h % 9;
  float fx = (X & 31) + 0.5f, fy = (Y & 31) + 0.5f, a = 0;
  int sn = snowy(), cave = L->theme == 2;
  if (kind == 0 || kind == 1) {
    // a flower (jungle) or an ice crystal (snow)
    float cx = 10 + (h >> 8) % 12, cy = 16;
    if (cave) return 0;
    if (sn) {
      float dd = minf(sd_tri(fx, fy, cx - 4, 32, cx, 12, cx + 4, 32), sd_tri(fx, fy, cx + 1, 32, cx + 7, 20, cx + 9, 32));
      *out = mixc(0xFFEAF8FF, 0xFF7FB8DC, (fx - cx + 4) / 12);
      float al = cov(dd);
      if (dd > 0 && dd < 1.5f) { *out = 0xFF3F7FB8; al = cov(dd - 1.2f); }
      return al;
    }
    static const uint32_t PET[4] = { 0xFFFF7FA8, 0xFFFFE066, 0xFF8FC7FF, 0xFFFFFFFF };
    float dstem = sd_seg(fx, fy, cx, 32, cx, cy) - 1.0f;
    float dpet = 99;
    for (int k = 0; k < 5; k++) dpet = minf(dpet, sd_circle(fx, fy, cx + cosf(k * 1.2566f - 1.57f) * 4.2f, cy + sinf(k * 1.2566f - 1.57f) * 4.2f, 3.3f));
    float dcen = sd_circle(fx, fy, cx, cy, 2.4f);
    float dleaf = sd_ellipse(fx, fy, cx + 3.5f, 26, 3.5f, 1.8f);
    float dall = minf(minf(dstem, dpet), dleaf);
    if (dall > 2) return 0;
    *out = OUTLINE; a = cov(dall - 1.3f);
    uint32_t tmp = a > 0 ? (((uint32_t)(a * 255)) << 24) | (OUTLINE & 0xFFFFFF) : 0;
    over(&tmp, 0xFF3B9A36, cov(minf(dstem, dleaf)));
    over(&tmp, mixc(PET[(h >> 12) % 4], 0xFF000000, (fy - cy) / 30), cov(dpet));
    over(&tmp, 0xFFF2A21C, cov(dcen));
    *out = tmp | 0xFF000000;
    return (tmp >> 24) / 255.0f;
  }
  if (kind == 2 || kind == 3 || kind == 4) {
    // a tuft of grass / a little snow mound / pebbles in caves
    if (cave) {
      float dd = minf(sd_ellipse(fx, fy, 10, 30, 6, 4), sd_ellipse(fx, fy, 21, 31, 4, 3));
      *out = mixc(0xFF9A8C7C, 0xFF5A4E44, (fy - 26) / 6);
      return cov(dd);
    }
    if (sn) {
      float dd = sd_ellipse(fx, fy, 16, 33, 13, 8);
      *out = mixc(0xFFFFFFFF, 0xFFC9DCF0, (fx - 8) / 22);
      return cov(dd);
    }
    float dd = 99; int n = 3 + (h >> 6) % 3;
    for (int k = 0; k < n; k++) {
      float bx = 6 + k * (20.0f / n) + (hash2(h, k) % 4), tip = bx + ((int)(hash2(k, h) % 9) - 4), hh = 9 + hash2(h + k, 3) % 9;
      dd = minf(dd, sd_tri(fx, fy, bx - 2.4f, 33, tip, 32 - hh, bx + 2.4f, 33));
    }
    if (dd > 1.6f) return 0;
    *out = dd > 0.2f ? 0xFF2A7A2E : mixc(0xFFB6EE6A, 0xFF4FAE3C, (fy - 16) / 16);
    return cov(dd - 1.0f);
  }
  if (kind == 5 && !cave) {
    // a round bush (or snowy bush)
    float d1 = sd_circle(fx, fy, 10, 25, 8), d2 = sd_circle(fx, fy, 19, 21, 10), d3 = sd_circle(fx, fy, 26, 27, 6);
    float dd = minf(d1, minf(d2, d3));
    if (dd > 2 || fy > 32) return 0;
    float cxb = d2 <= d1 && d2 <= d3 ? 19 : (d1 <= d3 ? 10 : 26), cyb = d2 <= d1 && d2 <= d3 ? 21 : (d1 <= d3 ? 25 : 27);
    float s = clampf(((fx - cxb) * 0.5f + (fy - cyb) * 0.8f) / 9 + 0.45f, 0, 1);
    *out = sn ? mixc(0xFFFFFFFF, 0xFF9FBAD8, s) : mixc(0xFF9BE860, 0xFF2F8E3A, s);
    if (dd > 0) *out = sn ? 0xFF6F8FB4 : 0xFF1F5E2A;
    return cov(dd - 1.4f);
  }
  return 0;
}
static void hd_render_level(void) {
  if (LVB.px) free(LVB.px);
  if (LV_ROW) free(LV_ROW);
  LVB = bmp_new(LW * HT, LH * HT);
  LV_ROW = (unsigned char*)calloc(LH * HT, 1);
  for (int i = 0; i < 32; i++) CAP_TAB[i] = fabsf(sinf(i * 3.14159f / 16));
  int sn = snowy(), th = L->theme;
  const uint32_t *tl = ground_tex(sn, 0), *td = ground_tex(sn, 1), *ts = stone_tex(stone_pal()), *ti = ice_tex();
  int stone_cap = th == 0 || th == 3 || th == 4 || th == 5 || th == 7;
  int decor_ok_x = !(th == 1 || th == 2 || th == 6);
  for (int ty = 0; ty < LH; ty++) for (int tx = 0; tx < LW; tx++) {
    char c = G[ty][tx];
    uint32_t* base = &LVB.px[ty * HT * LVB.w + tx * HT];
    int X0 = tx * HT, Y0 = ty * HT;
    if (c == '#') {
      int top = tile(tx, ty - 1) != '#', lft = tile(tx - 1, ty) != '#', rgt = tile(tx + 1, ty) != '#', bot = tile(tx, ty + 1) != '#' && ty + 1 < LH;
      int dt = 0;
      for (int k = 1; k <= 3 && tile(tx, ty - k) == '#'; k++) dt++;
      if (dt >= 3 && !top && !lft && !rgt && !bot) {
        // deep inside the ground: straight copy from the texture (the top rows of the tile still fade in)
        for (int v = 0; v < HT; v++) {
          int Y = Y0 + v, depth = v + dt * 32;
          const uint32_t* srow = td + (Y & (TXH - 1)) * TXW;
          if (depth >= 110) for (int u = 0; u < HT; u++) base[v * LVB.w + u] = srow[(X0 + u) & (TXW - 1)];
          else { const uint32_t* lrow = tl + (Y & (TXH - 1)) * TXW; int t = depth * 256 / 110; for (int u = 0; u < HT; u++) base[v * LVB.w + u] = lerpc(lrow[(X0 + u) & (TXW - 1)], srow[(X0 + u) & (TXW - 1)], t); }
        }
      } else for (int v = 0; v < HT; v++) for (int u = 0; u < HT; u++) {
        float al; uint32_t col = ground_px(X0 + u, Y0 + v, tx, ty, dt, top, lft, rgt, bot, tl, td, &al);
        if (al >= 1) base[v * LVB.w + u] = col; else over(&base[v * LVB.w + u], col, al);
      }
    } else if (c == 'X') {
      int top = !solid_c(tile(tx, ty - 1)) && stone_cap;
      for (int v = 0; v < HT; v++) {
        const uint32_t* srow = ts + ((Y0 + v) & (TXH - 1)) * TXW;
        for (int u = 0; u < HT; u++) {
          uint32_t col = srow[(X0 + u) & (TXW - 1)];
          if (top && v < 13) {
            float edge = 7 + 2.5f * CAP_TAB[(X0 + u + 4) & 31];
            if (v < edge) {
              col = mixc(sn ? 0xFFFFFFFF : 0xFFC8F27A, sn ? 0xFFCFE0F2 : 0xFF4FAE3C, v / edge);
              if (edge - v < 1.4f) col = sn ? 0xFF8FAACB : 0xFF2A7A2E;
            } else if (v < edge + 3) col = lerpc(col, 0xFF000000, 56);
          }
          base[v * LVB.w + u] = col;
        }
      }
    } else if (c == 'I') {
      int top = tile(tx, ty - 1) != 'I', bot = tile(tx, ty + 1) != 'I', lft = tile(tx - 1, ty) != 'I', rgt = tile(tx + 1, ty) != 'I';
      for (int v = 0; v < HT; v++) {
        const uint32_t* srow = ti + ((Y0 + v) & (TXH - 1)) * TXW;
        for (int u = 0; u < HT; u++) {
          uint32_t col = srow[(X0 + u) & (TXW - 1)];
          if (top && v < 3) col = v == 0 ? 0xFFFFFFFF : lerpc(col, 0xFFFFFFFF, 180);
          if (lft && u < 3) col = lerpc(col, 0xFFFFFFFF, 128);
          if (bot && v > 28) col = lerpc(col, 0xFF2F6FA8, 140);
          if (rgt && u > 28) col = lerpc(col, 0xFF2F6FA8, 115);
          if ((lft && u == 0) || (rgt && u == 31) || (bot && v == 31)) col = 0xFF3F7FB8;
          base[v * LVB.w + u] = col;
        }
      }
    } else if (c == '=') {
      for (int v = 0; v < 21; v++) for (int u = 0; u < HT; u++) {
        // a bridge of round logs
        uint32_t* d = &base[v * LVB.w + u];
        float fx = u + 0.5f, fy = v + 0.5f;
        int k = u / 16; float lx = k * 16 + 8;
        float dd = sd_rrect(fx, fy, lx, 6.5f, 7.6f, 6, 4.5f);
        over(d, OUTLINE, cov(dd - 0.2f));
        over(d, mixc(mixc(0xFFE9B36A, 0xFF9A6233, (fy - 1) / 11), 0xFF000000, (hashf(tx * 2 + k, ty) - 0.5f) * 0.18f), cov(dd + 1.3f));
        over(d, 0xFFFFE2A8, cov(sd_seg(fx, fy, lx - 3.5f, 3.8f, lx + 3.5f, 3.8f) - 0.8f) * 0.7f);
        if (fy > 12 && fy < 20 && (fabsf(fx - 6) < 1.6f || fabsf(fx - 26) < 1.6f)) over(d, 0xFF5A3A1E, 1);
      }
    } else if (c == '^') {
      for (int v = 4; v < HT; v++) for (int u = 0; u < HT; u++) {
        uint32_t* d = &base[v * LVB.w + u];
        float fx = u + 0.5f, fy = v + 0.5f;
        for (int k = 0; k < 2; k++) {
          float cx = 8 + k * 16;
          float dd = sd_tri(fx, fy, cx - 7, 31, cx, 7, cx + 7, 31);
          over(d, OUTLINE, cov(dd - 1.2f));
          over(d, fx < cx - 1 ? mixc(0xFFFFFFFF, 0xFFC9CFDC, (fy - 7) / 24) : mixc(0xFF9CA2B2, 0xFF5C6272, (fy - 7) / 24), cov(dd));
        }
      }
    } else if (c == ' ') {
      char below = tile(tx, ty + 1);
      if (!(below == '#' || (below == 'X' && decor_ok_x))) continue;
      if (hash2(tx * 3 + 1, ty * 5 + lvl) % 9 > 5) continue;
      for (int v = 0; v < HT; v++) for (int u = 0; u < HT; u++) {
        uint32_t col; float al = decor_px(X0 + u, Y0 + v, tx, ty, &col);
        if (al > 0) over(&base[v * LVB.w + u], col, al);
      }
    } else continue;
    for (int v = 0; v < HT; v++) LV_ROW[Y0 + v] = 1;
  }
}

// ------------------------------------------------------------ backgrounds
// Two wide strips that scroll at different speeds. The far one holds the sky, clouds and hills
// and has no see-through parts, so it is copied a row at a time; the near one (trees, bushes)
// knows which of its rows are empty, solid or mixed.
#define BGW 1024
static Bmp BG_FAR, BG_NEAR, HD_SUN, HD_MOON;
static unsigned char NEAR_ROW[FB_H];   // 0 empty, 1 solid, 2 mixed
static int bg_theme = -1;
static void hd_render_bg(void) {
  int th = L->theme;
  if (bg_theme == th) return;
  bg_theme = th;
  if (!BG_FAR.px) {
    BG_FAR = bmp_new(BGW, FB_H); BG_NEAR = bmp_new(BGW, FB_H);
    HD_SUN = bmp_new(68, 68); HD_MOON = bmp_new(62, 62);
    for (int j = 0; j < 68; j++) for (int i = 0; i < 68; i++) {
      float r = sqrtf((i - 33.5f) * (i - 33.5f) + (j - 33.5f) * (j - 33.5f));
      if (r < 34) HD_SUN.px[j * 68 + i] = ((uint32_t)((r < 20 ? 1 : (34 - r) / 14 * 0.5f) * 255) << 24) | 0xFFF7D0;
    }
    for (int j = 0; j < 62; j++) for (int i = 0; i < 62; i++) {
      float r = sqrtf((i - 30.5f) * (i - 30.5f) + (j - 30.5f) * (j - 30.5f));
      if (r < 30.5f) HD_MOON.px[j * 62 + i] = ((uint32_t)(cov(r - 30) * 255) << 24) | (mixc(0xFFFFFBEA, 0xFFD9D2BC, clampf(((i - 30.5f) * 0.5f + (j - 30.5f) * 0.7f) / 30 + 0.3f, 0, 1)) & 0xFFFFFF);
    }
  }
  memset(BG_NEAR.px, 0, (size_t)BGW * FB_H * 4);
  int sn = th == 5 || th == 7, cave = th == 2 || th == 6, sky = th == 3;
  uint32_t hA = mixc(L->far, L->sky1, 0.5f), hB = L->far, hC = mixc(L->far, L->nearc, 0.5f);
  static float H1[BGW], H2[BGW], H3[BGW];
  for (int x = 0; x < BGW; x++) {
    float fx = x + 0.5f, k = 6.28318f / BGW;
    if (cave) { H1[x] = 60 + 46 * fabsf(sinf(fx * k * 5)) * fabsf(sinf(fx * k * 2 + 1)); H2[x] = 250 - 26 * sinf(fx * k * 3 + 0.4f) - 12 * sinf(fx * k * 7 + 0.92f); }
    else if (sky) { H1[x] = 290 - 14 * sinf(fx * k * 4) - 8 * sinf(fx * k * 9); H2[x] = 320 - 12 * sinf(fx * k * 3 + 2) - 6 * sinf(fx * k * 11 + 4.6f); }
    else { H1[x] = 196 - 30 * sinf(fx * k * 2 + 0.3f) - 12 * sinf(fx * k * 5 + 0.69f); H2[x] = 232 - 24 * sinf(fx * k * 3 + 1.7f) - 10 * sinf(fx * k * 7 + 3.91f); H3[x] = 268 - 16 * sinf(fx * k * 4 + 3.1f) - 8 * sinf(fx * k * 9 + 7.13f); }
  }
  // sky first, then whatever sits in it
  for (int y = 0; y < FB_H; y++) {
    uint32_t c = mixc(L->sky0, L->sky1, clampf((y - 20) / (float)FB_H, 0, 1));
    uint32_t* row = BG_FAR.px + y * BGW;
    for (int x = 0; x < BGW; x++) row[x] = c;
  }
  if (th == 1 || th == 7) for (int i = 0; i < 110; i++) {   // stars
    uint32_t h = hash2(i, 99);
    int x = h % (BGW - 1), y = (h >> 12) % 200;
    uint32_t c = i % 7 ? 0xFFE8E0F8 : 0xFF9C8FC8;
    BG_FAR.px[y * BGW + x] = c; if (i % 3 == 0) BG_FAR.px[y * BGW + x + 1] = c;
  }
  if (th == 7) for (int x = 0; x < BGW; x++) {   // northern lights
    float k = 6.28318f / BGW;
    int y0 = 62 + (int)(sinf(x * k * 3) * 20 + sinf(x * k + 1) * 16);
    for (int j = 0; j < 44; j++) {
      float a = (1 - j / 44.0f) * 0.45f * (0.6f + 0.4f * sinf(x * k * 7));
      if ((unsigned)(y0 + j) < FB_H) { uint32_t* d = &BG_FAR.px[(y0 + j) * BGW + x]; *d = mixc(*d, j < 16 ? 0xFF7CFFB2 : 0xFF58D6E8, a); }
    }
  }
  if (!cave && th != 7) for (int k = 0; k < 7; k++) {   // fluffy clouds
    float cx = k * (BGW / 7.0f) + 40 + hashf(k, 1) * 60, cy = 50 + hashf(k, 2) * 110;
    for (int y = (int)cy - 30; y <= (int)cy + 30; y++) for (int xx = (int)cx - 70; xx <= (int)cx + 70; xx++) {
      if ((unsigned)y >= FB_H) continue;
      float fx = xx + 0.5f, fy = y + 0.5f;
      float dd = minf(minf(sd_circle(fx, fy, cx - 22, cy + 4, 14), sd_circle(fx, fy, cx, cy - 4, 20)), minf(sd_circle(fx, fy, cx + 24, cy + 2, 16), sd_rrect(fx, fy, cx, cy + 10, 40, 8, 8)));
      if (dd < 1) { uint32_t* d = &BG_FAR.px[y * BGW + (xx & (BGW - 1))]; *d = mixc(*d, mixc(0xFFFFFFFF, mixc(L->sky0, 0xFFFFFFFF, 0.6f), (fy - cy + 14) / 34), cov(dd) * 0.95f); }
    }
  }
  for (int y = 0; y < FB_H; y++) for (int x = 0; x < BGW; x++) {
    float fy = y + 0.5f;
    uint32_t* d = &BG_FAR.px[y * BGW + x];
    if (cave) {
      // hanging rock above, lumpy rock below
      if (fy < H1[x]) *d = mixc(L->far, 0xFF000000, 0.25f - 0.25f * fy / H1[x]);
      if (fy > H2[x]) *d = mixc(mixc(L->far, 0xFFFFFFFF, 0.08f), L->far, (fy - H2[x]) / 40);
    } else if (sky) {
      if (fy > H1[x] - 1) *d = mixc(*d, mixc(0xFFFFFFFF, 0xFFD6EAFB, (fy - H1[x]) / 50), cov(H1[x] - fy + 0.5f));
      if (fy > H2[x] - 1) *d = mixc(*d, 0xFFFFFFFF, cov(H2[x] - fy + 0.5f));
    } else if (fy > H1[x] - 1) {
      // three rows of rolling hills, lighter and softer the further away
      *d = mixc(*d, mixc(mixc(hA, 0xFFFFFFFF, sn ? 0.5f : 0.2f), hA, clampf((fy - H1[x]) / 26, 0, 1)), cov(H1[x] - fy + 0.5f));
      if (fy > H2[x] - 1) *d = mixc(*d, mixc(mixc(hB, 0xFFFFFFFF, sn ? 0.6f : 0.22f), hB, clampf((fy - H2[x]) / 22, 0, 1)), cov(H2[x] - fy + 0.5f));
      if (fy > H3[x] - 1) *d = mixc(*d, mixc(mixc(hC, 0xFFFFFFFF, sn ? 0.7f : 0.2f), hC, clampf((fy - H3[x]) / 18, 0, 1)), cov(H3[x] - fy + 0.5f));
    }
  }
  if (cave) {
    for (int k = 0; k < 4; k++) {   // rock pillars close by
      float cx = k * 256 + 90 + hashf(k, 5) * 60, w = 26 + hashf(k, 6) * 14;
      for (int y = 0; y < FB_H; y++) {
        float wd = w + 10 * fabsf(y + 0.5f - 180) / 180;
        for (int xx = (int)(cx - wd); xx <= (int)(cx + wd); xx++) {
          float fx = xx + 0.5f;
          if (fabsf(fx - cx) < wd) BG_NEAR.px[y * BGW + (xx & (BGW - 1))] = mixc(mixc(L->nearc, 0xFFFFFFFF, 0.10f), L->nearc, clampf((fx - cx + wd) / (2 * wd) * 1.4f, 0, 1));
        }
      }
    }
  } else if (!sky) {
    uint32_t tl = mixc(L->nearc, 0xFFFFFFFF, 0.30f), td = mixc(L->nearc, 0xFF000000, 0.25f);
    for (int ki = 0; ki < 8; ki++) {
      float cx = ki * 128 + 40 + hashf(ki, 3) * 50, hgt = 100 + hashf(ki, 4) * 70, base = 330;
      for (int y = (int)(base - hgt - 60); y < (int)base; y++) for (int xx = (int)cx - 64; xx <= (int)cx + 64; xx++) {
        if ((unsigned)y >= FB_H) continue;
        float fx = xx + 0.5f, fy = y + 0.5f;
        uint32_t* d = &BG_NEAR.px[y * BGW + (xx & (BGW - 1))];
        if (sn) {
          // a frosty pine: three tiers, snow on each
          over(d, td, cov(fabsf(fx - cx) - 4) * (fy > base - 30 && fy < base ? 1 : 0));
          for (int t = 0; t < 3; t++) {
            float ty0 = base - 18 - hgt + t * hgt * 0.27f, th2 = hgt * 0.5f, wmax = 22 + t * 12;
            if (fy < ty0 - 1 || fy > ty0 + th2 + 1 || fabsf(fx - cx) > wmax + 1) continue;
            float dd = sd_tri(fx, fy, cx - wmax, ty0 + th2, cx, ty0, cx + wmax, ty0 + th2);
            if (dd > 1) continue;
            float sh = clampf((fx - cx) / wmax * 0.5f + 0.5f, 0, 1);
            uint32_t c = mixc(mixc(L->nearc, 0xFF2F7A6A, 0.5f), mixc(L->nearc, 0xFF1B4A52, 0.6f), sh);
            float snowline = ty0 + th2 * (0.38f + 0.12f * sinf(fx * 0.5f));
            if (fy < snowline) c = mixc(0xFFFFFFFF, 0xFFCFE0F2, sh);
            over(d, c, cov(dd));
          }
        } else {
          // a tree with a round, shaded top
          over(d, td, cov(fabsf(fx - cx) - 5) * (fy > base - hgt && fy < base ? 1 : 0));
          for (int c2 = 0; c2 < 4; c2++) {
            float bx = cx + (hashf(ki, 10 + c2) - 0.5f) * 56, by = base - hgt + (hashf(ki, 20 + c2) - 0.5f) * 26, r = 24 + hashf(ki, 30 + c2) * 12;
            if (fabsf(fx - bx) > r + 1 || fabsf(fy - by) > r + 1) continue;
            float dd = sd_circle(fx, fy, bx, by, r);
            if (dd > 1) continue;
            float sh = clampf(((fx - bx) * 0.45f + (fy - by) * 0.75f) / r + 0.45f, 0, 1);
            over(d, mixc(tl, td, sh), cov(dd));
          }
        }
      }
    }
    // a row of bushes along the bottom
    for (int x = 0; x < BGW; x++) {
      float fx = x + 0.5f;
      float bb = 330 - 14 * fabsf(sinf(fx * 6.28318f * 16 / BGW)) - 6 * fabsf(sinf(fx * 6.28318f * 7 / BGW + 1));
      for (int y = (int)bb - 1; y < FB_H; y++) {
        float fy = y + 0.5f;
        if (fy > bb - 0.5f) over(&BG_NEAR.px[y * BGW + x], sn ? mixc(0xFFFFFFFF, 0xFFBFD4EA, (fy - bb) / 30) : mixc(tl, td, clampf((fy - bb) / 34, 0, 1)), cov(bb - fy + 0.5f));
      }
    }
  }
  for (int y = 0; y < FB_H; y++) {
    int any = 0, all = 1;
    const uint32_t* row = BG_NEAR.px + y * BGW;
    for (int x = 0; x < BGW; x++) { uint32_t a = row[x] >> 24; if (a) any = 1; if (a != 255) all = 0; }
    NEAR_ROW[y] = !any ? 0 : all ? 1 : 2;
  }
}
static void hd_background(void) {
  hd_render_bg();
  float vy = LH * T > SCR_H ? camy / (float)(LH * T - SCR_H) : 1;
  int th = L->theme;
  // far strip: no see-through parts, so each row is two straight copies
  {
    int scroll = (((int)(camx * 0.5f) % BGW) + BGW) % BGW, yoff = (int)((1 - vy) * 40) - 44;
    int n1 = BGW - scroll; if (n1 > FB_W) n1 = FB_W;
    for (int y = 0; y < FB_H; y++) {
      int sy = y - yoff;
      if (sy < 0) sy = 0;
      if (sy >= FB_H) sy = FB_H - 1;
      const uint32_t* src = BG_FAR.px + sy * BGW;
      memcpy(FB + y * FB_W, src + scroll, (size_t)n1 * 4);
      if (n1 < FB_W) memcpy(FB + y * FB_W + n1, src, (size_t)(FB_W - n1) * 4);
    }
  }
  if (th == 0 || th == 3 || th == 4 || th == 5) hblit(&HD_SUN, 486, 28, 0);
  if (th == 1 || th == 7) hblit(&HD_MOON, 470, 42, 0);
  {
    int scroll = (((int)(camx * 1.0f) % BGW) + BGW) % BGW, yoff = (int)((1 - vy) * 70) - 50;
    for (int y = 0; y < FB_H; y++) {
      int sy = y - yoff;
      if (sy < 0) continue;
      if (sy >= FB_H) sy = FB_H - 1;
      if (!NEAR_ROW[sy]) continue;
      const uint32_t* src = BG_NEAR.px + sy * BGW;
      uint32_t* dst = FB + y * FB_W;
      if (NEAR_ROW[sy] == 1) {
        int n1 = BGW - scroll; if (n1 > FB_W) n1 = FB_W;
        memcpy(dst, src + scroll, (size_t)n1 * 4);
        if (n1 < FB_W) memcpy(dst + n1, src, (size_t)(FB_W - n1) * 4);
        continue;
      }
      for (int x = 0; x < FB_W; x++) {
        uint32_t c = src[(x + scroll) & (BGW - 1)], a = c >> 24;
        if (!a) continue;
        if (a == 255) dst[x] = c;
        else dst[x] = lerpc(dst[x], c, a + (a >> 7));
      }
    }
  }
}
