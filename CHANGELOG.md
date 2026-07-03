## v0.4.0 (2026-07-02)

### Feat

- sprites & camera demo (top-down world)
- **ffi**: textures + 2D camera, both by value
- **ffi**: keyboard/mouse/gamepad input + shapes + collisions

### Fix

- **orderbook**: spawn the matching engine as an actor

### Refactor

- demos use ffi.keys instead of hardcoded keycodes

## v0.3.0 (2026-07-02)

### Refactor

- **ffi**: drop shims — Font/Sound/Music cross by value

## v0.2.1 (2026-06-28)

## v0.2.0 (2026-06-28)

### Feat

- **ffi**: add audio support (SFX + music streaming)
- **ffi**: migrate raylib binding to FFI v2 (struct-by-value)
- **ffi**: TTF font loading + DrawTextEx
- add observatory demo (N-body, HR diagram, parallax)
- add orderbook demo (actors + matching engine)
- kaikai 0.41 alignment + three new demos

### Refactor

- layer demos into MODEL / STATE / VIEW
- use stdlib loop module (repeat / until)
- drop redundant `} else { () }` branches
