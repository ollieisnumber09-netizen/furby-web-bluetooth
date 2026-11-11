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
// into one big picture when the level loads.
static Bmp LVB;
static int snowy(void) { return L->theme >= 5; }

static uint32_t ground_px(int X, int Y, int tx, int ty, float* alpha) {
  int u = X & 31, v = Y & 31;
  int sn = snowy();
  int top = tile(tx, ty - 1) != '#', lft = tile(tx - 1, ty) != '#', rgt = tile(tx + 1, ty) != '#';
  // round the top outer corners
  *alpha = 1;
  if (top && lft && u < 9 && v < 9) *alpha = cov(sqrtf((9 - u - 0.5f) * (9 - u - 0.5f) + (9 - v - 0.5f) * (9 - v - 0.5f)) - 9);
  if (top && rgt && u > 22 && v < 9) *alpha = cov(sqrtf((u + 0.5f - 23) * (u + 0.5f - 23) + (9 - v - 0.5f) * (9 - v - 0.5f)) - 9);
  int depth = v;
  for (int k = 1; k <= 3 && tile(tx, ty - k) == '#'; k++) depth += 32;
  uint32_t a = sn ? 0xFF8FA2BE : 0xFFD79A58, b2 = sn ? 0xFF5E6F8C : 0xFF8E5A30;
  uint32_t c = mixc(a, b2, depth / 110.0f);
  // soft layers of earth and a few pebbles
  float wob = sinf(X * 0.045f) * 5 + sinf(X * 0.013f + 2) * 7;
  float band = sinf((Y + wob) * 0.26f);
  c = mixc(c, band > 0 ? 0xFFFFFFFF : 0xFF000000, fabsf(band) * 0.07f);
  c = mixc(c, 0xFF000000, (noise2(X * 0.09f, Y * 0.09f) - 0.5f) * 0.14f);
  int cxp = (X / 26), cyp = (Y / 26);
  float px0 = cxp * 26 + 6 + hashf(cxp, cyp) * 14, py0 = cyp * 26 + 6 + hashf(cyp, cxp + 9) * 14;
  if (hash2(cxp, cyp * 7) % 3 == 0) {
    float dp = sd_ellipse(X + 0.5f, Y + 0.5f, px0, py0, 4.5f, 3.2f);
    if (dp < 1) c = mixc(c, (X + 0.5f - px0) + (Y + 0.5f - py0) < -1 ? 0xFFFFFFFF : 0xFF000000, 0.16f * cov(dp));
  }
  if (lft && u < 4) c = mixc(c, 0xFF000000, 0.28f - u * 0.06f);
  if (rgt && u > 27) c = mixc(c, 0xFF000000, 0.28f - (31 - u) * 0.06f);
  if (tile(tx, ty + 1) != '#' && ty + 1 < LH && v > 27) c = mixc(c, 0xFF000000, 0.3f);
  if (top) {
    // a thick cap of grass (or snow) with a scalloped lower edge
    float edge = 12 + 4 * fabsf(sinf((X + 4) * 3.14159f / 16));
    if (v < edge) {
      uint32_t g0 = sn ? 0xFFFFFFFF : 0xFFC8F27A, g1 = sn ? 0xFFEAF4FF : 0xFF6CC943, g2 = sn ? 0xFFB7CEE8 : 0xFF3B9A36;
      float k = v / edge;
      c = k < 0.45f ? mixc(g0, g1, k / 0.45f) : mixc(g1, g2, (k - 0.45f) / 0.55f);
      if (v < 2) c = mixc(c, 0xFFFFFFFF, 0.45f);
      if (edge - v < 1.6f) c = sn ? 0xFF8FAACB : 0xFF2A7A2E;
      c = mixc(c, 0xFF000000, (noise2(X * 0.2f, Y * 0.2f) - 0.5f) * 0.08f);
    } else if (v < edge + 5) c = mixc(c, 0xFF000000, 0.26f * (1 - (v - edge) / 5));
  }
  return c;
}
static uint32_t stone_px(int X, int Y, int tx, int ty) {
  int u = X & 31, v = Y & 31, th = L->theme;
  uint32_t base = 0xFFA9AFBE, dark = 0xFF646A7A, lite = 0xFFD6DBE6;
  if (th == 1) { base = 0xFF9A8BC0; dark = 0xFF54487A; lite = 0xFFC9BCE8; }
  if (th == 2) { base = 0xFF857869; dark = 0xFF483E36; lite = 0xFFAA9C8C; }
  if (th == 5 || th == 7) { base = 0xFF9CC0DE; dark = 0xFF567FA6; lite = 0xFFD2E8F7; }
  if (th == 6) { base = 0xFF5F8FB8; dark = 0xFF2A527A; lite = 0xFF8FBBDD; }
  // two bricks per tile, staggered every other row
  int row = v / 16, bu = (u + ((row + ty * 2) % 2 ? 16 : 0)) & 31, bv = v & 15;
  float dd = sd_rrect(bu + 0.5f, bv + 0.5f, 16, 8, 15.5f, 7.5f, 3);
  uint32_t c = mixc(base, dark, (noise2(X * 0.12f, Y * 0.12f) - 0.4f) * 0.35f);
  if (dd > -2.2f) c = (bv < 8 && bu > 1 && bu < 30) || bu < 3 ? lite : dark;
  if (bu < 3 && bv > 12) c = dark;
  if (dd > -0.3f) c = mixc(dark, 0xFF000000, 0.35f);
  int top = !solid_c(tile(tx, ty - 1));
  if (top && (th == 0 || th == 3 || th == 4 || th == 5 || th == 7)) {
    int sn = th >= 5;
    float edge = 7 + 2.5f * fabsf(sinf((X + 4) * 3.14159f / 16));
    if (v < edge) {
      c = mixc(sn ? 0xFFFFFFFF : 0xFFC8F27A, sn ? 0xFFCFE0F2 : 0xFF4FAE3C, v / edge);
      if (edge - v < 1.4f) c = sn ? 0xFF8FAACB : 0xFF2A7A2E;
    } else if (v < edge + 3) c = mixc(c, 0xFF000000, 0.22f);
  }
  return c;
}
static uint32_t ice_px(int X, int Y, int tx, int ty) {
  int u = X & 31, v = Y & 31;
  uint32_t c = mixc(0xFFC6ECFC, 0xFF86C6EE, v / 31.0f * 0.6f + (noise2(X * 0.05f, Y * 0.05f) - 0.5f) * 0.3f);
  int dg = (X + Y * 2) % 96;
  if (dg < 5) c = mixc(c, 0xFFFFFFFF, 0.75f); else if (dg > 30 && dg < 33) c = mixc(c, 0xFFFFFFFF, 0.45f);
  int top = tile(tx, ty - 1) != 'I', bot = tile(tx, ty + 1) != 'I', lft = tile(tx - 1, ty) != 'I', rgt = tile(tx + 1, ty) != 'I';
  if (top && v < 3) c = v == 0 ? 0xFFFFFFFF : mixc(c, 0xFFFFFFFF, 0.7f);
  if (lft && u < 3) c = mixc(c, 0xFFFFFFFF, 0.5f);
  if (bot && v > 28) c = mixc(c, 0xFF2F6FA8, 0.55f);
  if (rgt && u > 28) c = mixc(c, 0xFF2F6FA8, 0.45f);
  if ((top && v == 0 && 0) || (lft && u == 0) || (rgt && u == 31) || (bot && v == 31)) c = 0xFF3F7FB8;
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
  LVB = bmp_new(LW * HT, LH * HT);
  for (int ty = 0; ty < LH; ty++) for (int tx = 0; tx < LW; tx++) {
    char c = G[ty][tx];
    for (int v = 0; v < HT; v++) for (int u = 0; u < HT; u++) {
      int X = tx * HT + u, Y = ty * HT + v;
      uint32_t* d = &LVB.px[Y * LVB.w + X];
      float fx = u + 0.5f, fy = v + 0.5f;
      if (c == '#') { float al; uint32_t col = ground_px(X, Y, tx, ty, &al); over(d, col, al); }
      else if (c == 'X') *d = stone_px(X, Y, tx, ty);
      else if (c == 'I') *d = ice_px(X, Y, tx, ty);
      else if (c == '=') {
        // a bridge of round logs
        int k = u / 16; float lx = k * 16 + 8;
        float dd = sd_rrect(fx, fy, lx, 6.5f, 7.6f, 6, 4.5f);
        over(d, OUTLINE, cov(dd - 0.2f));
        over(d, mixc(mixc(0xFFE9B36A, 0xFF9A6233, (fy - 1) / 11), 0xFF000000, (hashf(tx * 2 + k, ty) - 0.5f) * 0.18f), cov(dd + 1.3f));
        over(d, 0xFFFFE2A8, cov(sd_seg(fx, fy, lx - 3.5f, 3.8f, lx + 3.5f, 3.8f) - 0.8f) * 0.7f);
        if (fy > 12 && fy < 20 && (fabsf(fx - 6) < 1.6f || fabsf(fx - 26) < 1.6f)) over(d, 0xFF5A3A1E, 1);
      } else if (c == '^') {
        for (int k = 0; k < 2; k++) {
          float cx = 8 + k * 16;
          float dd = sd_tri(fx, fy, cx - 7, 31, cx, 7, cx + 7, 31);
          over(d, OUTLINE, cov(dd - 1.2f));
          over(d, fx < cx - 1 ? mixc(0xFFFFFFFF, 0xFFC9CFDC, (fy - 7) / 24) : mixc(0xFF9CA2B2, 0xFF5C6272, (fy - 7) / 24), cov(dd));
        }
      } else if (c == ' ') {
        uint32_t col; float al = decor_px(X, Y, tx, ty, &col);
        if (al > 0) over(d, col, al);
      }
    }
  }
}

