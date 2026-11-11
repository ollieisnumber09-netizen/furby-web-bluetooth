#include "../src/game.c"
void plat_sfx(int id) {}
int main(int argc, char** argv) {
  int n = atoi(argv[1]), x0 = atoi(argv[2]);
  hd_init(); m3d_build(); load_level(n);
  for (int y = 0; y < LH; y++) { for (int x = x0; x < x0 + 70 && x < LW; x++) putchar(G[y][x]); putchar('\n'); }
  for (int i = 0; i < nents; i++) if (ents[i].x / 16 >= x0 && ents[i].x / 16 < x0 + 70) printf("ent %d at %.0f,%.0f\n", ents[i].type, ents[i].x / 16, ents[i].y / 16);
  return 0;
}
