// anim.h: Dino and the creatures have no fixed frames. Each one is drawn fresh every frame
// from a "pose" (how squashed, how tilted, where each foot is, how far the spikes swing...),
// and the pose glides smoothly from tick to tick, so every movement is as smooth as the screen.

// ------------------------------------------------------------ Dino
// mood: 0 normal, 2 hurt, 3 angry (headbutt), 4 happy, 5 sad
typedef struct { float sq, tilt, bob, lx[2], ly[2], spk, eye, mouth, wing; int mood, air; } DinoPose;
#define DN 72          // drawing area at scale 1; his feet stand at (36, 64)
static Bmp DSCR, ESCR;

static void bmp_fit(Bmp* b, int w, int h) {
  if (b->w != w || b->h != h) { if (b->px) free(b->px); *b = bmp_new(w, h); }
  else memset(b->px, 0, (size_t)w * h * 4);
}

static void dino_render(Bmp* b, const DinoPose* p, int flip, float S) {
  int W = (int)(DN * S);
  bmp_fit(b, W, W);
#define C(d) cov((d) * S)
  float ct = cosf(p->tilt), st = sinf(p->tilt);
  float sy = p->sq, sx = 1.0f / sqrtf(p->sq), lsx = flip ? -1.0f : 1.0f;
  float cs = cosf(p->spk), ss = sinf(p->spk), cs2 = cosf(p->spk * 1.35f), ss2 = sinf(p->spk * 1.35f);
  float cw = cosf(p->wing), sw = sinf(p->wing);
  // where the two legs join the body and where the feet are
  float hx[2], hy[2], fx2[2], fy2[2];
  for (int i = 0; i < 2; i++) {
    float bx = i ? 4.6f : -4.6f, by = 9.5f;
    float ux = bx * sx, uy = (by - 13.5f) * sy - p->bob;
    hx[i] = ux * ct - uy * st; hy[i] = ux * st + uy * ct - 8;
    if (p->air) {
      float ox = p->lx[i], oy = 7.2f - p->ly[i];
      fx2[i] = hx[i] + ox * ct - oy * st; fy2[i] = hy[i] + ox * st + oy * ct;
    } else { fx2[i] = (i ? 4.6f : -4.6f) + p->lx[i]; fy2[i] = -3.9f - p->ly[i]; }
  }
  for (int y = 0; y < W; y++) for (int x = 0; x < W; x++) {
    float wx = (x + 0.5f) / S - 36, wy = (y + 0.5f) / S - 64;
    if (wx < -34 || wx > 30) continue;
    // into the body's own space: undo the tilt (about the hips) and the squash
    float qx = wx, qy = wy + 8;
    float ux = qx * ct + qy * st, uy = -qx * st + qy * ct;
    float bx = ux / sx, by = (uy + p->bob) / sy + 13.5f;
    float wide = 1 + 0.05f * clampf(by / 14, -1, 1);
    float dbody = sd_rrect(bx / wide, by, 0, 0, 10.5f, 14.5f, 10.2f);
    float dl0 = sd_seg(wx, wy, hx[0], hy[0], fx2[0], fy2[0]) - 3.9f;
    float dl1 = sd_seg(wx, wy, hx[1], hy[1], fx2[1], fy2[1]) - 3.9f;
    // two smooth, rounded spikes on his back, each swinging about its base
    float ax = bx + 6.75f, ay = by + 8.75f;
    float sxa = ax * cs + ay * ss - 6.75f, sya = -ax * ss + ay * cs - 8.75f;
    float ds1 = sd_tri(sxa, sya, -5, -11, -9.5f, -6, -14.5f, -17.5f) - 4.2f;
    float bx2 = bx + 9.25f, by2 = by - 2;
    float sxb = bx2 * cs2 + by2 * ss2 - 9.25f, syb = -bx2 * ss2 + by2 * cs2 + 2;
    float ds2 = sd_tri(sxb, syb, -9, -0.5f, -9, 4, -15.5f, -3.5f) - 3.6f;
    float dsp = minf(ds1, ds2);
    float dwing = 99;
    if (p->wing > -5) {
      float gx = bx + 8, gy = by + 2;
      float rx = gx * cw + gy * sw, ry = -gx * sw + gy * cw;
      dwing = minf(sd_ellipse(rx, ry, -8, -1, 9, 4.2f), sd_ellipse(rx, ry, -6, 2.5f, 6.5f, 3));
    }
    float dwhite = minf(dbody, dl0);
    float dall = minf(minf(dwhite, dl1), minf(dsp, dwing));
    if (dall > 3) continue;
    uint32_t* d = &b->px[y * W + x];
    over(d, OUTLINE, C(dall - 1.7f));
    if (dsp < 1) {
      int first = ds1 <= ds2;
      float g = first ? clampf((sxa + 16) * 0.03f + (sya + 19.5f) * 0.045f, 0, 1) : clampf((sxb + 17.5f) * 0.04f + (syb + 4.5f) * 0.07f, 0, 1);
      uint32_t c = mixc(0xFFA8EE6A, 0xFF2F9440, g);
      c = mixc(c, 0xFF1F6E32, clampf(1 + dsp / 2.2f, 0, 1) * 0.4f);
      over(d, c, C(dsp));
    }
    if (dwing < 2) {
      over(d, OUTLINE, C(dwing - 1.3f));
      over(d, mixc(0xFFFFFFFF, 0xFFBCD6F4, clampf((by + 6) / 12, 0, 1)), C(dwing));
    }
    if (dl1 < 2) {   // the far leg, a little darker
      over(d, OUTLINE, C(dl1 - 1.5f));
      over(d, mixc(0xFFDDE6F4, 0xFF9FB2D2, clampf((wy - hy[1]) / 9, 0, 1)), C(dl1));
    }
    if (dwhite < 2.2f) over(d, OUTLINE, C(dwhite - 1.6f));
    if (dwhite < 1) {
      float s = clampf((lsx * bx * 0.35f + by * 0.6f) / 15 + 0.25f, 0, 1);
      uint32_t c = mixc(0xFFFFFFFF, 0xFFAFC0DC, s * s);
      float edge = clampf(1 + dwhite / 4.5f, 0, 1);
      c = mixc(c, 0xFF9FB2D2, edge * edge * (0.25f + 0.35f * s));
      over(d, c, C(dwhite));
    }
    if (dbody < 0) {
      // face: two tall line eyes and a little "w" mouth
      float e1 = 1.8f, e2 = 6.6f, eyc = -5.5f, eh = 4.2f * maxf(p->eye, 0.02f), de;
      if (p->mood == 2)
        de = minf(minf(sd_seg(bx, by, e1 - 2, eyc - 2.5f, e1 + 2, eyc + 2.5f), sd_seg(bx, by, e1 + 2, eyc - 2.5f, e1 - 2, eyc + 2.5f)),
                  minf(sd_seg(bx, by, e2 - 2, eyc - 2.5f, e2 + 2, eyc + 2.5f), sd_seg(bx, by, e2 + 2, eyc - 2.5f, e2 - 2, eyc + 2.5f))) - 0.85f;
      else if (p->mood == 4)
        de = minf(minf(sd_seg(bx, by, e1 - 2, eyc + 1, e1, eyc - 2), sd_seg(bx, by, e1, eyc - 2, e1 + 2, eyc + 1)),
                  minf(sd_seg(bx, by, e2 - 2, eyc + 1, e2, eyc - 2), sd_seg(bx, by, e2, eyc - 2, e2 + 2, eyc + 1))) - 0.9f;
      else {
        float top = p->mood == 3 ? eyc - eh * 0.4f : eyc - eh;
        de = minf(sd_seg(bx, by, e1, top, e1, eyc + eh), sd_seg(bx, by, e2, top, e2, eyc + eh)) - 1.15f;
        if (p->mood == 3) de = minf(de, minf(sd_seg(bx, by, e1 - 2.4f, eyc - 5.2f, e1 + 1.6f, eyc - 3.2f), sd_seg(bx, by, e2 - 1.6f, eyc - 3.2f, e2 + 2.4f, eyc - 5.2f)) - 0.8f);
        if (p->mood == 5) de = minf(de, minf(sd_seg(bx, by, e1 - 2.6f, eyc - 3.4f, e1 + 1.4f, eyc - 5.6f), sd_seg(bx, by, e2 - 1.4f, eyc - 5.6f, e2 + 2.6f, eyc - 3.4f)) - 0.8f);   // sad eyebrows
      }
      float mx = bx - 0.2f, my = by - 3.4f, dm;
      if (p->mood == 2) dm = sd_seg(mx, my, 1, 1.5f, 7.4f, 1.5f) - 0.8f;
      else if (p->mood == 5) dm = minf(minf(sd_seg(mx, my, 0, 3.0f, 2.1f, 0.6f), sd_seg(mx, my, 2.1f, 0.6f, 4.2f, 2.2f)), minf(sd_seg(mx, my, 4.2f, 2.2f, 6.3f, 0.6f), sd_seg(mx, my, 6.3f, 0.6f, 8.4f, 3.0f))) - 0.75f;   // his squiggly mouth, turned down
      else {
        dm = minf(minf(sd_seg(mx, my, 0, 0, 2.1f, 2.1f), sd_seg(mx, my, 2.1f, 2.1f, 4.2f, 0)),
                  minf(sd_seg(mx, my, 4.2f, 0, 6.3f, 2.1f), sd_seg(mx, my, 6.3f, 2.1f, 8.4f, 0))) - 0.75f;
        if (p->mouth > 0.05f) {
          // open: the "w" on top with a round "U" hanging under it
          float m = p->mouth, tw = fmodf(mx + 42, 4.2f), yw = 2.1f * (1 - fabsf(tw / 2.1f - 1));
          float du = sd_ellipse(mx, my, 4.2f, 0.6f, 4.3f, 1.2f + 5.2f * m);
          if (my > yw - 0.2f && mx > -0.3f && mx < 8.7f) {
            over(d, 0xFFE8707E, C(du) * minf(1, m * 3));
            dm = minf(dm, fabsf(du) - 0.7f);
          }
        }
      }
      over(d, OUTLINE, C(minf(de, dm)));
    }
  }
#undef C
}

