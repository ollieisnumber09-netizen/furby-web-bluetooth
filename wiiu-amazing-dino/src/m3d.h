// m3d.h: a small 3D renderer. Dino, the beetles, the bees, the coins and the boxes are real 3D
// models (triangle meshes built from round "lathe" shapes and flat faces). They are lit, drawn
// with a depth buffer at twice the size and shrunk down for smooth edges, then copied to the
// screen like any other picture. Model space: x right, y up, z towards the viewer; one unit is
// one screen pixel at scale 1.

typedef struct { float m[9], t[3]; } M3;   // p' = m * p + t
static M3 m3_id(void) { M3 r = { { 1, 0, 0, 0, 1, 0, 0, 0, 1 }, { 0, 0, 0 } }; return r; }
static M3 m3_mul(M3 a, M3 b) {   // a after b
  M3 r;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) r.m[i * 3 + j] = a.m[i * 3] * b.m[j] + a.m[i * 3 + 1] * b.m[3 + j] + a.m[i * 3 + 2] * b.m[6 + j];
    r.t[i] = a.m[i * 3] * b.t[0] + a.m[i * 3 + 1] * b.t[1] + a.m[i * 3 + 2] * b.t[2] + a.t[i];
  }
  return r;
}
static M3 m3_trans(float x, float y, float z) { M3 r = m3_id(); r.t[0] = x; r.t[1] = y; r.t[2] = z; return r; }
static M3 m3_scale(float x, float y, float z) { M3 r = m3_id(); r.m[0] = x; r.m[4] = y; r.m[8] = z; return r; }
static M3 m3_rotx(float a) { M3 r = m3_id(); float c = cosf(a), s = sinf(a); r.m[4] = c; r.m[5] = -s; r.m[7] = s; r.m[8] = c; return r; }
static M3 m3_roty(float a) { M3 r = m3_id(); float c = cosf(a), s = sinf(a); r.m[0] = c; r.m[2] = s; r.m[6] = -s; r.m[8] = c; return r; }
static M3 m3_rotz(float a) { M3 r = m3_id(); float c = cosf(a), s = sinf(a); r.m[0] = c; r.m[1] = -s; r.m[3] = s; r.m[4] = c; return r; }
static void m3_pt(const M3* a, const float* p, float* o) {
  for (int i = 0; i < 3; i++) o[i] = a->m[i * 3] * p[0] + a->m[i * 3 + 1] * p[1] + a->m[i * 3 + 2] * p[2] + a->t[i];
}
// turns a surface direction (normal) the right way even when the matrix squashes things
static void m3_nrm(const M3* a, const float* n, float* o) {
  const float* m = a->m;
  float c[9] = { m[4] * m[8] - m[5] * m[7], m[5] * m[6] - m[3] * m[8], m[3] * m[7] - m[4] * m[6],
                 m[2] * m[7] - m[1] * m[8], m[0] * m[8] - m[2] * m[6], m[1] * m[6] - m[0] * m[7],
                 m[1] * m[5] - m[2] * m[4], m[2] * m[3] - m[0] * m[5], m[0] * m[4] - m[1] * m[3] };
  for (int i = 0; i < 3; i++) o[i] = c[i * 3] * n[0] + c[i * 3 + 1] * n[1] + c[i * 3 + 2] * n[2];
  float l = sqrtf(o[0] * o[0] + o[1] * o[1] + o[2] * o[2]);
  if (l > 0) { o[0] /= l; o[1] /= l; o[2] /= l; }
}

// ---- the drawing target (twice the final size, with depth)
static struct { int w, h, cap, ss; float* z; uint32_t* c; float ox, oy, sc; } RT;
static int RT_SS = 2;   // 2: draw at twice the size and shrink (smooth edges); 1: draw directly (four times quicker, for big things)
static void rt_begin(int w, int h, float ox, float oy, float sc) {   // w, h: final size; ox, oy: where the model's origin lands
  int ss = RT_SS, n = w * ss * h * ss;
  if (n > RT.cap) { free(RT.z); free(RT.c); RT.z = (float*)malloc(n * sizeof(float)); RT.c = (uint32_t*)malloc(n * 4); RT.cap = n; }
  RT.ss = ss; RT.w = w * ss; RT.h = h * ss; RT.ox = ox * ss; RT.oy = oy * ss; RT.sc = sc * ss;
  memset(RT.z, 0, (size_t)n * sizeof(float));   // depths are stored +1000, so 0 means "nothing here yet"
  memset(RT.c, 0, (size_t)n * 4);
}
static int RT_EDGE = 1, RT_TMP;
static void rt_end(Bmp* b) {
  bmp_fit(b, RT.w / RT.ss, RT.h / RT.ss);
  if (RT.ss == 1) memcpy(b->px, RT.c, (size_t)RT.w * RT.h * 4);
  else for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) {
    int r = 0, g = 0, bl = 0, n = 0;
    for (int k = 0; k < 4; k++) {
      uint32_t c = RT.c[(y * 2 + (k >> 1)) * RT.w + x * 2 + (k & 1)];
      if (c >> 24) { r += (c >> 16) & 255; g += (c >> 8) & 255; bl += c & 255; n++; }
    }
    if (n) b->px[y * b->w + x] = ((uint32_t)(n * 255 / 4) << 24) | ((r / n) << 16) | ((g / n) << 8) | (bl / n);
  }
  if (!RT_EDGE) return;
  // a thin soft dark edge, so white Dino still stands out against clouds and snow
  for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) {
    uint32_t c = b->px[y * b->w + x], a = c >> 24;
    if (a == 255) continue;
    int near = 0;
    for (int k = 0; k < 4; k++) {
      int xx = x + (k == 0) - (k == 1), yy = y + (k == 2) - (k == 3);
      if (xx >= 0 && yy >= 0 && xx < b->w && yy < b->h && ((b->px[yy * b->w + xx] >> 24) == 255 || (RT_TMP && 0))) near = 1;
    }
    if (!near && !a) continue;
    float fa = a / 255.0f, ea = near ? 0.6f : 0.35f;
    uint32_t col = a ? mixc(0xFF2A2A3A, c | 0xFF000000, fa) : 0xFF2A2A3A;
    b->px[y * b->w + x] = ((uint32_t)((fa + (1 - fa) * ea) * 255) << 24) | (col & 0xFFFFFF);
  }
}

