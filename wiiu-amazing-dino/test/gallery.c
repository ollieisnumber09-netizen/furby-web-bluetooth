// Every creature on one sheet, at twice size: out/gallery.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
#define CW 130
#define CH 110
#define NC 6
static uint32_t sheet[NC * CW * 4 * CH];
static void putb(Bmp* b, int i) {
  int col = i % NC, row = i / NC;
  for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) {
    uint32_t c = b->px[y * b->w + x]; int a = c >> 24; if (!a) continue;
    int X = col * CW + 10 + x, Y = row * CH + 4 + y; if (X >= NC * CW || Y >= 4 * CH) continue;
    sheet[Y * NC * CW + X] = lerpc(sheet[Y * NC * CW + X], c | 0xFF000000, a);
  }
}
int main(void) {
  game_init();
  for (int i = 0; i < NC * CW * 4 * CH; i++) sheet[i] = 0xFF7EC8F0;
  Bmp b = { 0 }; int n = 0; float S = 2;
  beetle3d(&b, 0.5f, 1, 0.9f, 0, 0, 0, S, 0); putb(&b, n++);
  beetle3d(&b, 0.5f, 1, 0.9f, 1, 0, 0, S, 1); putb(&b, n++);
  bee3d(&b, 0.5f, 0, 0.9f, 0, S, 0); putb(&b, n++);
  for (int t = E_FROG; t <= E_SNOWMAN; t++) { critter3d(&b, t, 1, 0.9f, 0, S, 0); putb(&b, n++); }
  { DinoPose p; dino_idle_pose(&p, 50); dino3d(&b, &p, 0.6f, 1.4f); putb(&b, n++); }
  FILE* fp = fopen("out/gallery.ppm", "wb"); fprintf(fp, "P6 %d %d 255\n", NC * CW, 4 * CH);
  for (int i = 0; i < NC * CW * 4 * CH; i++) { unsigned char c[3] = { sheet[i] >> 16, sheet[i] >> 8, sheet[i] }; fwrite(c, 1, 3, fp); } fclose(fp);
  printf("%d drawn\n", n);
}