// the moving parts of Dino's pose, carried from one tick to the next
static struct { float sq, sqv, tilt, bob, ph, lx[2], ly[2], spk, spkv, mouth, wing; int wasg, mood, air; float vy0, yaw; } DA = { 1 };

static void dino_tick(void) {
  float sp = fabsf(pvx) / WALK_V; if (sp > 1) sp = 1;
  float run = clampf((fabsf(pvx) - WALK_V) / (RUN_V - WALK_V), 0, 1);   // 0 walking .. 1 full sprint
  float tsq = 1, ttilt = 0, tbob = 0, tmouth = 0, tl[2][2] = { { 0, 0 }, { 0, 0 } };
  int walling = !on_ground && wall_dir && pvy > 0 && !dead_t;
  int butting = atk_t > 4 && power != 2;
  DA.mood = 0; DA.air = !on_ground;
  if (dead_t) {
    DA.mood = 2; DA.air = 1; tsq = 1.05f;
    tl[0][0] = -3; tl[1][0] = 3; tl[0][1] = tl[1][1] = 1.5f;
  } else {
    if (fabsf(DA.tilt) > 1.5f) DA.tilt = 0;   // back upright after a tumble
    if (butting) {
      ttilt = 0.8f; tsq = 1.1f; DA.mood = 3; DA.air = 1;
      tl[0][0] = -4; tl[1][0] = -2; tl[0][1] = 0.5f; tl[1][1] = 1.5f;
    } else if (walling) {
      ttilt = -0.14f; tl[0][0] = -2; tl[1][0] = -1; tl[0][1] = 0.5f; tl[1][1] = 3.5f; tmouth = 0.3f;
    } else if (!on_ground) {
      if (pvy < 0) { tsq = 1.12f; ttilt = -0.07f; tl[0][0] = -2.5f; tl[1][0] = 1.5f; tl[0][1] = 2.5f; tl[1][1] = 4.2f; tmouth = 1; }
      else {
        tsq = 1.05f; ttilt = 0.1f; tmouth = 0.5f;
        tl[0][0] = -2.5f + sinf(frame * 0.45f) * 1.6f; tl[1][0] = 2.5f - sinf(frame * 0.45f) * 1.6f; tl[0][1] = tl[1][1] = 0.5f;
      }
    } else if (sp > 0.04f) {
      // a slow, flat shuffle when walking; when sprinting, quicker longer strides and a bounce in every step
      DA.ph += fabsf(pvx) * (0.13f + 0.04f * run);
      for (int i = 0; i < 2; i++) {
        float a = DA.ph + i * 3.14159f;
        tl[i][0] = (sinf(a) * (5.0f + 3.0f * run) + (i ? -2.4f : 2.4f)) * sp; tl[i][1] = maxf(0, cosf(a)) * (2.2f + 2.5f * run) * sp;
      }
      tbob = (0.45f + 2.8f * run) * sp * fabsf(cosf(DA.ph));
      ttilt = 0.06f * sp + 0.16f * run; tsq = 1 + (0.012f + 0.025f * run) * sinf(DA.ph * 2) * sp;
      if (atk_t > 0) ttilt = 0.25f;
    } else {
      tsq = 1 + 0.025f * sinf(frame * 0.06f);   // breathing
      if (state == ST_CLEAR) { DA.mood = 4; tmouth = 1; tbob = 3.5f * fabsf(sinf(state_t * 0.16f)); }
    }
    if (inv_t > 70) DA.mood = 2;
    if (on_ground && !DA.wasg) DA.sqv -= 0.05f + clampf(DA.vy0, 0, 6) * 0.022f;   // squash on landing
    if (!on_ground && DA.wasg && pvy < 0) DA.sqv += 0.07f;                       // stretch on take-off
  }
  DA.sqv += (tsq - DA.sq) * 0.22f; DA.sqv *= 0.72f; DA.sq = clampf(DA.sq + DA.sqv, 0.72f, 1.28f);
  if (!dead_t) DA.tilt += (ttilt - DA.tilt) * 0.25f;
  DA.bob += (tbob - DA.bob) * 0.4f;
  DA.mouth += (tmouth - DA.mouth) * 0.3f;
  { float fo = 0.4f + 0.3f * run; for (int i = 0; i < 2; i++) { DA.lx[i] += (tl[i][0] - DA.lx[i]) * fo; DA.ly[i] += (tl[i][1] - DA.ly[i]) * fo; } }
  // the spikes lag behind: they lift when he falls, press down when he rises, and bounce as he runs
  float tspk = clampf(pvy * 0.08f, -0.45f, 0.6f);
  if (on_ground) tspk = sp > 0.04f ? sinf(DA.ph * 2 + 1) * (0.05f + 0.08f * run) * sp - 0.05f * sp : sinf(frame * 0.05f) * 0.05f;
  if (butting) tspk = -0.35f;
  DA.spkv += (tspk - DA.spk) * 0.16f; DA.spkv *= 0.8f; DA.spk += DA.spkv;
  DA.wing = power == 1 ? (on_ground ? 0.25f + sinf(frame * 0.12f) * 0.2f : sinf(frame * 0.9f) * 0.8f) : -9;
  // which way he is turned: he swings round through facing you, like a real model would
  float tyaw = face * 0.75f;
  if (walling) tyaw = wall_dir > 0 ? -0.8f : 0.8f;
  if (butting) tyaw = face * 1.05f;
  if (state == ST_CLEAR && on_ground) tyaw = 0.2f;
  if (dead_t) { DA.yaw += 0.3f; DA.tilt = -0.4f; }
  else if (twirl_t > 0) {   // the twirl: one full spin, ending facing the same way
    DA.yaw = face * 0.75f + 6.28318f * (TWIRL_T - twirl_t) / TWIRL_T;
    if (DA.yaw > 3.14159f) DA.yaw -= 6.28318f;
  }
  else if (snap_turn) DA.yaw = tyaw;   // turned round while running: face the new way at once
  else { if (fabsf(DA.yaw) > 2) DA.yaw = fmodf(DA.yaw, 6.28318f) > 3.14159f ? 0 : DA.yaw > 0 ? 1 : -1; DA.yaw += (tyaw - DA.yaw) * 0.22f; }
  DA.wasg = on_ground; DA.vy0 = pvy; snap_turn = 0;
}
static void dino_pose(DinoPose* p) {
  int k = frame % 230;
  p->sq = DA.sq; p->tilt = DA.tilt; p->bob = DA.bob; p->spk = DA.spk; p->mouth = DA.mouth; p->wing = power == 1 ? DA.wing : -9; p->mood = DA.mood; p->air = DA.air;
  for (int i = 0; i < 2; i++) { p->lx[i] = DA.lx[i]; p->ly[i] = DA.ly[i]; }
  p->eye = k < 10 ? fabsf(k - 5) / 5.0f : (k >= 24 && k < 32 ? fabsf(k - 28) / 4.0f : 1);
}
// a calm standing pose for the title, the map and the HUD
static void dino_idle_pose(DinoPose* p, int t) {
  memset(p, 0, sizeof *p);
  int k = t % 230;
  p->sq = 1 + 0.025f * sinf(t * 0.06f); p->spk = sinf(t * 0.05f) * 0.06f; p->wing = -9;
  p->eye = k < 10 ? fabsf(k - 5) / 5.0f : 1;
}

