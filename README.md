# kaikai-raylib

A small [raylib](https://www.raylib.com/) FFI binding for
[kaikai](https://kaikai-lang.org/), built on the `[<extern_c>]`
axiom support landed in the FFI v1 lane (kaikai 0.37+). Ships
the binding as a reusable module plus four demos that exercise
it.

The binding covers the slice of raylib needed for 2D games and
visualisations: window lifecycle, frame loop, basic shapes,
text, input, random, and a handful of math helpers. Anything
that takes a `Color` or `Vector2` by value (i.e. nearly all of
raylib) is bridged through a tiny C shim that flattens those
into primitives, because kaikai FFI v1 only carries `Int`,
`Real`, `Bool`, `String`, and `Unit` across the boundary.

## What's in the box

```
kaikai-raylib/
├── ffi/
│   ├── raylib.kai       # kaikai-side bindings
│   ├── raylib_shim.c    # C wrapper: scalar-only signatures
│   └── raylib_shim.h    # forward decls, -include'd into out.c
├── conway.kai           # demo: Game of Life
├── boids.kai            # demo: Reynolds flocking
├── mandelbrot.kai       # demo: Mandelbrot explorer (pan + zoom)
├── snake.kai            # demo: classic snake
├── Makefile
└── README.md
```

## Prerequisites

- macOS (tested) or Linux. Windows untested.
- `cc` (clang or gcc) with C99.
- `pkg-config`.
- raylib 5.x. On macOS: `brew install raylib`.
- A working `kai` driver from a sibling kaikai checkout. The
  Makefile expects it at `../kaikai/bin/kai`; override with
  `KAI_BIN=/path/to/kai`. kaikai 0.37+ is required (FFI v1).

## Build & run

```sh
make                  # all demos -> ./build/{conway,boids,mandelbrot,snake}
make conway           # one at a time
make run-snake        # build and run
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
exact incantation. The short version: pipe the shim source and
`pkg-config --cflags --libs raylib` through `CFLAGS`, set
`KAI_NO_STDLIB=1` (kaikai 0.37.0's stdlib has unrelated
type-check regressions), and let `kai build` do the rest.

## Binding reference

Colours are packed as `0xRRGGBBAA` `Int`s. Pass them to anything
that paints. There's no `Color` type on the kaikai side.

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
| `draw_triangle(x1, y1, x2, y2, x3, y3, color)`    | `DrawTriangle`    |

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

kaikai's FFI v1 supports primitive types only and rejects
pass-by-value structs. raylib takes `Color` and `Vector2` by
value almost everywhere. The shim
([`ffi/raylib_shim.c`](ffi/raylib_shim.c)) bridges the two
worlds: every `kai_*` function takes scalars (an `int64_t`
packed `0xRRGGBBAA` for colours, separate `Real`s for vector
components) and rebuilds the aggregates internally.

The build pipeline:

```
*.kai  --kaic2-->  out.c  --cc-->  binary
                              ^
                              |
                  ffi/raylib_shim.c (linked in via CFLAGS)
                  ffi/raylib_shim.h (forward decls, -include'd)
```

The kaikai-side bindings ([`ffi/raylib.kai`](ffi/raylib.kai))
declare each shim function as a private `[<extern_c>] axiom`
and re-expose it through a thin `pub fn` wrapper. The wrapper
layer exists because `pub axiom` does not currently re-export
across modules
([lnds/kaikai#244](https://github.com/lnds/kaikai/issues/244)).
With `-O2`, clang inlines the wrapper and the indirection is
free at runtime — `nm` confirms no `kai_raylib__*` symbols
survive in the final binary.

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

## Limitations / known gotchas

- **No struct support in FFI v1.** Colours go through a packed
  `int64_t` and vectors get split into two `Real`s. A future
  kaikai release with FFI v2 (struct-by-value, out-params) will
  let bindings call `DrawCircleV(Vector2, ...)` etc. directly.
- **`pub axiom` doesn't re-export** ([kaikai#244](https://github.com/lnds/kaikai/issues/244)).
  Workaround in this repo: each axiom is private to `ffi/raylib.kai`
  and a `pub fn` wraps it. Removable once #244 lands.
- **`KAI_NO_STDLIB=1` is required** in 0.37.0 because the stdlib
  has type-check regressions in unrelated modules
  (`option`, `result`, `list`, `regexp`, `json`) that cascade
  into any build that loads it. Demos rely only on compiler
  builtins (`Array[T]`, `print`, `int_to_string`,
  `real_to_string`, `int_to_real`).

## License

MIT. See [LICENSE](LICENSE).