// ---- materials: what colour a point of a surface is before the light falls on it
enum { MAT_PLAIN, MAT_DINO, MAT_SHELL, MAT_BEE, MAT_QBOX, MAT_COIN, MAT_SHINY };
static struct { float eye, mouth; int mood; } FACE;
static uint32_t mat_colour(uint32_t col, int mat, const float* o, float* gloss) {
  *gloss = 0.12f;
  switch (mat) {
    case MAT_DINO: {
      if (o[2] <= 0 || o[0] < -6.5f || o[0] > 6.5f || o[1] > 11 || o[1] < -11) return col;
      // the face, painted on the front: two tall line eyes and a little "w" mouth
      float bx = o[0] + 4.2f, by = -o[1];
      float e1 = 1.6f, e2 = 6.8f, eyc = -5.5f, eh = 4.2f * maxf(FACE.eye, 0.02f), de;
      if (FACE.mood == 2)
        de = minf(minf(sd_seg(bx, by, e1 - 2, eyc - 2.5f, e1 + 2, eyc + 2.5f), sd_seg(bx, by, e1 + 2, eyc - 2.5f, e1 - 2, eyc + 2.5f)),
                  minf(sd_seg(bx, by, e2 - 2, eyc - 2.5f, e2 + 2, eyc + 2.5f), sd_seg(bx, by, e2 + 2, eyc - 2.5f, e2 - 2, eyc + 2.5f))) - 0.85f;
      else if (FACE.mood == 4)
        de = minf(minf(sd_seg(bx, by, e1 - 2, eyc + 1, e1, eyc - 2), sd_seg(bx, by, e1, eyc - 2, e1 + 2, eyc + 1)),
                  minf(sd_seg(bx, by, e2 - 2, eyc + 1, e2, eyc - 2), sd_seg(bx, by, e2, eyc - 2, e2 + 2, eyc + 1))) - 0.9f;
      else {
        float top = FACE.mood == 3 ? eyc - eh * 0.4f : eyc - eh;
        de = minf(sd_seg(bx, by, e1, top, e1, eyc + eh), sd_seg(bx, by, e2, top, e2, eyc + eh)) - 1.15f;
        if (FACE.mood == 3) de = minf(de, minf(sd_seg(bx, by, e1 - 2.4f, eyc - 5.6f, e1 + 1.6f, eyc - 3.4f), sd_seg(bx, by, e2 - 1.6f, eyc - 3.4f, e2 + 2.4f, eyc - 5.6f)) - 0.8f);
        if (FACE.mood == 5) de = minf(de, minf(sd_seg(bx, by, e1 - 2.8f, eyc - 3.6f, e1 + 1.2f, eyc - 6.0f), sd_seg(bx, by, e2 - 1.2f, eyc - 6.0f, e2 + 2.8f, eyc - 3.6f)) - 0.8f);   // sad eyebrows
      }
      float mx = bx, my = by - 3.4f, dm;
      if (FACE.mood == 2) dm = sd_seg(mx, my, 1, 1.5f, 7.4f, 1.5f) - 0.8f;
      else if (FACE.mood == 5) dm = minf(minf(sd_seg(mx, my, 0.6f, 2.8f, 2.6f, 0.8f), sd_seg(mx, my, 2.6f, 0.8f, 5.8f, 0.8f)), sd_seg(mx, my, 5.8f, 0.8f, 7.8f, 2.8f)) - 0.8f;   // a frown
      else {
        dm = minf(minf(sd_seg(mx, my, 0, 0, 2.1f, 2.1f), sd_seg(mx, my, 2.1f, 2.1f, 4.2f, 0)),
                  minf(sd_seg(mx, my, 4.2f, 0, 6.3f, 2.1f), sd_seg(mx, my, 6.3f, 2.1f, 8.4f, 0))) - 0.75f;
        if (FACE.mouth > 0.05f) {
          float m = FACE.mouth, tw = fmodf(mx + 42, 4.2f), yw = 2.1f * (1 - fabsf(tw / 2.1f - 1));
          float du = sd_ellipse(mx, my, 4.2f, 0.6f, 4.3f, 1.2f + 5.2f * m);
          if (my > yw - 0.2f && mx > -0.3f && mx < 8.7f) {
            if (du < 0) col = mixc(col, 0xFFE8707E, minf(1, m * 3));
            dm = minf(dm, fabsf(du) - 0.7f);
          }
        }
      }
      return mixc(col, 0xFF1E1A22, clampf(0.5f - minf(de, dm) * 1.6f, 0, 1));
    }
    case MAT_SHELL: {
      *gloss = 0.5f;
      static const float SP[5][3] = { { -5, 9, 2.6f }, { 5, 8, 2.2f }, { 0, 3, 2.4f }, { -6, 1, 1.8f }, { 6, 0, 2.0f } };
      for (int i = 0; i < 5; i++) { float dx = o[0] - SP[i][0], dz = o[2] - SP[i][1] + 5; if (dx * dx + dz * dz < SP[i][2] * SP[i][2] && o[1] > 0) return mixc(col, 0xFF000000, 0.3f); }
      if (fabsf(o[0]) < 0.5f && o[1] > 0) return mixc(col, 0xFF000000, 0.25f);   // the seam down the middle
      return col;
    }
    case MAT_BEE:
      *gloss = 0.3f;
      return (o[2] > -8.5f && o[2] < -4.5f) || (o[2] > -1 && o[2] < 3) ? 0xFF2A2026 : col;
    case MAT_QBOX: {
      *gloss = 0.3f;
      float u = o[0], v = -o[1];
      if (o[2] < 15.5f) { if (fabsf(o[0]) > 15.5f) u = o[2]; else { u = o[0]; v = -o[2]; } }
      float e = maxf(fabsf(u), fabsf(v));
      if (e > 13.6f) return mixc(col, 0xFF7A4A10, 0.55f);                       // darker frame
      float bolt = minf(minf(sd_circle(u, v, -10.5f, -10.5f, 1.4f), sd_circle(u, v, 10.5f, -10.5f, 1.4f)), minf(sd_circle(u, v, -10.5f, 10.5f, 1.4f), sd_circle(u, v, 10.5f, 10.5f, 1.4f)));
      if (bolt < 0) return mixc(col, 0xFF7A4A10, 0.6f);
      if (o[2] >= 15.5f && col == 0xFFF7B52E) {
        float ang = atan2f(v + 3.5f, u);
        float ring = fabsf(sd_circle(u, v, 0, -3.5f, 4.6f)) - 1.7f;
        if (ang > 1.6f || ang < -2.9f) ring = 99;
        float q = minf(ring, minf(sd_seg(u, v, 0, 1.0f, 0, 3.4f) - 1.7f, sd_circle(u, v, 0, 8, 2.0f)));
        return mixc(col, 0xFFFFFFFF, clampf(0.5f - q, 0, 1));
      }
      return col;
    }
    case MAT_COIN: {
      *gloss = 0.9f;
      float r = sqrtf(o[0] * o[0] + o[1] * o[1]);
      if (fabsf(o[2]) > 1.0f && r < 10.5f) {
        if (r > 8.2f) return mixc(col, 0xFFB86A08, 0.45f);                      // groove round the rim
        float star = 99;                                                        // a star stamped in the middle
        for (int k = 0; k < 5; k++) {
          float a = k * 1.2566f - 1.5708f, b2 = a + 2.5133f;
          star = minf(star, sd_seg(o[0], -o[1], cosf(a) * 6, sinf(a) * 6, cosf(b2) * 6, sinf(b2) * 6) - 0.9f);
        }
        if (star < 0) return mixc(col, 0xFFFFFFFF, 0.55f);
      }
      return col;
    }
    case MAT_SHINY: *gloss = 0.6f; return col;
  }
  return col;
}