// ------------------------------------------------------------ beetle
// ph: walk cycle, sq: 1 normal .. 0.3 squashed flat, xs: 1 normal .. 0 edge-on while turning round
static void beetle_render(Bmp* b, float ph, float sq, float xs, int snow, int dead, int t) {
  bmp_fit(b, 52, 40);
  uint32_t lite = snow ? 0xFFEAF8FF : 0xFFB2F27E, mid = snow ? 0xFF8FC7E8 : 0xFF4DB85A, dark = snow ? 0xFF4F86B4 : 0xFF237A40;
  float bobb = sinf(ph * 2) * 0.7f, wide = 1.0f / sqrtf(sq);
  float ant = sinf(ph * 0.7f) * 1.6f;
  int bl = t % 170; float eye = dead ? 1 : (bl < 8 ? fabsf(bl - 4) / 4.0f : 1);
  float lxk[3], lyk[3];
  for (int k = 0; k < 3; k++) { float a = ph + k * 2.094f; lxk[k] = sinf(a) * 2.8f; lyk[k] = maxf(0, cosf(a)) * 2.2f; }
  if (xs < 0.12f) xs = 0.12f;
  for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) {
    float fx = (x + 0.5f - 26) / (xs * wide), fy = (y + 0.5f - 36) / sq;
    float dshell = maxf(sd_ellipse(fx, fy, -1, -4.5f - bobb, 12.5f, 13.5f), fy + 4 + bobb);
    float dhead = sd_circle(fx, fy, 11.5f, -7.2f - bobb * 0.5f, 5.2f);
    float dleg = 99;
    for (int k = 0; k < 3; k++) { float lx = -8 + k * 7; dleg = minf(dleg, sd_seg(fx, fy, lx, -5, lx + lxk[k], -1.5f - lyk[k]) - 1.5f); }
    float dant = minf(sd_seg(fx, fy, 12.5f, -11.5f - bobb * 0.5f, 16.5f + ant, -17), sd_seg(fx, fy, 10, -12 - bobb * 0.5f, 12.5f + ant * 0.8f, -18.5f)) - 0.7f;
    dant = minf(dant, minf(sd_circle(fx, fy, 16.5f + ant, -17, 1.5f), sd_circle(fx, fy, 12.5f + ant * 0.8f, -18.5f, 1.5f)));
    float dall = minf(minf(dshell, dhead), minf(dleg, dant));
    if (dall > 3) continue;
    uint32_t* d = &b->px[y * b->w + x];
    over(d, OUTLINE, cov(dall - 1.7f));
    over(d, 0xFF3A2A22, cov(minf(dleg, dant)));
    if (dhead < 1) {
      over(d, mixc(0xFFF2C9A0, 0xFFB98258, clampf((fy + 12) / 9, 0, 1)), cov(dhead));
      float ey = -8.4f - bobb * 0.5f;
      if (dead) over(d, 0xFFFFFFFF, cov(minf(sd_seg(fx, fy, 11.6f, ey - 1.6f, 14.8f, ey + 1.6f), sd_seg(fx, fy, 14.8f, ey - 1.6f, 11.6f, ey + 1.6f)) - 0.7f));
      else {
        over(d, 0xFFFFFFFF, cov(sd_ellipse(fx, fy, 13.2f, ey, 2.5f, 2.5f * maxf(eye, 0.12f))));
        if (eye > 0.4f) over(d, 0xFF111111, cov(sd_circle(fx, fy, 14.0f, ey + 0.3f, 1.15f)));
        over(d, 0xFF111111, cov(sd_seg(fx, fy, 10.6f, ey - 3.3f, 15.2f, ey - 1.9f) - 0.7f));   // grumpy brow
      }
    }
    if (dshell < 2) over(d, OUTLINE, cov(dshell - 1.5f));
    if (dshell < 1) {
      float s = clampf((fx * 0.45f + (fy + 5) * 0.95f) / 14 + 0.62f, 0, 1);
      uint32_t c = s < 0.5f ? mixc(lite, mid, s * 2) : mixc(mid, dark, s * 2 - 1);
      c = mixc(c, dark, clampf(1 + dshell / 3, 0, 1) * 0.35f);
      if (sd_circle(fx, fy, -5.5f, -9 - bobb, 2.3f) < 0 || sd_circle(fx, fy, 4.5f, -8 - bobb, 1.9f) < 0 || sd_circle(fx, fy, -1, -13 - bobb, 1.6f) < 0) c = mixc(c, dark, 0.55f);
      over(d, c, cov(dshell));
      over(d, 0xFFFFFFFF, cov(sd_ellipse(fx, fy, -5, -13.5f - bobb, 4.2f, 1.9f)) * 0.75f);
    }
  }
}

