# uira

> *uira* — Rapa Nui for **rayo / relámpago** (lightning).
> Drawing fast pixels feels close enough.

A small [raylib](https://www.raylib.com/) FFI binding for
[kaikai](https://kaikai-lang.org/), built on the FFI v2 features
(struct-by-value, fixed-width boundary types). Ships the binding
as a reusable module plus ten demos that exercise it.

The binding covers the slice of raylib needed for 2D games and
visualisations: window lifecycle, frame loop, basic shapes,
text, input, random, audio (SFX + music), and a handful of math
helpers. `Color`, `Vector2` and `Rectangle` cross the boundary
by value as `extern "C" type` records whose layout matches
raylib's, so the bindings link straight to raylib with no C shim.
The exceptions are fonts and audio: raylib's `Font`, `Sound` and
`Music` hold pointers, so each goes through a tiny slot-table
shim (`ffi/raylib_fonts.c`, `ffi/raylib_audio.c`). See
[docs/ffi-v2-migration.md](docs/ffi-v2-migration.md) for the
design.

## What's in the box

```
uira/
├── ffi/
│   ├── raylib.kai        # kaikai-side bindings (raylib direct)
│   ├── raylib_fonts.c    # font slot-table shim (Font isn't by-value)
│   └── raylib_audio.c    # audio slot-table shim (Sound/Music aren't by-value)
├── conway.kai            # demo: Game of Life
├── boids.kai             # demo: Reynolds flocking
├── mandelbrot.kai        # demo: Mandelbrot explorer (pan + zoom)
├── snake.kai             # demo: classic snake
├── solar.kai             # demo: solar system (orbits)
├── portfolio.kai         # demo: P&L dashboard
├── orderbook.kai         # demo: matching engine (actors)
├── observatory.kai       # demo: N-body / HR diagram / parallax
├── audiopad.kai          # demo: one-octave synth keyboard (audio)
├── kaikai.kai            # demo: morphing logo
├── Makefile
└── README.md
```

## Prerequisites

- macOS (tested) or Linux. Windows untested.
- `cc` (clang or gcc) with C99.
- `pkg-config`.
- raylib 5.x. On macOS: `brew install raylib`.
- kaikai 0.91+. On macOS:
  `brew install lnds/kaikai/kaikai`. The Makefile picks up
  whatever `kai` is on `$PATH`; override to point at a sibling
  checkout with `make KAI_BIN=../kaikai/bin/kai`. The build
  forces `--backend=c`: struct-by-value FFI isn't on the native
  backend yet.

## Build & run

```sh
make                  # all 10 demos -> ./build/
make conway           # one at a time
make run-snake        # build and run
make run-audiopad     # the audio demo
```

Or invoke the binaries directly under `./build/`.

## Using the binding in your own program

Drop `ffi/` into your project (or symlink it from here) and:

```kaikai
import ffi.raylib

fn main() : Unit / Ffi {
  raylib.init_window(800, 600, "hello")
  raylib.set_target_fps(60)
  loop()
  raylib.close_window()
}

fn loop() : Unit / Ffi {
  if raylib.window_should_close() { () }
  else {
    raylib.begin_drawing()
    raylib.clear_background(0x101820FF)
    raylib.draw_text("hello, kaikai", 80, 280, 30, 0xfdf9b9FF)
    raylib.end_drawing()
    loop()
  }
}
```

Build it the same way as the demos — see the `Makefile` for the
exact incantation. The short version: pipe the font shim and
`pkg-config --cflags --libs raylib` through `CFLAGS`, build with
`--backend=c`, and let `kai build` do the rest. The demos
`import loop` for `repeat` / `until` / `while` / `forever`.

## Binding reference

`Color` is an `extern "C" type` (`{ r, g, b, a: U8 }`). Write
colours as `0xRRGGBBAA` hex through the helper `rgba(p): Color`
and pass the result to anything that paints. `Vector2` and
`Rect` are likewise by-value records.

### Window / lifecycle

| `pub fn`                                | raylib           |
| --------------------------------------- | ---------------- |
| `init_window(w, h, title)`              | `InitWindow`     |
| `close_window()`                        | `CloseWindow`    |
| `window_should_close()`                 | `WindowShouldClose` |
| `set_target_fps(fps)`                   | `SetTargetFPS`   |
| `get_fps()`                             | `GetFPS`         |
| `get_time()`                            | `GetTime`        |

### Frame

| `pub fn`                | raylib              |
| ----------------------- | ------------------- |
| `begin_drawing()`       | `BeginDrawing`      |
| `end_drawing()`         | `EndDrawing`        |
| `clear_background(c)`   | `ClearBackground`   |

### Shapes / text

| `pub fn`                                          | raylib            |
| ------------------------------------------------- | ----------------- |
| `draw_rectangle(x, y, w, h, color)`               | `DrawRectangle`   |
| `draw_pixel(x, y, color)`                         | `DrawPixel`       |
| `draw_text(s, x, y, size, color)`                 | `DrawText`        |
| `draw_line(x1, y1, x2, y2, color)`                | `DrawLine`        |
| `draw_circle(x, y, r, color)`                     | `DrawCircle`      |
| `draw_triangle_v(v1, v2, v3, color)`              | `DrawTriangle`    |

### Fonts

`Font` isn't a by-value struct, so it goes through a slot table
in `ffi/raylib_fonts.c`. `load_font` returns a slot (or `-1`);
`draw_text_ex(-1, ...)` falls back to the default bitmap font.

| `pub fn`                                       | C symbol                  |
| ---------------------------------------------- | ------------------------- |
| `load_font(path, size)`                        | `kai_raylib_load_font`    |
| `unload_font(slot)`                            | `kai_raylib_unload_font`  |
| `draw_text_ex(slot, s, x, y, size, spacing, color)` | `kai_raylib_draw_text_ex` |

### Audio

`Sound` and `Music` embed an `AudioStream` with internal pointers,
so like `Font` they go through a slot table
(`ffi/raylib_audio.c`). Call `init_audio_device()` once before
loading anything. Loaders return a slot (or `-1` when the table
is full or the asset is invalid); every play/stop/volume call is a
no-op on a `-1` slot, so the error propagates harmlessly.

**Sounds** are short SFX, loaded whole into memory:

| `pub fn`                            | raylib            |
| ----------------------------------- | ----------------- |
| `init_audio_device()`               | `InitAudioDevice` |
| `close_audio_device()`              | `CloseAudioDevice` |
| `audio_device_ready()`              | `IsAudioDeviceReady` |
| `set_master_volume(v)`              | `SetMasterVolume` |
| `load_sound(path)`                  | `LoadSound`       |
| `gen_tone(freq, seconds, amp)`      | `LoadSoundFromWave` (synth) |
| `play_sound(slot)`                  | `PlaySound`       |
| `stop_sound(slot)`                  | `StopSound`       |
| `sound_playing(slot)`               | `IsSoundPlaying`  |
| `set_sound_volume(slot, v)`         | `SetSoundVolume`  |
| `set_sound_pitch(slot, p)`          | `SetSoundPitch`   |
| `unload_sound(slot)`                | `UnloadSound`     |

`gen_tone` synthesises a mono sine straight to a `Sound` (no file
needed) — handy for placeholder SFX; it's what `audiopad.kai` uses.

**Music** is streamed; `update_music(slot)` must run every frame
to refill the buffers:

| `pub fn`                            | raylib            |
| ----------------------------------- | ----------------- |
| `load_music(path)`                  | `LoadMusicStream` |
| `play_music(slot)`                  | `PlayMusicStream` |
| `update_music(slot)`                | `UpdateMusicStream` |
| `stop_music(slot)`                  | `StopMusicStream` |
| `pause_music(slot)`                 | `PauseMusicStream` |
| `resume_music(slot)`                | `ResumeMusicStream` |
| `music_playing(slot)`               | `IsMusicStreamPlaying` |
| `set_music_volume(slot, v)`         | `SetMusicVolume`  |
| `set_music_looping(slot, on)`       | (sets `Music.looping`) |
| `seek_music(slot, seconds)`         | `SeekMusicStream` |
| `unload_music(slot)`                | `UnloadMusicStream` |

raylib decodes by file extension: `.wav`, `.ogg`, `.mp3`, `.flac`,
`.qoa`, `.xm` and `.mod`. A minimal background-music loop:

```kaikai
raylib.init_audio_device()
let song = raylib.load_music("assets/theme.ogg")
raylib.set_music_looping(song, true)
raylib.play_music(song)

until { raylib.window_should_close() } {
  raylib.update_music(song)   # required every frame
  raylib.begin_drawing()
  # ... draw ...
  raylib.end_drawing()
}
raylib.unload_music(song)
raylib.close_audio_device()
```

### Input

| `pub fn`                | raylib              |
| ----------------------- | ------------------- |
| `is_key_pressed(key)`   | `IsKeyPressed`      |

Common keycodes: `262 right`, `263 left`, `264 down`, `265 up`,
`32 space`, `82 R`, `67 C`, `61 +`, `45 -`.

### Random

| `pub fn`                       | raylib            |
| ------------------------------ | ----------------- |
| `set_random_seed(s)`           | `SetRandomSeed`   |
| `get_random_value(min, max)`   | `GetRandomValue`  |

### Math

| `pub fn`            | libm        |
| ------------------- | ----------- |
| `sin(x)`            | `sin`       |
| `cos(x)`            | `cos`       |
| `atan2(y, x)`       | `atan2`     |
| `sqrt(x)`           | `sqrt`      |

## How it's wired

`Color`, `Vector2` and `Rect` are `extern "C" type` records
whose field layout matches raylib's own structs, so kaikai
passes them by value straight to `ClearBackground`,
`DrawCircleV`, `DrawRectangleRec`, `DrawTriangle`, etc. — no C
shim. The one invariant that makes this work: the `out.c` kaikai
emits must NOT `-include raylib.h`, or raylib's `Color`/`Vector2`
typedefs collide with the ones kaikai generates. The Makefile
only links `libraylib`; it never pulls `raylib.h` into `out.c`.

Fonts are the exception. raylib's `Font` holds pointers and a
nested `Texture2D`, so it can't be a by-value record. It goes
through [`ffi/raylib_fonts.c`](ffi/raylib_fonts.c): a slot table
with scalar-only entry points. That file is a separate
translation unit that *does* include `raylib.h` and uses raylib's
own `Color` (ABI-identical to kaikai's) in its `draw_text_ex`
prototype.

The build pipeline:

```
*.kai  --kaic2 (--backend=c)-->  out.c  --cc-->  binary
                                            ^
                                            |
                       ffi/raylib_fonts.c (linked via CFLAGS)
                       ffi/raylib_audio.c (linked via CFLAGS)
                       libraylib          (linked via CFLAGS)
```

Each binding ([`ffi/raylib.kai`](ffi/raylib.kai)) is an
`extern "C" pub fn` with a name override binding the nice
kaikai identifier to the raylib symbol:

```kaikai
extern "C"("InitWindow")
pub fn init_window(w: I32, h: I32, title: String) : Unit / Ffi
```

The call site reads `raylib.init_window(...)`; the linker
resolves `InitWindow`. `I32`/`F32` annotate the exact C width at
the boundary while staying plain `Int`/`Real` on the kaikai side.
Math axioms (`sin`, `cos`, `atan2`, `sqrt`) link directly to libm
under their bare names.

## Demos

### Conway's Game of Life — `conway.kai`

| Key   | Action                |
| ----- | --------------------- |
| SPACE | Pause / resume        |
| R     | Reset to initial seed |
| C     | Clear the grid        |
| ESC   | Quit                  |

80×60 toroidal grid with two gliders, an R-pentomino (chaotic
for ~1100 generations), an LWSS, and a couple of blinkers as
the seed. ~10 generations/sec.

### Boids flocking — `boids.kai`

| Key   | Action              |
| ----- | ------------------- |
| SPACE | Pause / resume      |
| R     | Randomize positions |
| ESC   | Quit                |

150 boids in 800×600 with the three Reynolds rules
(separation, alignment, cohesion). O(n²) is ~22 500
pair-checks/frame, well inside the 16 ms budget on M-series
silicon.

### Mandelbrot explorer — `mandelbrot.kai`

| Key       | Action                       |
| --------- | ---------------------------- |
| Arrows    | Pan                          |
| `+` / `-` | Zoom in / out around centre  |
| R         | Reset to default view        |
| ESC       | Quit                         |

400×300 internal grid blitted at 2× scale. The iteration
counts are cached in an `Array[Int]` so intermediate frames are
cheap; recompute is gated by a `dirty` flag set on input. Default
view: re ∈ [-2.5, 1], im ∈ [-1, 1], 100 max iterations.

### Snake — `snake.kai`

| Key    | Action  |
| ------ | ------- |
| Arrows | Turn    |
| R      | Restart |
| ESC    | Quit    |

32×24 grid, 10 steps/sec. Body lives in two parallel
`Array[Int]`s wired up as a circular ring (head/tail indices
modulo `max_body`), so growing on apple-eat is just "don't
advance the tail". Input is buffered to the next step boundary
so a fast 180° doesn't self-collide.

### Audio pad — `audiopad.kai`

| Key             | Action                  |
| --------------- | ----------------------- |
| A S D F G H J K | Play a note (do…do)     |
| Up / Down       | Transpose by an octave  |
| `+` / `-`       | Master volume           |
| ESC             | Quit                    |

A one-octave C-major keyboard. The eight tones are synthesised in
memory at startup with `gen_tone` (one `Sound` per note), so the
demo needs no audio files on disk; transposing re-generates them
for the new octave. Doubles as the smoke test for the audio shim.

## Limitations / known gotchas

- **`--backend=c` is required.** Struct-by-value FFI doesn't run
  on the native (libLLVM) backend yet; it reports the gap and
  points at the flag. The Makefile passes `--backend=c` for all
  targets.
- **`raylib.h` must stay out of `out.c`.** kaikai emits its own
  `Color`/`Vector2` typedefs; pulling in `raylib.h` (e.g. via
  `-include`) collides with them. The font shim includes
  `raylib.h` in its own translation unit instead.
- **`Font`, `Sound` and `Music` aren't by-value.** Their pointers
  (and `Font`'s nested texture, `Sound`/`Music`'s embedded
  `AudioStream`) mean they can't be `extern "C" type` records, so
  each keeps a slot-table shim rather than crossing the boundary
  directly.

## License

MIT. See [LICENSE](LICENSE).