// ---- triangles
typedef struct { float p[3], n[3], o[3]; } V3;   // p: screen x, y and depth; n: which way the surface faces; o: place on the model
static void rt_tri(const V3* a, const V3* b, const V3* c, uint32_t col, int mat) {
  if (a->n[2] < 0 && b->n[2] < 0 && c->n[2] < 0) return;   // faces away from the camera: never seen, so skip it
  float area = (b->p[0] - a->p[0]) * (c->p[1] - a->p[1]) - (b->p[1] - a->p[1]) * (c->p[0] - a->p[0]);
  if (fabsf(area) < 1e-6f) return;
  int x0 = (int)floorf(minf(a->p[0], minf(b->p[0], c->p[0]))), x1 = (int)ceilf(maxf(a->p[0], maxf(b->p[0], c->p[0])));
  int y0 = (int)floorf(minf(a->p[1], minf(b->p[1], c->p[1]))), y1 = (int)ceilf(maxf(a->p[1], maxf(b->p[1], c->p[1])));
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > RT.w - 1) x1 = RT.w - 1;
  if (y1 > RT.h - 1) y1 = RT.h - 1;
  float inv = 1.0f / area;
  // the three weights change by a fixed step from one pixel to the next, so they are stepped, not recalculated
  float sx0 = x0 + 0.5f, sy0 = y0 + 0.5f;
  float r0 = ((b->p[0] - sx0) * (c->p[1] - sy0) - (b->p[1] - sy0) * (c->p[0] - sx0)) * inv, r1 = ((c->p[0] - sx0) * (a->p[1] - sy0) - (c->p[1] - sy0) * (a->p[0] - sx0)) * inv;
  float dx0 = (b->p[1] - c->p[1]) * inv, dy0 = (c->p[0] - b->p[0]) * inv, dx1 = (c->p[1] - a->p[1]) * inv, dy1 = (a->p[0] - c->p[0]) * inv;
  for (int y = y0; y <= y1; y++, r0 += dy0, r1 += dy1) {
    float w0 = r0 - dx0, w1 = r1 - dx1;
    for (int x = x0; x <= x1; x++) {
    w0 += dx0; w1 += dx1;
    float w2 = 1 - w0 - w1;
    if (w0 < -1e-5f || w1 < -1e-5f || w2 < -1e-5f) continue;
    float z = w0 * a->p[2] + w1 * b->p[2] + w2 * c->p[2] + 1000;
    int i = y * RT.w + x;
    if (z <= RT.z[i]) continue;
    float n[3], o[3];
    for (int k = 0; k < 3; k++) { n[k] = w0 * a->n[k] + w1 * b->n[k] + w2 * c->n[k]; o[k] = w0 * a->o[k] + w1 * b->o[k] + w2 * c->o[k]; }
    float l = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    if (l > 0) { n[0] /= l; n[1] /= l; n[2] /= l; }
    if (n[2] < 0) { n[0] = -n[0]; n[1] = -n[1]; n[2] = -n[2]; }
    float gloss;
    uint32_t base = mat_colour(col, mat, o, &gloss);
    // light from the upper left, in front; soft wrap-around shading, a highlight, darker towards the edges
    float d = n[0] * -0.42f + n[1] * 0.68f + n[2] * 0.60f, h = d * 0.5f + 0.5f;
    float lit = 0.62f + 0.48f * h;
    float hx = -0.25f, hy = 0.40f, hz = 0.88f, sp = n[0] * hx + n[1] * hy + n[2] * hz;
    if (sp > 0.8f) { sp *= sp; sp *= sp; sp *= sp; sp *= sp; sp *= sp; sp *= gloss; } else sp = 0;
    float rim = 1 - n[2]; lit *= 1 - 0.30f * rim * rim * rim;
    int r = (int)(((base >> 16) & 255) * lit + 255 * sp), g = (int)(((base >> 8) & 255) * lit + 255 * sp), bl = (int)((base & 255) * lit + 250 * sp);
    if (n[1] < 0) { float sh = -n[1] * 0.18f; r = (int)(r * (1 - sh)); g = (int)(g * (1 - sh * 0.8f)); }   // cool shadow underneath
    RT.z[i] = z;
    RT.c[i] = 0xFF000000 | ((r > 255 ? 255 : r) << 16) | ((g > 255 ? 255 : g) << 8) | (bl > 255 ? 255 : bl);
    }
  }
}
static M3 VIEW;   // model space -> what the camera sees
static M3 CUR_P, CUR_WV; static float CUR_N[9];
static void cof(const float* m, float* c) {
  c[0] = m[4] * m[8] - m[5] * m[7]; c[1] = m[5] * m[6] - m[3] * m[8]; c[2] = m[3] * m[7] - m[4] * m[6];
  c[3] = m[2] * m[7] - m[1] * m[8]; c[4] = m[0] * m[8] - m[2] * m[6]; c[5] = m[1] * m[6] - m[0] * m[7];
  c[6] = m[1] * m[5] - m[2] * m[4]; c[7] = m[2] * m[3] - m[0] * m[5]; c[8] = m[0] * m[4] - m[1] * m[3];
}
// called once per part: W places the whole model, P places this part in it
static void set_mats(const M3* W, const M3* P) {
  CUR_P = *P; CUR_WV = m3_mul(VIEW, *W);
  M3 all = m3_mul(CUR_WV, *P);
  cof(all.m, CUR_N);
}
static void v3_make(V3* v, const float* lp, const float* ln) {
  float wp[3];
  m3_pt(&CUR_P, lp, v->o);
  m3_pt(&CUR_WV, v->o, wp);
  v->p[0] = RT.ox + wp[0] * RT.sc; v->p[1] = RT.oy - wp[1] * RT.sc; v->p[2] = wp[2];
  for (int i = 0; i < 3; i++) v->n[i] = CUR_N[i * 3] * ln[0] + CUR_N[i * 3 + 1] * ln[1] + CUR_N[i * 3 + 2] * ln[2];
}
// spin a profile (pairs of radius, height) round the y axis. W places the whole model, P places this part in it.
#define LATHE_MAX 20
static int LATHE_COARSE;   // 1: use every other profile point (half the triangles), for small models
static void lathe(const M3* W, const M3* P, const float* prof0, int np0, int nseg, int flat, uint32_t col, int mat) {
  static float thin[LATHE_MAX * 2];
  const float* prof = prof0; int np = np0;
  if (LATHE_COARSE && !flat && np0 >= 9) {
    np = 0;
    for (int i = 0; i < np0; i += 2) { thin[np * 2] = prof0[i * 2]; thin[np * 2 + 1] = prof0[i * 2 + 1]; np++; }
    if ((np0 - 1) % 2) { thin[np * 2] = prof0[(np0 - 1) * 2]; thin[np * 2 + 1] = prof0[(np0 - 1) * 2 + 1]; np++; }
    prof = thin;
  }
  static V3 ring[2][LATHE_MAX * 2][33];
  static float CS[33], SN[33]; static int cs_n;
  if (cs_n != nseg) { cs_n = nseg; for (int j = 0; j <= nseg; j++) { CS[j] = cosf(j * 6.28318f / nseg); SN[j] = sinf(j * 6.28318f / nseg); } }
  set_mats(W, P);
  int rows = 0;
  // each profile point gives one row of vertices (or two, with different normals, when "flat" keeps edges sharp)
  for (int i = 0; i < np; i++) {
    for (int side = 0; side < (flat ? 2 : 1); side++) {
      int ia, ib;
      if (flat) { ia = side == 0 ? i - 1 : i; ib = ia + 1; if (ia < 0 || ib >= np) continue; }
      else { ia = i > 0 ? i - 1 : i; ib = i < np - 1 ? i + 1 : i; }
      float dr = prof[ib * 2] - prof[ia * 2], dy = prof[ib * 2 + 1] - prof[ia * 2 + 1];
      float nl = sqrtf(dr * dr + dy * dy); if (nl < 1e-6f) nl = 1;
      float nr = dy / nl, ny = -dr / nl;
      for (int j = 0; j <= nseg; j++) {
        float ca = CS[j], sa = SN[j];
        float lp[3] = { prof[i * 2] * ca, prof[i * 2 + 1], prof[i * 2] * sa }, ln[3] = { nr * ca, ny, nr * sa };
        v3_make(&ring[0][rows][j], lp, ln);
      }
      rows++;
    }
  }
  int step = flat ? 2 : 1;
  for (int r = 0; r + 1 < rows; r += step) for (int j = 0; j < nseg; j++) {
    const V3 *a = &ring[0][r][j], *b = &ring[0][r][j + 1], *c = &ring[0][r + 1][j], *d = &ring[0][r + 1][j + 1];
    rt_tri(a, b, c, col, mat); rt_tri(b, d, c, col, mat);
  }
}
static float PROF_BALL[13 * 2], PROF_EGG[15 * 2], PROF_SPIKE[10 * 2], PROF_COIN[6 * 2] = { 0, -1, 0.84f, -1, 1, -0.55f, 1, 0.55f, 0.84f, 1, 0, 1 };
static void m3d_init(void) {
  for (int i = 0; i < 13; i++) { float a = -1.5708f + i * 3.14159f / 12; PROF_BALL[i * 2] = cosf(a); PROF_BALL[i * 2 + 1] = sinf(a); }
  for (int i = 0; i < 15; i++) {   // Dino's body: an egg with flatter sides, a little wider at the bottom
    float a = -1.5708f + i * 3.14159f / 14, c = maxf(0, cosf(a)), s = sinf(a);
    PROF_EGG[i * 2] = powf(c, 0.62f) * (1 - 0.05f * s); PROF_EGG[i * 2 + 1] = (s < 0 ? -1 : 1) * powf(fabsf(s), 0.66f);
  }
  for (int i = 0; i < 10; i++) {   // a smooth, round-tipped spike
    float t = i / 9.0f; PROF_SPIKE[i * 2] = i == 9 ? 0 : powf(1 - t * t, 0.75f) * (1 - 0.25f * t); PROF_SPIKE[i * 2 + 1] = t;
  }
}
static void ball(const M3* W, float x, float y, float z, float rx, float ry, float rz, uint32_t col, int mat, int nseg) {
  M3 P = m3_mul(m3_trans(x, y, z), m3_scale(rx, ry, rz));
  lathe(W, &P, PROF_BALL, 13, nseg, 0, col, mat);
}
// a rounded stick from a to b
static void stick(const M3* W, const float* a, const float* b, float r, uint32_t col, int nseg) {
  float d[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] }, len = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
  float yaw = atan2f(d[0], d[2]), pit = len > 0 ? acosf(clampf(d[1] / len, -1, 1)) : 0;
  M3 P = m3_mul(m3_trans((a[0] + b[0]) * 0.5f, (a[1] + b[1]) * 0.5f, (a[2] + b[2]) * 0.5f), m3_mul(m3_roty(yaw), m3_mul(m3_rotx(pit), m3_scale(r, len * 0.5f + r, r))));
  lathe(W, &P, PROF_BALL, 13, nseg, 0, col, MAT_PLAIN);
}
// a stub leg: straight sides like a cylinder, with a softly rounded end
static float PROF_STUB[12 * 2];
static void stub(const M3* W, const float* a, const float* b, float r, uint32_t col, int nseg) {
  if (PROF_STUB[1] == 0) for (int i = 0; i < 12; i++) {
    if (i < 4) { PROF_STUB[i * 2] = 1; PROF_STUB[i * 2 + 1] = 1 - i * 0.5f; }                     // straight part, top to near the bottom
    else { float t = (i - 3) / 8.0f * 1.5708f; PROF_STUB[i * 2] = cosf(t) > 0 ? powf(cosf(t), 0.7f) : 0; PROF_STUB[i * 2 + 1] = -0.5f - sinf(t) * 0.5f; }
  }
  float d[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] }, len = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
  float yaw = atan2f(-d[0], -d[2]), pit = len > 0 ? acosf(clampf(-d[1] / len, -1, 1)) : 0;
  float hl2 = len * 0.5f + r * 0.5f;
  M3 P = m3_mul(m3_trans((a[0] + b[0]) * 0.5f, (a[1] + b[1]) * 0.5f - r * 0.5f + r * 0.5f, (a[2] + b[2]) * 0.5f), m3_mul(m3_roty(yaw), m3_mul(m3_rotx(pit), m3_scale(r, hl2 + r * 0.5f, r))));
  lathe(W, &P, PROF_STUB, 12, nseg, 0, col, MAT_PLAIN);
}
static void quad(const M3* W, const float* a, const float* b, const float* c, const float* d, const float* n, uint32_t col, int mat) {
  M3 I = m3_id(); V3 v[4];
  set_mats(W, &I);
  v3_make(&v[0], a, n); v3_make(&v[1], b, n); v3_make(&v[2], c, n); v3_make(&v[3], d, n);
  rt_tri(&v[0], &v[1], &v[2], col, mat); rt_tri(&v[0], &v[2], &v[3], col, mat);
}

