/* Forward declarations for the kaikai-side FFI shim. Pulled into
 * the kaikai-emitted out.c via `cc -include ffi/raylib_shim.h` so
 * the FFI call sites resolve without warnings. */

#ifndef KAI_RAYLIB_SHIM_H
#define KAI_RAYLIB_SHIM_H

#include <stdint.h>

void    kai_init_window(int64_t w, int64_t h, const char *title);
void    kai_close_window(void);
int     kai_window_should_close(void);
void    kai_set_target_fps(int64_t fps);
int64_t kai_get_fps(void);
double  kai_get_frame_time(void);
double  kai_get_time(void);

void    kai_begin_drawing(void);
void    kai_end_drawing(void);
void    kai_clear_background(int64_t color);

void    kai_draw_circle(int64_t x, int64_t y, double r, int64_t color);
void    kai_draw_rectangle(int64_t x, int64_t y, int64_t w, int64_t h, int64_t color);
void    kai_draw_text(const char *text, int64_t x, int64_t y, int64_t size, int64_t color);
void    kai_draw_line(int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color);
void    kai_draw_triangle(double v1x, double v1y, double v2x, double v2y,
                          double v3x, double v3y, int64_t color);
void    kai_draw_pixel(int64_t x, int64_t y, int64_t color);

int     kai_is_key_pressed(int64_t key);
int     kai_is_key_down(int64_t key);
int64_t kai_get_mouse_x(void);
int64_t kai_get_mouse_y(void);

void    kai_set_random_seed(int64_t s);
int64_t kai_get_random_value(int64_t min, int64_t max);

double  kai_sin(double x);
double  kai_cos(double x);
double  kai_atan2(double y, double x);
double  kai_sqrt(double x);

#endif
