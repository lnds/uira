/*
 * raylib_shim.c — bridge between kaikai FFI v1 (primitives only) and
 * raylib's struct-by-value API. All exported `kai_*` functions take
 * scalars (int64_t, double, const char*, bool/int) and rebuild the
 * raylib aggregates (Color, Vector2) internally.
 *
 * Color is packed as 0xRRGGBBAA in an int64_t.
 */

#include <raylib.h>
#include <stdint.h>
#include <math.h>

static Color color_from_packed(int64_t p) {
  return (Color){
    (unsigned char)((p >> 24) & 0xFF),
    (unsigned char)((p >> 16) & 0xFF),
    (unsigned char)((p >>  8) & 0xFF),
    (unsigned char)( p        & 0xFF),
  };
}

/* --- window / lifecycle --- */

void kai_init_window(int64_t w, int64_t h, const char *title) {
  InitWindow((int)w, (int)h, title);
}

void kai_close_window(void) { CloseWindow(); }

/* kaikai FFI maps Bool to int (i32). Returning a plain int works
 * because raylib's WindowShouldClose returns bool which the C ABI
 * widens to int. */
int kai_window_should_close(void) {
  return WindowShouldClose() ? 1 : 0;
}

void kai_set_target_fps(int64_t fps) { SetTargetFPS((int)fps); }

int64_t kai_get_fps(void) { return (int64_t)GetFPS(); }

double kai_get_frame_time(void) { return (double)GetFrameTime(); }
double kai_get_time(void)       { return GetTime(); }

/* --- frame --- */

void kai_begin_drawing(void) { BeginDrawing(); }
void kai_end_drawing(void)   { EndDrawing(); }

void kai_clear_background(int64_t color) {
  ClearBackground(color_from_packed(color));
}

/* --- shapes --- */

void kai_draw_circle(int64_t x, int64_t y, double r, int64_t color) {
  DrawCircle((int)x, (int)y, (float)r, color_from_packed(color));
}

void kai_draw_rectangle(int64_t x, int64_t y, int64_t w, int64_t h, int64_t color) {
  DrawRectangle((int)x, (int)y, (int)w, (int)h, color_from_packed(color));
}

void kai_draw_text(const char *text, int64_t x, int64_t y, int64_t size, int64_t color) {
  DrawText(text, (int)x, (int)y, (int)size, color_from_packed(color));
}

void kai_draw_line(int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color) {
  DrawLine((int)x1, (int)y1, (int)x2, (int)y2, color_from_packed(color));
}

void kai_draw_triangle(double v1x, double v1y, double v2x, double v2y,
                       double v3x, double v3y, int64_t color) {
  Vector2 a = {(float)v1x, (float)v1y};
  Vector2 b = {(float)v2x, (float)v2y};
  Vector2 c = {(float)v3x, (float)v3y};
  DrawTriangle(a, b, c, color_from_packed(color));
}

void kai_draw_pixel(int64_t x, int64_t y, int64_t color) {
  DrawPixel((int)x, (int)y, color_from_packed(color));
}

/* --- input --- */

int kai_is_key_pressed(int64_t key) { return IsKeyPressed((int)key) ? 1 : 0; }
int kai_is_key_down(int64_t key)    { return IsKeyDown((int)key) ? 1 : 0; }
int64_t kai_get_mouse_x(void) { return (int64_t)GetMouseX(); }
int64_t kai_get_mouse_y(void) { return (int64_t)GetMouseY(); }

/* --- random --- */

void kai_set_random_seed(int64_t s) { SetRandomSeed((unsigned int)s); }
int64_t kai_get_random_value(int64_t min, int64_t max) {
  return (int64_t)GetRandomValue((int)min, (int)max);
}

/* --- math --- */

double kai_sin(double x)  { return sin(x); }
double kai_cos(double x)  { return cos(x); }
double kai_atan2(double y, double x) { return atan2(y, x); }
double kai_sqrt(double x) { return sqrt(x); }