// ------------------------------------------------------------ the models
#define CAM_TILT 0.2f     // the camera looks down a little, so you see the tops of things

// Dino: faces +z; "yaw" turns him (0 = looking at you, about 1 = looking right)
static void dino3d(Bmp* b, const DinoPose* p, float yaw, float S) {
  int W = (int)(DN * S);
  rt_begin(W, W, 36 * S, 64 * S, S);
  LATHE_COARSE = S <= 1.5f;
  VIEW = m3_rotx(CAM_TILT);
  FACE.eye = p->eye; FACE.mouth = p->mouth; FACE.mood = p->mood;
  float sy = p->sq, sx = 1.0f / sqrtf(p->sq);
  M3 turn = m3_roty(yaw);
  M3 body = m3_mul(m3_trans(0, 8, 0), m3_mul(m3_rotx(p->tilt), m3_mul(m3_trans(0, p->bob, 0), m3_mul(m3_scale(sx, sy, sx), m3_trans(0, 13.5f, 0)))));
  M3 Wb = m3_mul(turn, body);
  // legs first (they join the body at the hips; on the ground the feet stay planted)
  for (int i = 0; i < 2; i++) {
    float hl[3] = { i ? 5.9f : -5.9f, -8.5f, 0 }, hip[3], foot[3];   // stubs: their outer sides line up with the body
    m3_pt(&body, hl, hip);
    if (p->air) {
      float fl[3] = { hl[0], -9.5f - (7.2f - p->ly[i]) / sy, p->lx[i] / sx };
      m3_pt(&body, fl, foot);
    } else { foot[0] = hl[0]; foot[1] = 4.6f + p->ly[i]; foot[2] = p->lx[i]; }
    stub(&turn, hip, foot, 5.4f, 0xFFF8FAFF, S > 1.5f ? 14 : 10);
  }
  // two smooth spikes on his back
  M3 s1 = m3_mul(m3_trans(0, 7.5f, -5.5f), m3_mul(m3_rotx(-(0.80f - p->spk)), m3_scale(7.2f, 18, 5.4f)));
  M3 s2 = m3_mul(m3_trans(0, -1.5f, -7.5f), m3_mul(m3_rotx(-(1.25f - p->spk * 1.35f)), m3_scale(6.0f, 14, 4.6f)));
  lathe(&Wb, &s1, PROF_SPIKE, 10, S > 1.5f ? 14 : 9, 0, 0xFF5FD04A, MAT_SHINY);
  lathe(&Wb, &s2, PROF_SPIKE, 10, S > 1.5f ? 14 : 9, 0, 0xFF5FD04A, MAT_SHINY);
  if (p->wing > -5) for (int i = 0; i < 2; i++) {
    float sgn = i ? 1 : -1;
    M3 wg = m3_mul(m3_trans(sgn * 7, 2, -5), m3_mul(m3_rotz(sgn * (0.5f + p->wing)), m3_mul(m3_trans(sgn * 8, 0, 0), m3_scale(9, 4.2f, 2))));
    lathe(&Wb, &wg, PROF_BALL, 13, 12, 0, 0xFFEAF4FF, MAT_PLAIN);
  }
  M3 bd = m3_scale(11.5f, 14.5f, 10.6f);
  lathe(&Wb, &bd, PROF_EGG, 15, S > 1.5f ? 28 : 16, 0, 0xFFF8FAFF, MAT_DINO);
  LATHE_COARSE = 0;
  rt_end(b);
}

