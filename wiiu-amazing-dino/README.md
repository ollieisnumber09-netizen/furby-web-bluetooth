# Amazing Dino (Wii U)

An original platformer starring Dino (white egg body, smooth green spikes). Walk the world maps
from Dino's house through 56 levels in 7 worlds of 8: Jungle Island, Coral Coast, Frosty Forest,
Sunny Dunes, Wild Jungle, Snowflake Peaks and Cloud Castle. Clearing a level opens the next one,
and the last level of each world is a boss. Levels are long and built in sections: moving and
crumbly platforms, pools and whole underwater levels (tap jump to swim), pretend walls hiding
secret rooms, cracked blocks, and three key pieces per level that make a key for the locked door
to a bonus room. Creatures include beetles, bees, frogs, spiky balls, seed-spitting flowers, bats,
crabs, fish, jellyfish (never stomp them), armadillos that curl up and roll, sliding penguins,
moles that throw dirt, diving birds and snowmen that throw snowballs.
Stomp or headbutt them, grab coins (50 coins = 1 extra life).
? boxes (jump into them from below, or headbutt them) hold 5 coins or a power-up:
wings (double jump), a sword (X/Y swings it), a helmet (headbutts smash cracked blocks and
spiky balls) or a star (X/Y throws bouncing stars). The sword, helmet and star all show on Dino.
Picking up a new power-up while wearing one puts the old one in the slot on the GamePad; tap the slot
(or press -) to drop it back down. Hold ZL or ZR to sprint, press
L or R in the air to twirl (one spin that hangs for 0.3 seconds), and hold jump to jump higher.
The circle in the top corner is Dino's health: a hit knocks off his power-up if he has one, otherwise
it takes one of the three slots, and losing the last one costs a life. Grabbing the power-up he already
has gives one slot back. Leaving a level any way but the flag leaves behind the coins, lives and
power-ups found in it. Green bubbles with a little Dino inside are 1-ups. Dino is safe for a moment after respawning and after grabbing a power-up. Each kind of area has its own song (14 in all).
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
on the fly). `src/main.c` is the Wii U side. Levels are made by `build_level()` in game.c from
the level table (name, theme, seed) and a set of section builders.
`test/video.c` records a gameplay video with an autopilot. `test/test.c` runs the game on a PC without a screen and saves screenshots
(`gcc -o kiptest test/test.c -lm && ./kiptest .`).

## Phone and browser version

`web/build.sh` builds the same game for a browser with on-screen touch buttons (needs Emscripten). It writes `web/out/amazing-dino.html` and `music.wav`, which must sit next to each other.
