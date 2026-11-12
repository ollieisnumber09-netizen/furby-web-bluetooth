// Packs the recorded songs for the game: each out/<song>.raw (mono 32 kHz s16) becomes 4-bit IMA ADPCM,
// all joined into music.bin, with a table of where each one starts in music_tab.h.
//   adpcm_enc <out music.bin> <out music_tab.h> song1 song2 ...   (reads out/<song>.raw)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
static const int STEP[89] = { 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 };
static const int IDX[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
int main(int argc, char** argv) {
  FILE* fo = fopen(argv[1], "wb"); FILE* ft = fopen(argv[2], "w"); long off = 0;
  fprintf(ft, "// made by tools/adpcm_enc: where each song sits in music.bin (offset in bytes, samples, first sample)\nstatic const struct { int off, n, first; } MUS_TAB[] = {\n");
  for (int a = 3; a < argc; a++) {
    char nm[256]; snprintf(nm, sizeof nm, "out/%s.raw", argv[a]);
    FILE* fi = fopen(nm, "rb"); if (!fi) { fprintf(stderr, "missing %s\n", nm); return 1; }
    fseek(fi, 0, SEEK_END); long n = ftell(fi) / 2; fseek(fi, 0, SEEK_SET);
    int16_t* x = malloc(n * 2); fread(x, 2, n, fi); fclose(fi);
    n &= ~1L;
    int pred = x[0], idx = 0; unsigned char byte = 0; double err = 0;
    for (long i = 0; i < n; i++) {
      int step = STEP[idx], d = x[i] - pred, code = 0;
      if (d < 0) { code = 8; d = -d; }
      if (d >= step) { code |= 4; d -= step; }
      if (d >= step >> 1) { code |= 2; d -= step >> 1; }
      if (d >= step >> 2) code |= 1;
      int dq = step >> 3; if (code & 4) dq += step; if (code & 2) dq += step >> 1; if (code & 1) dq += step >> 2;
      pred += code & 8 ? -dq : dq; if (pred > 32767) pred = 32767; if (pred < -32768) pred = -32768;
      idx += IDX[code]; if (idx < 0) idx = 0; if (idx > 88) idx = 88;
      err += (double)(x[i] - pred) * (x[i] - pred);
      if (i & 1) { byte |= code << 4; fputc(byte, fo); } else byte = code;
    }
    fprintf(ft, "  { %ld, %ld, %d },   // %s\n", off, n, x[0], argv[a]);
    fprintf(stderr, "%s: %ld samples, error rms %.1f\n", argv[a], n, __builtin_sqrt(err / n));
    off += n / 2; free(x);
  }
  fprintf(ft, "};\n"); fclose(ft); fclose(fo);
  return 0;
}
