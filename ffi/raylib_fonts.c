/*
 * Font shim — the one piece of raylib that can't cross the FFI by
 * value: `Font` holds pointers and a nested Texture2D, so it isn't a
 * fixed-width `extern "C" type`. We park loaded fonts in a slot table
 * and expose scalar-only entry points the kaikai bindings call.
 *
 * This file includes raylib.h and uses raylib's own `Color` in the
 * draw_text_ex prototype. That Color is ABI-identical to the one
 * kaikai emits, so the value passed from kaikai lands intact. The
 * emitted out.c does NOT include raylib.h (that would clash with its
 * generated typedefs), which is why the font entry points live here
 * rather than as direct binds.
 *
 * Slot -1 (or any unloaded slot) makes draw_text_ex fall back to the
 * default bitmap font via DrawText.
 */

#include <raylib.h>
#include <stdint.h>
#include <stddef.h>

#define KAI_RAYLIB_MAX_FONTS 4
static Font fonts[KAI_RAYLIB_MAX_FONTS];
static int  loaded[KAI_RAYLIB_MAX_FONTS] = {0, 0, 0, 0};

int64_t kai_raylib_load_font(const char *path, int size) {
  for (int i = 0; i < KAI_RAYLIB_MAX_FONTS; i++) {
    if (!loaded[i]) {
      Font f = LoadFontEx(path, size, NULL, 0);
      if (!IsFontValid(f)) return (int64_t)(-1);
      fonts[i] = f;
      loaded[i] = 1;
      return (int64_t)i;
    }
  }
  return (int64_t)(-1);
}

void kai_raylib_unload_font(int64_t slot) {
  int i = (int)slot;
  if (i < 0 || i >= KAI_RAYLIB_MAX_FONTS || !loaded[i]) return;
  UnloadFont(fonts[i]);
  loaded[i] = 0;
}

void kai_raylib_draw_text_ex(int64_t slot, const char *text,
                             int x, int y, int size,
                             float spacing, Color tint) {
  int i = (int)slot;
  if (i < 0 || i >= KAI_RAYLIB_MAX_FONTS || !loaded[i]) {
    DrawText(text, x, y, size, tint);
    return;
  }
  Vector2 pos = {(float)x, (float)y};
  DrawTextEx(fonts[i], text, pos, (float)size, spacing, tint);
}
