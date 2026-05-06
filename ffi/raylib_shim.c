/*
 * raylib_shim.c — bridge between kaikai FFI v1 (primitives only)
 * and raylib's struct-by-value API. All exported functions take
 * scalars (int64_t, double, const char*, bool/int) and rebuild
 * the raylib aggregates (Color, Vector2) internally.
 *
 * Color is packed as 0xRRGGBBAA in an int64_t. Symbols carry the
 * `kai_raylib_*` prefix; the kaikai-side bindings in
 * `ffi/raylib.kai` route the nice `raylib.<name>(...)` call sites
 * to these prefixed symbols via the `extern "C"("c_symbol") pub
 * fn nice_name(...)` name override (issue #261).
 *
 * Math (sin/cos/sqrt/atan2) is omitted: those axioms link
 * straight to libm under their bare names.
 */

#include <raylib.h>
#include <stdint.h>
#include <stddef.h>

static Color color_from_packed(int64_t p) {
  return (Color){
    (unsigned char)((p >> 24) & 0xFF),
    (unsigned char)((p >> 16) & 0xFF),
    (unsigned char)((p >>  8) & 0xFF),
    (unsigned char)( p        & 0xFF),
  };
}

/* --- window / lifecycle --- */

void kai_raylib_init_window(int64_t w, int64_t h, const char *title) {
  InitWindow((int)w, (int)h, title);
}

void kai_raylib_close_window(void) { CloseWindow(); }

/* kaikai FFI maps Bool to int (i32). Returning a plain int works
 * because raylib's WindowShouldClose returns bool which the C ABI
 * widens to int. */
int kai_raylib_window_should_close(void) {
  return WindowShouldClose() ? 1 : 0;
}

void kai_raylib_set_target_fps(int64_t fps) { SetTargetFPS((int)fps); }

int64_t kai_raylib_get_fps(void) { return (int64_t)GetFPS(); }

double kai_raylib_get_time(void) { return GetTime(); }

/* --- frame --- */

void kai_raylib_begin_drawing(void) { BeginDrawing(); }
void kai_raylib_end_drawing(void)   { EndDrawing(); }

void kai_raylib_clear_background(int64_t color) {
  ClearBackground(color_from_packed(color));
}

/* --- shapes --- */

void kai_raylib_draw_circle(int64_t x, int64_t y, double r, int64_t color) {
  DrawCircle((int)x, (int)y, (float)r, color_from_packed(color));
}

void kai_raylib_draw_rectangle(int64_t x, int64_t y, int64_t w, int64_t h, int64_t color) {
  DrawRectangle((int)x, (int)y, (int)w, (int)h, color_from_packed(color));
}

void kai_raylib_draw_text(const char *text, int64_t x, int64_t y, int64_t size, int64_t color) {
  DrawText(text, (int)x, (int)y, (int)size, color_from_packed(color));
}

void kai_raylib_draw_line(int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color) {
  DrawLine((int)x1, (int)y1, (int)x2, (int)y2, color_from_packed(color));
}

void kai_raylib_draw_triangle(double v1x, double v1y, double v2x, double v2y,
                              double v3x, double v3y, int64_t color) {
  Vector2 a = {(float)v1x, (float)v1y};
  Vector2 b = {(float)v2x, (float)v2y};
  Vector2 c = {(float)v3x, (float)v3y};
  DrawTriangle(a, b, c, color_from_packed(color));
}

void kai_raylib_draw_pixel(int64_t x, int64_t y, int64_t color) {
  DrawPixel((int)x, (int)y, color_from_packed(color));
}

/* --- input --- */

int kai_raylib_is_key_pressed(int64_t key) { return IsKeyPressed((int)key) ? 1 : 0; }

/* --- random --- */

void kai_raylib_set_random_seed(int64_t s) { SetRandomSeed((unsigned int)s); }
int64_t kai_raylib_get_random_value(int64_t min, int64_t max) {
  return (int64_t)GetRandomValue((int)min, (int)max);
}

/* --- fonts ---
 *
 * raylib's Font is a struct (texture + atlas + glyph recs).
 * kaikai FFI v1 can't pass structs, so we keep a small slot
 * table of loaded fonts inside the shim and hand kaikai an
 * Int slot id. Slot 0 is reserved for "default" — falls back
 * to DrawText if the user hasn't loaded a TTF.
 *
 * load_font returns the slot id, or -1 if all slots are full
 * or the file failed to load. draw_text_ex(-1, ...) falls
 * back to the default bitmap font.
 */

#define KAI_RAYLIB_MAX_FONTS 4
static Font kai_raylib_fonts[KAI_RAYLIB_MAX_FONTS];
static int  kai_raylib_font_loaded[KAI_RAYLIB_MAX_FONTS] = {0,0,0,0};

int64_t kai_raylib_load_font(const char *path, int64_t size) {
  for (int i = 0; i < KAI_RAYLIB_MAX_FONTS; i++) {
    if (!kai_raylib_font_loaded[i]) {
      Font f = LoadFontEx(path, (int)size, NULL, 0);
      if (!IsFontValid(f)) return (int64_t)(-1);
      kai_raylib_fonts[i] = f;
      kai_raylib_font_loaded[i] = 1;
      return (int64_t)i;
    }
  }
  return (int64_t)(-1);
}

void kai_raylib_unload_font(int64_t slot) {
  int i = (int)slot;
  if (i < 0 || i >= KAI_RAYLIB_MAX_FONTS) return;
  if (!kai_raylib_font_loaded[i]) return;
  UnloadFont(kai_raylib_fonts[i]);
  kai_raylib_font_loaded[i] = 0;
}

void kai_raylib_draw_text_ex(int64_t slot, const char *text,
                             int64_t x, int64_t y, int64_t size,
                             double spacing, int64_t color) {
  int i = (int)slot;
  if (i < 0 || i >= KAI_RAYLIB_MAX_FONTS || !kai_raylib_font_loaded[i]) {
    /* Fall back to the default font. */
    DrawText(text, (int)x, (int)y, (int)size, color_from_packed(color));
    return;
  }
  Vector2 pos = {(float)x, (float)y};
  DrawTextEx(kai_raylib_fonts[i], text, pos, (float)size,
             (float)spacing, color_from_packed(color));
}