// ------------------------------------------------------------ bee
static void bee_render(Bmp* b, int t, float tilt, int dead) {
  bmp_fit(b, 56, 52);
  float ct = cosf(tilt), st = sinf(tilt);
  float flap = sinf(t * 1.25f);                        // wings beat about twelve times a second
  float a1 = -0.55f + flap * 0.75f, a2 = 0.15f + flap * 0.6f;
  float c1 = cosf(a1), s1 = sinf(a1), c2 = cosf(a2), s2 = sinf(a2);
  float sway = sinf(t * 0.2f) * 1.2f;
  int bl = t % 150; float eye = bl < 8 ? fabsf(bl - 4) / 4.0f : 1;
  for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) {
    float wx = x + 0.5f - 28, wy = y + 0.5f - 28;
    float fx = wx * ct + wy * st, fy = -wx * st + wy * ct;
    float dbody = sd_ellipse(fx, fy, 0, 2, 11.5f, 8.6f);
    float dst = sd_tri(fx, fy, -10, 0, -17, 2.5f, -10, 5);
    float g1x = fx + 3, g1y = fy + 5, w1x = g1x * c1 + g1y * s1, w1y = -g1x * s1 + g1y * c1;
    float g2x = fx - 2, g2y = fy + 5, w2x = g2x * c2 + g2y * s2, w2y = -g2x * s2 + g2y * c2;
    float dw = minf(sd_ellipse(w1x, w1y, 0, -8, 4.4f, 8), sd_ellipse(w2x, w2y, 0, -8, 4.4f, 8));
    float dleg = 99;
    for (int k = 0; k < 3; k++) { float lx = -5 + k * 5; dleg = minf(dleg, sd_seg(fx, fy, lx, 9, lx - 1.5f + sway, 13.5f) - 0.9f); }
    float dant = minf(sd_seg(fx, fy, 7, -5, 11 + sway * 0.6f, -11), sd_seg(fx, fy, 4, -6, 6.5f + sway * 0.6f, -12.5f)) - 0.7f;
    float dall = minf(minf(dbody, dst), minf(dw, minf(dleg, dant)));
    if (dall > 3) continue;
    uint32_t* d = &b->px[y * b->w + x];
    over(d, OUTLINE, cov(dall - 1.6f));
    over(d, 0xFF3A2A22, cov(minf(dst, minf(dleg, dant))));
    if (dw < 1) over(d, mixc(0xFFFFFFFF, 0xFFBFE0FF, clampf((fy + 16) / 14, 0, 1)), cov(dw) * 0.88f);
    if (dbody < 2) over(d, OUTLINE, cov(dbody - 1.4f));
    if (dbody < 1) {
      float s = clampf((fx * 0.3f + (fy - 2) * 0.9f) / 9 + 0.5f, 0, 1);
      uint32_t c = mixc(0xFFFFF07A, 0xFFE89A1A, s);
      if (fabsf(fx + 1.5f) < 2.1f || fabsf(fx + 7.5f) < 1.9f) c = mixc(0xFF4A3A30, 0xFF1E1A22, s);
      c = mixc(c, 0xFF8A4A10, clampf(1 + dbody / 3, 0, 1) * 0.3f);
      over(d, c, cov(dbody));
      over(d, 0xFFFFFFFF, cov(sd_ellipse(fx, fy, -1, -3.5f, 5.5f, 1.5f)) * 0.5f);
      if (dead) over(d, 0xFF111111, cov(minf(sd_seg(fx, fy, 4.5f, -2, 8.5f, 2), sd_seg(fx, fy, 8.5f, -2, 4.5f, 2)) - 0.8f));
      else {
        over(d, 0xFFFFFFFF, cov(sd_ellipse(fx, fy, 6.5f, 0, 2.7f, 2.7f * maxf(eye, 0.12f))));
        if (eye > 0.4f) over(d, 0xFF111111, cov(sd_circle(fx, fy, 7.3f, 0.3f, 1.25f)));
        over(d, 0xFF111111, cov(sd_seg(fx, fy, 3.8f, -3.8f, 8.8f, -2.2f) - 0.7f));
      }
    }
  }
}