// beetle: 52x40 picture, feet at (26, 36)
// a little gold crown for the bosses
static void crown3d(const M3* Wm, float x, float y, float z, float r) {
  static const float BAND[4 * 2] = { 1, 0, 1, 1, 0.8f, 1, 0.8f, 0 };
  M3 P = m3_mul(m3_trans(x, y, z), m3_scale(r, r * 0.7f, r));
  lathe(Wm, &P, BAND, 4, 14, 1, 0xFFFFC61E, MAT_SHINY);
  for (int k = 0; k < 5; k++) {
    float a = k * 1.2566f;
    M3 sp = m3_mul(m3_trans(x + cosf(a) * r * 0.85f, y + r * 0.5f, z + sinf(a) * r * 0.85f), m3_scale(r * 0.32f, r * 1.1f, r * 0.32f));
    lathe(Wm, &sp, PROF_SPIKE, 10, 6, 0, 0xFFFFC61E, MAT_SHINY);
  }
}
// kind: 0 green, 1 icy blue, 2 royal red
static void beetle3d(Bmp* b, float ph, float sq, float yaw, int snow, int dead, int t, float S, int crown) {
  rt_begin((int)(52 * S), (int)(40 * S) + (crown ? (int)(10 * S) : 0), 26 * S, 36 * S + (crown ? (int)(10 * S) : 0), S);
  VIEW = m3_rotx(CAM_TILT);
  uint32_t shell = snow == 2 ? 0xFFE8503C : snow ? 0xFF9FD4F2 : 0xFF58C85A;
  float bobb = sinf(ph * 2) * 0.6f, ant = sinf(ph) * 1.5f;
  M3 Wm = m3_mul(m3_roty(yaw), m3_scale(1.0f / sqrtf(sq), sq, 1.0f / sqrtf(sq)));
  for (int k = 0; k < 6; k++) {   // six little legs, walking in turn
    float sgn = k < 3 ? -1 : 1, z = -7 + (k % 3) * 6.5f, a = ph + (k % 3) * 2.094f + (k < 3 ? 0 : 3.14159f);
    float h[3] = { sgn * 7, 4, z }, f[3] = { sgn * 10.5f, 1.4f + maxf(0, cosf(a)) * 2.2f, z + sinf(a) * 2.8f };
    stick(&Wm, h, f, 1.4f, 0xFF4A3830, 6);
  }
  M3 I = m3_trans(0, 3.5f + bobb, -2.5f), dome = m3_mul(I, m3_scale(11, 11.5f, 12.5f));
  lathe(&Wm, &dome, PROF_BALL + 12, 7, 20, 0, shell, MAT_SHELL);                 // the top half of a ball
  ball(&Wm, 0, 4.2f + bobb, -2.5f, 10.6f, 2.2f, 12.1f, 0xFF4A3830, MAT_PLAIN, 14);   // belly
  ball(&Wm, 0, 6.5f + bobb * 0.5f, 10, 5.2f, 5, 5, 0xFFF2C9A0, MAT_PLAIN, 12);       // head
  for (int i = 0; i < 2; i++) {
    float sgn = i ? 1 : -1, ey = 8 + bobb * 0.5f;
    if (dead) ball(&Wm, sgn * 2.7f, ey, 13.6f, 2.2f, 0.7f, 1.5f, 0xFF1E1A22, MAT_PLAIN, 8);
    else {
      ball(&Wm, sgn * 2.7f, ey, 13.4f, 2.4f, 2.4f, 2.0f, 0xFFFFFFFF, MAT_SHINY, 10);
      ball(&Wm, sgn * 2.9f, ey - 0.2f, 15.0f, 1.2f, 1.25f, 0.9f, 0xFF16121A, MAT_SHINY, 8);
      float b0[3] = { sgn * 0.8f, ey + 2.2f, 14.6f }, b1[3] = { sgn * 4.6f, ey + 3.6f, 13.6f };   // grumpy brows
      stick(&Wm, b0, b1, 0.7f, 0xFF1E1A22, 5);
    }
    float a0[3] = { sgn * 2.2f, 10.5f, 10.5f }, a1[3] = { sgn * (4.5f + ant * 0.4f), 16.5f, 13 + ant };
    stick(&Wm, a0, a1, 0.7f, 0xFF4A3830, 5);
    ball(&Wm, a1[0], a1[1], a1[2], 1.5f, 1.5f, 1.5f, 0xFF4A3830, MAT_SHINY, 8);
  }
  if (crown) crown3d(&Wm, 0, 14.2f + bobb, -2.5f, 4.2f);
  (void)t;
  rt_end(b);
}

