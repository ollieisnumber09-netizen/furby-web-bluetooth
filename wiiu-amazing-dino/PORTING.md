# Porting Amazing Dino to a new machine

Rule for every port: **the graphics chip draws the big pictures, the processor only draws what moves.**

The game draws into a 640x360 picture (`game_draw`). Drawing the whole background and every tile
on the processor each frame is what made the Wii U lag on busy screens, so every port must do this:

1. Set `g_gpu = 1` before the first `game_draw`.
2. After each `game_draw(fb)`, call `game_layers(&g, 0)` (see `src/game.h`).
3. If `g.bg` or `g.level` is set, draw underneath, on the graphics chip, in this order:
   - the far background strip (repeats sideways; rows above/below it are its top/bottom row stretched),
   - the sun or moon,
   - the near background strip,
   - the level picture (`g.lvb`, split into slices of at most 1024 px wide), offset by `g.cx`, `g.cy`.
   Re-upload the strips only when `g.bg_ver` changes, and the level picture only when `g.lvb_ver` changes.
4. Lay `fb` on top. Where layers are on, `fb` is see-through in **premultiplied** colour: use a
   premultiplied blend (ONE, ONE_MINUS_SRC_ALPHA), or undo the premultiplying and use normal blending.
   With no layers on (title, map), `fb` is opaque: just copy it.
5. Use two streaming textures for `fb` in turn, so uploading never waits on the last frame.

Working examples: `src/main.c` (Wii U, SDL2: `layers_sync`, `layers_draw`, `overlay`, also for the
GamePad with `game_layers(&g, 1)`) and `web/page.html` + `web/web_main.c` (browser canvases).
`test/gpu.c` checks that laying `fb` over the layers gives the same picture as the processor drawing
everything; build `src/main.c` with `-DDINO_SELFTEST` to check a real SDL renderer the same way.
