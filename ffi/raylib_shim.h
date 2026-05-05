/* Forward declarations for the raylib FFI shim. Pulled into the
 * kaikai-emitted out.c via `cc -include ffi/raylib_shim.h` so the
 * FFI call sites resolve without warnings.
 *
 * Symbols carry the `kai_raylib_*` prefix so they don't collide
 * with raylib (CamelCase) or arbitrary code that links these
 * shims in. The kaikai-side bindings in `ffi/raylib.kai` use
 * `extern "C"("kai_raylib_<name>") pub fn <name>(...)` (issue
 * #261) so call sites read as `raylib.draw_line(...)` etc. while
 * the linker sees the prefixed symbol. Math (sin/cos/atan2/sqrt)
 * resolves directly against libm and is intentionally absent. */

#ifndef KAI_RAYLIB_SHIM_H
#define KAI_RAYLIB_SHIM_H

#include <stdint.h>
#include <math.h>  /* prototypes for sin/cos/atan2/sqrt — kaikai math
                    * axioms link directly to libm under these names */

void    kai_raylib_init_window(int64_t w, int64_t h, const char *title);
void    kai_raylib_close_window(void);
int     kai_raylib_window_should_close(void);
void    kai_raylib_set_target_fps(int64_t fps);
int64_t kai_raylib_get_fps(void);
double  kai_raylib_get_time(void);

void    kai_raylib_begin_drawing(void);
void    kai_raylib_end_drawing(void);
void    kai_raylib_clear_background(int64_t color);

void    kai_raylib_draw_circle(int64_t x, int64_t y, double r, int64_t color);
void    kai_raylib_draw_rectangle(int64_t x, int64_t y, int64_t w, int64_t h, int64_t color);
void    kai_raylib_draw_text(const char *text, int64_t x, int64_t y, int64_t size, int64_t color);
void    kai_raylib_draw_line(int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color);
void    kai_raylib_draw_triangle(double v1x, double v1y, double v2x, double v2y,
                                 double v3x, double v3y, int64_t color);
void    kai_raylib_draw_pixel(int64_t x, int64_t y, int64_t color);

int     kai_raylib_is_key_pressed(int64_t key);

void    kai_raylib_set_random_seed(int64_t s);
int64_t kai_raylib_get_random_value(int64_t min, int64_t max);

#endif
