// Draws the newer creatures in a few poses onto one sheet: out/newcrit.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t sheet[8 * 120 * 4 * 110];
#define SW (8 * 120)
static void putb(Bmp* b, int col, int row) {
  for (int y = 0; y < b->h; y++) for (int x = 0; x < b->w; x++) {
    uint32_t c = b->px[y * b->w + x]; int a = c >> 24; if (!a) continue;
    int X = col * 120 + 10 + x, Y = row * 110 + 4 + y; if (X >= SW || Y >= 4 * 110) continue;
    sheet[Y * SW + X] = lerpc(sheet[Y * SW + X], c | 0xFF000000, a);
  }
}
int main(void) {
  game_init();
  for (int i = 0; i < SW * 440; i++) sheet[i] = 0xFF7EC8F0;
  Bmp b = { 0 };
  float S = 2;
  crab3d(&b, 0, 0, 1.15f, 0, S); putb(&b, 0, 0); free(b.px); b = (Bmp){0};
  crab3d(&b, 1, 1, -1.15f, 0, S); putb(&b, 1, 0); free(b.px); b = (Bmp){0};
  fish3d(&b, 0, 1.15f, 0, S); putb(&b, 2, 0); free(b.px); b = (Bmp){0};
  fish3d(&b, 1.5f, -1.15f, 0, S); putb(&b, 3, 0); free(b.px); b = (Bmp){0};
  jelly3d(&b, 0, 0, 1.15f, 0, S); putb(&b, 4, 0); free(b.px); b = (Bmp){0};
  jelly3d(&b, 1, 2, 1.15f, 0, S); putb(&b, 5, 0); free(b.px); b = (Bmp){0};
  roller3d(&b, 0, 1.15f, 0, S); putb(&b, 6, 0); free(b.px); b = (Bmp){0};
  roller3d(&b, 2, -1.15f, 0, S); putb(&b, 7, 0); free(b.px); b = (Bmp){0};
  roller3d(&b, 4, 1.15f, 0, S); putb(&b, 0, 1); free(b.px); b = (Bmp){0};
  roller3d(&b, 6, 1.15f, 0, S); putb(&b, 1, 1); free(b.px); b = (Bmp){0};
  penguin3d(&b, 0, 0, 1.15f, 0, S); putb(&b, 2, 1); free(b.px); b = (Bmp){0};
  penguin3d(&b, 1.5f, 0, -1.15f, 0, S); putb(&b, 3, 1); free(b.px); b = (Bmp){0};
  penguin3d(&b, 0, 1, 1.15f, 0, S); putb(&b, 4, 1); free(b.px); b = (Bmp){0};
  mole3d(&b, 0, 0, 1.15f, 0, S); putb(&b, 5, 1); free(b.px); b = (Bmp){0};
  mole3d(&b, 0.5f, 0, 1.15f, 0, S); putb(&b, 6, 1); free(b.px); b = (Bmp){0};
  mole3d(&b, 1, 0, 1.15f, 0, S); putb(&b, 7, 1); free(b.px); b = (Bmp){0};
  mole3d(&b, 1, 1, -1.15f, 0, S); putb(&b, 0, 2); free(b.px); b = (Bmp){0};
  bird3d(&b, 0.8f, 0, 1.15f, 0, S, 0); putb(&b, 1, 2); free(b.px); b = (Bmp){0};
  bird3d(&b, -0.8f, 0, -1.15f, 0, S, 0); putb(&b, 2, 2); free(b.px); b = (Bmp){0};
  bird3d(&b, 0, 1, 1.15f, 0, S, 0); putb(&b, 3, 2); free(b.px); b = (Bmp){0};
  snowman3d(&b, 0, 0, 1.15f, 0, S); putb(&b, 4, 2); free(b.px); b = (Bmp){0};
  snowman3d(&b, 0, 1, -1.15f, 0, S); putb(&b, 5, 2); free(b.px); b = (Bmp){0};
  crab3d(&b, 0, 0, 0.3f, 1, S); putb(&b, 6, 2); free(b.px); b = (Bmp){0};
  penguin3d(&b, 0, 0, 0.2f, 0, S); putb(&b, 7, 2); free(b.px); b = (Bmp){0};
  FILE* fp = fopen("out/newcrit.ppm", "wb"); fprintf(fp, "P6 %d %d 255\n", SW, 330);
  for (int i = 0; i < SW * 330; i++) { unsigned char c[3] = { sheet[i] >> 16, sheet[i] >> 8, sheet[i] }; fwrite(c, 1, 3, fp); }
  fclose(fp);
}
