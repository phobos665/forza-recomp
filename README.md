# Forza Motorsport (Xbox) — static recompilation

A native build of Microsoft Game Studios' *Forza Motorsport* (Xbox, 2005;
title ID `0x4D53006E`) made with the
[DirectXbox-Recomp](https://github.com/phobos665/DirectXbox-Recomp) toolkit.
The game's own code is lifted to C from the XBE and runs natively; Direct3D 8
and DirectSound are replaced at the API boundary.

**No game code or data is in this repository.** You need the files from a
copy of the disc you own.

## Current State

Early bring-up. On macOS (Apple Silicon) it boots, fills its cache
partitions from the disc, plays the intro movie with its music, creates a
profile, and goes through the main menu and car select into a race, with the
track, cars and HUD drawn. The front end runs at 60 fps; a race costs about
18 ms a frame on an M-series Mac, so it runs a little under 60.

Forza does its profile and save work on an XAPI fiber. The toolkit replaces
`SwitchToFiber` with host fibers (`src/hle/hle_xapi_fiber.c`); a toolkit
without that on your platform stops presenting frames after "New profile".

- **Widescreen** (`widescreen = 1`, launcher or `RECOMP_WIDESCREEN=1`). Forza
  has a 16:9 mode of its own: races fill the 16:9 picture with a wider view
  and the HUD sits at the 16:9 edges, as the game shipped it. Every frame is
  16:9. Movies are shown in proportion between bars. The menus are laid out
  for 16:9 as they load: the frame (bands, headings, button prompts) runs to
  the edges, and the background art is widened from your own disc, its
  stripes carried on rather than stretched (`layout/widescreen.txt`,
  `src/fz_layout.c`, `src/fz_backdrop.c`). Some screens still have pieces at
  4:3: the track loading screen's panel and Select Car's lists.

Not yet checked: a race to the finish, the other modes, saving and loading,
and the sound by ear.

## Layout

```
forza-recomp/
  CMakeLists.txt          the project; pulls in ../xboxrecomp
  src/main.c              entry point, game folder lookup (from the toolkit's template)
  src/recomp_manual.c     the title's own overrides and hooks (pool, widescreen)
  src/fz_layout.c         menu layouts for 16:9: rules applied as each screen loads
  src/fz_backdrop.c       menu backgrounds widened for 16:9 when the game loads them
  layout/widescreen.txt   the 16:9 layout rules and background recipes
  src/recomp/gen/         lifted code (generated, not committed)
  scripts/recompile.sh    runs the toolkit's lift pipeline for this game
  game/                   your game files (not committed)
```

`recomp_manual.c` replaces the two functions that take and give back a page
of Forza's small-object pool, so that every pool page keeps a 64 KB window to
itself. `operator delete` tells pool slots from heap blocks by one bit per
64 KB region, which holds on the console and not here; without the overrides
the heap is corrupt by the first race load. The comment above them has the
details.

## Building

You need the toolkit checked out beside this repository, at
`../xboxrecomp`, at a commit with the fixes this game needs. Until they are
on the toolkit's `main`, that is its `title/forza` branch. Point
`XBOXRECOMP_DIR` elsewhere to use another checkout,
both for `scripts/recompile.sh` and for CMake (`-DXBOXRECOMP_DIR=...`).

The toolkit's own requirements apply: see its `docs/GETTING_STARTED.md`.

1. **Game files.** Extract the disc into `game/`, so that `game/default.xbe`
   exists.
2. **XDK symbols.** From the toolkit (it builds the XbSymbolDatabase CLI in
   `third_party/XbSymbolDatabase/build`):

   ```bash
   .venv/bin/python -m tools.xdk_symbols ../forza-recomp/game/default.xbe
   ```
3. **Lift.** `scripts/recompile.sh` (about a minute; 20,585 functions).
4. **Build.**

   ```bash
   cmake -S . -B build/Release -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build/Release --target forza_recomp forza_launcher --parallel
   ```

## Running

```bash
build/Release/forza_launcher      # settings, then the game
build/Release/forza_recomp        # the game directly
```

The first boot copies the game's archives to the cache partitions, which
takes a minute or two before the intro movie starts. F9 shows the frame rate,
F10 steps the frame cap, F11 saves a capture of the frame on screen.
