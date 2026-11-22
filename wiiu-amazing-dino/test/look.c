// Pictures of chosen places: look N COL ROW [file]: out/look.ppm
#include "../src/game.c"
void plat_sfx(int id) {}
static uint32_t fb[FB_W * FB_H];
int main(int argc, char** argv) {
  game_init();
  Input in = { 0 };
  start_level(atoi(argv[1]) - 1); intro_t = 0; lives = 9;
  px = atoi(argv[2]) * T; py = atoi(argv[3]) * T; set_stage(stage_of(px)); inv_t = 0;
  for (int f = 0; f < 3; f++) game_update(&in);
  px = atoi(argv[2]) * T; py = atoi(argv[3]) * T; update_camera(1);
  game_draw(fb);
  FILE* fp = fopen(argc > 4 ? argv[4] : "out/look.ppm", "wb"); fprintf(fp, "P6 %d %d 255\n", FB_W, FB_H);
  for (int i = 0; i < FB_W * FB_H; i++) { unsigned char c[3] = { fb[i] >> 16, fb[i] >> 8, fb[i] }; fwrite(c, 1, 3, fp); } fclose(fp);
  return 0;
}
