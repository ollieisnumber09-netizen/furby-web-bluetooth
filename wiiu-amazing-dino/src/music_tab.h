// made by tools/adpcm_enc: where each song sits in music.bin (offset in bytes, samples, first sample)
static const struct { int off, n, first; } MUS_TAB[] = {
  { 0, 731428, -3445 },   // title
  { 365714, 1097142, 29 },   // map
  { 914285, 1536000, -29 },   // grass
  { 1682285, 819200, 2307 },   // wild
  { 2091885, 1181538, -210 },   // cave
  { 2682654, 614400, 2254 },   // falls
  { 2989854, 930908, 1750 },   // snow
  { 3455308, 1280000, 368 },   // ice
  { 4095308, 1024000, -1447 },   // night
  { 4607308, 975238, -60 },   // desert
  { 5094927, 890434, -3822 },   // castle
  { 5540144, 667826, 2667 },   // boss
};
