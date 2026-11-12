#!/bin/sh
# Builds the phone/browser version: web/out/amazing-dino.html plus music.wav (music.bin with a 44-byte header).
# Needs Emscripten (emcc) and python3. Run from the wiiu-amazing-dino folder.
set -e
F="-O3 -ffast-math -w -DMUSIC_BIN_PATH=\"x\" -DMUSIC_PTR -DSAVE_PATH=\"/save.txt\" src/game.c web/web_main.c -sMODULARIZE=1 -sSINGLE_FILE=1 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864 -sFORCE_FILESYSTEM=1 -sEXPORTED_RUNTIME_METHODS=FS -sEXPORTED_FUNCTIONS=_malloc,_web_init,_web_tick,_web_draw,_web_audio -sENVIRONMENT=web"
mkdir -p web/out
emcc $F -sEXPORT_NAME=DinoModule -o web/out/dino.js
emcc $F -sWASM=0 -sEXPORT_NAME=DinoModuleJS -o web/out/dinojs.js
python3 - <<'P'
import struct
d = open('music/music.bin', 'rb').read()
h = b'RIFF' + struct.pack('<I', 36 + len(d)) + b'WAVEfmt ' + struct.pack('<IHHIIHH', 16, 1, 1, 32000, 32000, 1, 8) + b'data' + struct.pack('<I', len(d))
open('web/out/music.wav', 'wb').write(h + d)
s = open('web/page.html').read()
s = s.replace('/*DINOJS*/', open('web/out/dino.js').read()).replace('/*DINOJS2*/', open('web/out/dinojs.js').read())
open('web/out/amazing-dino.html', 'w').write(s)
P
