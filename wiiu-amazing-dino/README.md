# Amazing Dino (Wii U)

An original platformer starring Dino (white egg body, smooth green spikes). Walk the world maps
from Dino's house through 24 levels in 7 worlds: Jungle Island, Frosty Forest, Ice Mountain,
Wild Jungle, Snowflake Peaks, Sunny Dunes and Cloud Castle. Clearing a level opens the next one,
and seven bosses guard the way. The later worlds add moving platforms, frogs, spiky balls,
seed-spitting flowers, bats, pretend walls hiding secret rooms and cracked blocks.
Stomp or headbutt beetles and bees, grab coins (50 coins = 1 extra life).
? boxes (jump into them from below, or headbutt them) hold 5 coins or a power-up:
wings (double jump), a sword (X/Y swings it), a helmet (headbutts smash cracked blocks and
spiky balls) or a star (X/Y throws bouncing stars). Getting hit loses the power-up instead of a life.
Progress saves to `SD card/wiiu/apps/AmazingDino/save.txt` (an old Canopy Dino save is picked up the first time).

## Put it on the Wii U (Aroma)
Copy `AmazingDino.wuhb` to `SD card/wiiu/apps/`, put the card back and start "Amazing Dino" from the Wii U Menu.
In Cemu: File > Load and pick `AmazingDino.wuhb`.

## Controls
| | GamePad / Pro / Classic | Wii Remote sideways |
|---|---|---|
| Move | Left stick or D-pad | D-pad |
| Jump (hold = higher) | A or B | 2 |
| Headbutt dash (or sword swing) | X or Y | 1 |
| Wall-jump | Jump into a wall, then jump again | same |
| Map: walk / enter level | Stick or D-pad / A | D-pad / 2 |
| Pause | + | + |
| Back to the map | Pause, then Down | same |

Nunchuk works too (stick to move, A to jump, B to headbutt). HOME leaves the game.

## Build
Needs devkitPro with wut and wiiu-sdl2 (or the `devkitpro/devkitppc` Docker image):
`docker run --rm -v "$PWD":/src -w /src devkitpro/devkitppc make`

`src/game.c` is the whole game (art is drawn from text in the code, music and sounds are made
on the fly). `src/main.c` is the Wii U side. Levels are the `L1`/`L2`/`L3` lists near the top
of game.c, so adding a level is just adding another list.
`test/video.c` records a gameplay video with an autopilot. `test/test.c` runs the game on a PC without a screen and saves screenshots
(`gcc -o kiptest test/test.c -lm && ./kiptest .`).

## Phone and browser version

`web/build.sh` builds the same game for a browser with on-screen touch buttons (needs Emscripten). It writes `web/out/amazing-dino.html` and `music.wav`, which must sit next to each other.