// bee: 56x52 picture, middle at (28, 28)
static void bee3d(Bmp* b, float fph, float pitch, float yaw, int dead, float S, int crown) {
  rt_begin((int)(56 * S), (int)(52 * S), 28 * S, 28 * S, S);
  VIEW = m3_rotx(CAM_TILT * 0.5f);
  M3 Wm = m3_mul(m3_roty(yaw), m3_rotx(pitch));
  float flap = sinf(fph), sway = sinf(fph) * 0.7f;
  float st0[3] = { 0, 0, -9 }, st1[3] = { 0, -1, -15.5f };
  M3 sting = m3_mul(m3_trans(0, 0, -9.5f), m3_mul(m3_rotx(-1.65f), m3_scale(2.6f, 7, 2.6f)));
  lathe(&Wm, &sting, PROF_SPIKE, 10, 8, 0, 0xFF3A2A22, MAT_PLAIN);
  (void)st0; (void)st1;
  for (int k = 0; k < 6; k++) {
    float sgn = k < 3 ? -1 : 1, z = -4 + (k % 3) * 4.5f;
    float h[3] = { sgn * 4, -6, z }, f[3] = { sgn * 5.5f, -11, z - 1.5f + sway };
    stick(&Wm, h, f, 0.8f, 0xFF3A2A22, 5);
  }
  ball(&Wm, 0, 0, 0, 8.2f, 8.4f, 11.5f, 0xFFFFD23E, MAT_BEE, 18);
  for (int i = 0; i < 2; i++) {
    float sgn = i ? 1 : -1;
    M3 wg = m3_mul(m3_trans(sgn * 3, 6.5f, -1), m3_mul(m3_rotz(-sgn * (0.35f + flap * 0.75f)), m3_mul(m3_rotx(-0.25f), m3_mul(m3_trans(0, 8, 0), m3_scale(4.2f, 8.5f, 1.2f)))));
    lathe(&Wm, &wg, PROF_BALL, 13, 10, 0, 0xFFDDEEFF, MAT_SHINY);
    if (dead) ball(&Wm, sgn * 3.4f, 2, 10.2f, 2.4f, 0.8f, 1.2f, 0xFF16121A, MAT_PLAIN, 8);
    else {
      ball(&Wm, sgn * 3.4f, 2, 9.6f, 2.7f, 2.7f, 2.0f, 0xFFFFFFFF, MAT_SHINY, 10);
      ball(&Wm, sgn * 3.6f, 1.8f, 11.2f, 1.3f, 1.35f, 0.9f, 0xFF16121A, MAT_SHINY, 8);
      float b0[3] = { sgn * 1.2f, 4.6f, 10.6f }, b1[3] = { sgn * 5.6f, 6.2f, 9.2f };
      stick(&Wm, b0, b1, 0.7f, 0xFF16121A, 5);
    }
    float a0[3] = { sgn * 2, 7, 7 }, a1[3] = { sgn * 3.5f, 13, 10 + sway * 0.6f };
    stick(&Wm, a0, a1, 0.6f, 0xFF3A2A22, 5);
    ball(&Wm, a1[0], a1[1], a1[2], 1.3f, 1.3f, 1.3f, 0xFF3A2A22, MAT_SHINY, 6);
  }
  if (crown) crown3d(&Wm, 0, 7.6f, 4, 3.4f);
  rt_end(b);
}