// ------------------------------------------------------------ flags
// a flag that ripples in the wind: X, Y is where it joins the pole
static void fb_flag(float X, float Y, int len, int hgt, uint32_t c0, uint32_t c1, float wind, int emblem) {
  for (int i = 0; i < len; i++) {
    float k = i / (float)len, a = i * 0.24f - frame * 0.17f;
    float off = sinf(a) * 3.2f * k * wind + (1 - wind) * k * k * hgt * 0.7f, sh = cosf(a) * 0.16f * wind;
    float h2 = hgt * (1 - 0.18f * k) * (wind > 0.5f ? 1 : 1 - 0.3f * k);
    for (int j = -2; j < hgt + 3; j++) {
      float fy = j + 0.5f, dd = maxf(-fy, fy - h2);
      if (i > len - 3) dd = maxf(dd, (i - (len - 3)) + 0.5f - 1.5f);
      if (dd > 2) continue;
      int px2 = (int)(X + i), py2 = (int)(Y + j + off);
      fb_over(px2, py2, OUTLINE, cov(dd - 1.3f));
      uint32_t c = mixc(c0, c1, fy / h2);
      if (emblem && sd_circle(i + 0.5f, fy, len * 0.42f, h2 * 0.5f, hgt * 0.26f) < 0) c = 0xFFFFF4C2;
      c = sh > 0 ? mixc(c, 0xFFFFFFFF, sh) : mixc(c, 0xFF000000, -sh);
      fb_over(px2, py2, c, cov(dd));
    }
  }
}

#include "m3d.h"
