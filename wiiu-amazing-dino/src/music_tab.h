// made by tools/adpcm_enc: where each song sits in music.bin (offset in bytes, samples, first sample)
static const struct { int off, n, first; } MUS_TAB[] = {
  { 0, 1755428, 850 },   // title
  { 877714, 1645714, -556 },   // map
  { 1700571, 2304000, -3301 },   // grass
  { 2852571, 1638400, 343 },   // wild
  { 3671771, 1772308, 296 },   // cave
  { 4557925, 1228800, 2325 },   // falls
  { 5172325, 1861818, -100 },   // snow
  { 6103234, 1920000, 3348 },   // ice
  { 7063234, 1536000, -635 },   // night
  { 7831234, 1462856, 795 },   // desert
  { 8562662, 1335652, -1238 },   // castle
  { 9230488, 1001738, 1385 },   // boss
  { 9731357, 1335652, 364 },   // water
  { 10399183, 1861818, -1411 },   // beach
};
