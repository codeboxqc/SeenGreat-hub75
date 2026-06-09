/*
 * hub75_driver.h - HUB75 LED Matrix Graphics Engine
 * Features: True color BCM, gamma, clipping, sprites, dithering, scrolling
 */

#ifndef HUB75_DRIVER_H
#define HUB75_DRIVER_H

#include <Arduino.h>
#include "config.h"

// ==================== Feature Switches ====================
#define ENABLE_SPRITE_ENGINE   1
#define ENABLE_DITHERING       1
#define ENABLE_COLOR_CYCLING   1
#define ENABLE_DMA_REFRESH     0   // Keep 0 for short‑term optimisation

// ==================== RGB Color ====================
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

// Predefined colors
extern const rgb_t RGB_BLACK;
extern const rgb_t RGB_WHITE;
extern const rgb_t RGB_RED;
extern const rgb_t RGB_GREEN;
extern const rgb_t RGB_BLUE;
extern const rgb_t RGB_YELLOW;
extern const rgb_t RGB_CYAN;
extern const rgb_t RGB_MAGENTA;
extern const rgb_t RGB_ORANGE;
extern const rgb_t RGB_GRAY;
extern const rgb_t RGB_DARK_GRAY;

// System
void hub75_init(void);
void hub75_set_brightness(uint8_t brightness);
void hub75_refresh(void);
void hub75_swap_buffers(void);

// Camera offset
extern int camera_x;
extern int camera_y;
void hub75_set_camera(int x, int y);

// Clipping
void hub75_set_clip(int x, int y, int w, int h);
void hub75_reset_clip(void);

// Basic drawing
void hub75_clear(void);
void hub75_fill(rgb_t color);
void hub75_set_pixel(int x, int y, rgb_t color);
void hub75_blend_pixel(int x, int y, rgb_t new_color, uint8_t alpha);
rgb_t hub75_get_pixel(int x, int y);

// Primitives
void hub75_draw_hline(int x, int y, int width, rgb_t color);
void hub75_draw_vline(int x, int y, int height, rgb_t color);
void hub75_draw_line(int x0, int y0, int x1, int y1, rgb_t color);
void hub75_draw_rect(int x, int y, int width, int height, rgb_t color);
void hub75_fill_rect(int x, int y, int width, int height, rgb_t color);
void hub75_draw_circle(int cx, int cy, int radius, rgb_t color);
void hub75_fill_circle(int cx, int cy, int radius, rgb_t color);
void hub75_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, rgb_t color);

void hub75_draw_rounded_rect(int x, int y, int w, int h, int radius, rgb_t color);
void hub75_fill_rounded_rect(int x, int y, int w, int h, int radius, rgb_t color);
void hub75_draw_polygon(const int *x_verts, const int *y_verts, int num_verts, rgb_t color);
void hub75_fill_polygon(const int *x_verts, const int *y_verts, int num_verts, rgb_t color);
void hub75_draw_ellipse(int cx, int cy, int rx, int ry, rgb_t color);
void hub75_fill_ellipse(int cx, int cy, int rx, int ry, rgb_t color);
void hub75_fill_rect_gradient(int x, int y, int w, int h, rgb_t color1, rgb_t color2, bool horizontal);

// Text
void hub75_draw_char(int x, int y, char c, rgb_t color, int size);
void hub75_draw_string(int x, int y, const char *str, rgb_t color, int size);
int hub75_string_width(const char *str, int size);

// Images & Sprites
void hub75_draw_image(int x, int y, const uint16_t *bitmap, int w, int h);
void hub75_draw_sprite(int x, int y, const uint16_t *bitmap, int w, int h, uint16_t transparent_color);

 
 

// Scrolling
void hub75_scroll(int dx, int dy);

 
void hub75_enable_dithering(bool enable);
 

 
void hub75_set_hue_shift(uint16_t hue_shift);
 

// Helpers
rgb_t hsv_to_rgb(uint16_t hue, uint8_t sat, uint8_t val);
inline rgb_t rgb(uint8_t r, uint8_t g, uint8_t b) { rgb_t c = {r,g,b}; return c; }
inline rgb_t hex_to_rgb(uint32_t hex) { return rgb((hex>>16)&0xFF, (hex>>8)&0xFF, hex&0xFF); }

#endif