// ------------------------------------------------------------ backgrounds
#define BGW 1024
static Bmp BG_FAR, BG_NEAR;
static int bg_theme = -1;
static float hill(float x, float base, float a1, int k1, float a2, int k2, float ph) {
  return base - a1 * sinf(x * 6.28318f * k1 / BGW + ph) - a2 * sinf(x * 6.28318f * k2 / BGW + ph * 2.3f);
}
static void hd_render_bg(void) {
  int th = L->theme;
  if (bg_theme == th) return;
  bg_theme = th;
  if (!BG_FAR.px) { BG_FAR = bmp_new(BGW, FB_H); BG_NEAR = bmp_new(BGW, FB_H); }
  memset(BG_FAR.px, 0, (size_t)BGW * FB_H * 4); memset(BG_NEAR.px, 0, (size_t)BGW * FB_H * 4);
  int sn = th == 5 || th == 7, cave = th == 2 || th == 6, sky = th == 3;
  uint32_t hA = mixc(L->far, L->sky1, 0.5f), hB = L->far, hC = mixc(L->far, L->nearc, 0.5f);
  for (int y = 0; y < FB_H; y++) for (int x = 0; x < BGW; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    uint32_t* d = &BG_FAR.px[y * BGW + x];
    if (cave) {
      // hanging rock above, lumpy rock below
      float top = 60 + 46 * fabsf(sinf(fx * 6.28318f * 5 / BGW)) * fabsf(sinf(fx * 6.28318f * 2 / BGW + 1));
      if (fy < top) *d = mixc(L->far, 0xFF000000, 0.25f - 0.25f * fy / top);
      float h1 = hill(fx, 250, 26, 3, 12, 7, 0.4f);
      if (fy > h1) *d = mixc(mixc(L->far, 0xFFFFFFFF, 0.08f), L->far, (fy - h1) / 40);
      continue;
    }
    // fluffy clouds
    if (th != 7) for (int k = 0; k < 7; k++) {
      float cx = k * (BGW / 7.0f) + 40 + hashf(k, 1) * 60, cy = 40 + hashf(k, 2) * 110;
      if (fabsf(fx - cx) > 70 || fabsf(fy - cy) > 30) continue;
      float dd = minf(minf(sd_circle(fx, fy, cx - 22, cy + 4, 14), sd_circle(fx, fy, cx, cy - 4, 20)), minf(sd_circle(fx, fy, cx + 24, cy + 2, 16), sd_rrect(fx, fy, cx, cy + 10, 40, 8, 8)));
      if (dd < 1) over(d, mixc(0xFFFFFFFF, mixc(L->sky0, 0xFFFFFFFF, 0.6f), (fy - cy + 14) / 34), cov(dd) * 0.95f);
    }
    if (sky) {
      float h1 = hill(fx, 290, 14, 4, 8, 9, 0), h2 = hill(fx, 320, 12, 3, 6, 11, 2);
      if (fy > h1) over(d, mixc(0xFFFFFFFF, 0xFFD6EAFB, (fy - h1) / 50), cov(h1 - fy + 0.5f));
      if (fy > h2) over(d, 0xFFFFFFFF, cov(h2 - fy + 0.5f));
      continue;
    }
    // three rows of rolling hills, lighter and softer the further away
    float h1 = hill(fx, 196, 30, 2, 12, 5, 0.3f), h2 = hill(fx, 232, 24, 3, 10, 7, 1.7f), h3 = hill(fx, 268, 16, 4, 8, 9, 3.1f);
    if (fy > h1) over(d, mixc(mixc(hA, 0xFFFFFFFF, sn ? 0.5f : 0.2f), hA, clampf((fy - h1) / 26, 0, 1)), cov(h1 - fy + 0.5f));
    if (fy > h2) over(d, mixc(mixc(hB, 0xFFFFFFFF, sn ? 0.6f : 0.22f), hB, clampf((fy - h2) / 22, 0, 1)), cov(h2 - fy + 0.5f));
    if (fy > h3) over(d, mixc(mixc(hC, 0xFFFFFFFF, sn ? 0.7f : 0.2f), hC, clampf((fy - h3) / 18, 0, 1)), cov(h3 - fy + 0.5f));
  }
  if (cave || sky) {
    if (cave) for (int y = 0; y < FB_H; y++) for (int x = 0; x < BGW; x++) {
      // rock pillars close by
      float fx = x + 0.5f, fy = y + 0.5f;
      int k = (int)(fx / 256); float cx = k * 256 + 90 + hashf(k, 5) * 60, w = 26 + hashf(k, 6) * 14;
      float wd = w + 10 * fabsf(fy - 180) / 180;
      if (fabsf(fx - cx) < wd) BG_NEAR.px[y * BGW + x] = mixc(mixc(L->nearc, 0xFFFFFFFF, 0.10f), L->nearc, clampf((fx - cx + wd) / (2 * wd) * 1.4f, 0, 1));
    }
    return;
  }
  uint32_t tl = mixc(L->nearc, 0xFFFFFFFF, 0.30f), td = mixc(L->nearc, 0xFF000000, 0.25f);
  for (int y = 0; y < FB_H; y++) for (int x = 0; x < BGW; x++) {
    float fx = x + 0.5f, fy = y + 0.5f;
    uint32_t* d = &BG_NEAR.px[y * BGW + x];
    int k = (int)(fx / 128);
    for (int kk = k - 1; kk <= k + 1; kk++) {
      int ki = ((kk % 8) + 8) % 8;
      float cx = kk * 128 + 40 + hashf(ki, 3) * 50, hgt = 100 + hashf(ki, 4) * 70, base = 330;
      if (fabsf(fx - cx) > 64) continue;
      if (sn) {
        // a frosty pine: three tiers, snow on each
        over(d, td, cov(fabsf(fx - cx) - 4) * (fy > base - 30 && fy < base ? 1 : 0));
        for (int t = 0; t < 3; t++) {
          float ty0 = base - 18 - hgt + t * hgt * 0.27f, th2 = hgt * 0.5f, wmax = 22 + t * 12;
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
          float dd = sd_circle(fx, fy, bx, by, r);
          if (dd > 1) continue;
          float sh = clampf(((fx - bx) * 0.45f + (fy - by) * 0.75f) / r + 0.45f, 0, 1);
          over(d, mixc(tl, td, sh), cov(dd));
        }
      }
    }
    // a row of bushes along the bottom
    float bb = 330 - 14 * fabsf(sinf(fx * 6.28318f * 16 / BGW)) - 6 * fabsf(sinf(fx * 6.28318f * 7 / BGW + 1));
    if (fy > bb) over(d, sn ? mixc(0xFFFFFFFF, 0xFFBFD4EA, (fy - bb) / 30) : mixc(tl, td, clampf((fy - bb) / 34, 0, 1)), cov(bb - fy + 0.5f));
  }
}
static void hd_strip(const Bmp* b, int scroll, int yoff) {
  scroll = ((scroll % BGW) + BGW) % BGW;
  for (int y = 0; y < FB_H; y++) {
    int sy = y - yoff;
    if (sy < 0) continue;
    if (sy >= FB_H) sy = FB_H - 1;
    const uint32_t* src = b->px + sy * BGW;
    uint32_t* dst = FB + y * FB_W;
    for (int x = 0; x < FB_W; x++) {
      uint32_t c = src[(x + scroll) & (BGW - 1)], a = c >> 24;
      if (!a) continue;
      if (a == 255) dst[x] = c;
      else {
        uint32_t dd = dst[x], ia = 255 - a;
        dst[x] = 0xFF000000 | (((((c >> 16) & 255) * a + ((dd >> 16) & 255) * ia) / 255) << 16)
               | (((((c >> 8) & 255) * a + ((dd >> 8) & 255) * ia) / 255) << 8) | (((c & 255) * a + (dd & 255) * ia) / 255);
      }
    }
  }
}
static void hd_background(void) {
  hd_render_bg();
  for (int y = 0; y < FB_H; y++) {
    uint32_t c = mixc(L->sky0, L->sky1, y / (float)FB_H);
    uint32_t* row = FB + y * FB_W;
    for (int x = 0; x < FB_W; x++) row[x] = c;
  }
  float vy = LH * T > SCR_H ? camy / (float)(LH * T - SCR_H) : 1;
  int th = L->theme;
  if (th == 0 || th == 3 || th == 4 || th == 5) {
    // the sun, with a soft glow
    for (int j = -34; j <= 34; j++) for (int i = -34; i <= 34; i++) {
      int x = 520 + i, y = 62 + j;
      if ((unsigned)x >= FB_W || (unsigned)y >= FB_H) continue;
      float r = sqrtf((float)(i * i + j * j));
      if (r < 34) FB[y * FB_W + x] = mixc(FB[y * FB_W + x], 0xFFFFF7D0, r < 20 ? 1 : (34 - r) / 14 * 0.5f);
    }
  }
  if (th == 1 || th == 7) {
    for (int i = 0; i < 60; i++) {
      uint32_t h = hash2(i, 99);
      int x = (int)((h % 700) - camx * 0.1f) % 700, y = (h >> 12) % 200;
      if (x < 0) x += 700;
      if (x < FB_W - 1) { uint32_t c = (frame / 20 + i) % 7 ? 0xFFE8E0F8 : 0xFF9C8FC8; FB[y * FB_W + x] = c; if (i % 3 == 0) FB[y * FB_W + x + 1] = c; }
    }
    for (int j = -30; j <= 30; j++) for (int i = -30; i <= 30; i++) {
      float r = sqrtf((float)(i * i + j * j));
      if (r < 30) FB[(72 + j) * FB_W + 500 + i] = mixc(0xFFFFFBEA, 0xFFD9D2BC, clampf((i * 0.5f + j * 0.7f) / 30 + 0.3f, 0, 1));
    }
  }
  if (th == 7) {
    for (int x = 0; x < FB_W; x++) {
      float wx = x + camx * 0.1f;
      int y0 = 52 + (int)(sinf(wx * 0.015f + frame * 0.008f) * 20 + sinf(wx * 0.0055f - frame * 0.005f) * 16);
      for (int j = 0; j < 44; j++) {
        float a = (1 - j / 44.0f) * 0.45f * (0.6f + 0.4f * sinf(wx * 0.045f + frame * 0.02f));
        if ((unsigned)(y0 + j) < FB_H) FB[(y0 + j) * FB_W + x] = mixc(FB[(y0 + j) * FB_W + x], j < 16 ? 0xFF7CFFB2 : 0xFF58D6E8, a);
      }
    }
  }
  hd_strip(&BG_FAR, (int)(camx * 0.5f), (int)((1 - vy) * 40) - 44);
  hd_strip(&BG_NEAR, (int)(camx * 1.0f), (int)((1 - vy) * 70) - 50);
}
