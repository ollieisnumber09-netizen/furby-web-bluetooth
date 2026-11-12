#include "../src/game.c"
void plat_sfx(int id) {}
int main(int argc, char** argv) {
  game_init();
  int n = atoi(argv[1]) - 1, x0 = atoi(argv[2]), x1 = atoi(argv[3]);
  start_level(n);
  for (int y = 0; y < LH; y++) {
    printf("%2d ", y);
    for (int x = x0; x < x1 && x < LW; x++) {
      char c = G[y][x];
      for (int i = 0; i < nents; i++) if ((int)(ents[i].x / T) == x && (int)((ents[i].y + 4) / T) == y) c = ents[i].type == E_PLAT ? (ents[i].k == 2 ? 'F' : ents[i].k == 1 ? 'L' : 'M') : 'e';
      if (FAKE[y][x]) c = 'h';
      putchar(c == ' ' ? '.' : c);
    }
    putchar('\n');
  }
}