// coin: spun to "frames" angles once, then just copied
#define COIN_FRAMES 24
static Bmp HD_COIN[COIN_FRAMES], HD_QBOX[2];
static void coin3d(Bmp* b, float ang, float S) {
  rt_begin((int)(32 * S), (int)(32 * S), 16 * S, 16 * S, S);
  VIEW = m3_rotx(0.12f);
  M3 Wm = m3_roty(ang), P = m3_mul(m3_rotx(1.5708f), m3_scale(11.5f, 2.2f, 11.5f));
  lathe(&Wm, &P, PROF_COIN, 6, 28, 1, 0xFFFFC61E, MAT_COIN);
  rt_end(b);
}
static void box3d(Bmp* b, int used) {
  rt_begin(40, 40, 20, 22, 1);
  VIEW = m3_rotx(CAM_TILT);
  M3 Wm = m3_mul(m3_roty(0.0f), m3_scale(1, 1, 0.5f));
  uint32_t col = used ? 0xFFA9764A : 0xFFF7B52E;
  float h = 16;
  float A[3] = { -h, -h, h }, B[3] = { h, -h, h }, C[3] = { h, h, h }, D[3] = { -h, h, h };
  float E[3] = { -h, -h, -h }, F[3] = { h, -h, -h }, G2[3] = { h, h, -h }, H[3] = { -h, h, -h };
  float nz[3] = { 0, 0, 1 }, ny[3] = { 0, 1, 0 }, nx[3] = { 1, 0, 0 }, mx[3] = { -1, 0, 0 };
  quad(&Wm, A, B, C, D, nz, col, MAT_QBOX); quad(&Wm, D, C, G2, H, ny, mixc(col, 0xFFFFFFFF, 0.25f), MAT_QBOX);
  quad(&Wm, B, F, G2, C, nx, col, MAT_QBOX); quad(&Wm, E, A, D, H, mx, col, MAT_QBOX);
  rt_end(b);
}
static void m3d_build(void) {
  m3d_init();
  for (int i = 0; i < COIN_FRAMES; i++) coin3d(&HD_COIN[i], i * 3.14159f / COIN_FRAMES, 1);
  box3d(&HD_QBOX[0], 0); box3d(&HD_QBOX[1], 1);
}
