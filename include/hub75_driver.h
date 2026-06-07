/*
 * hub75_driver.h - HUB75 LED Matrix Graphics Engine for RP2040/RP2350
 * Features: Fast Array Maths, Camera Viewport, Alpha Blending, Sprite Transparency
 */

#ifndef HUB75_DRIVER_H
#define HUB75_DRIVER_H

#include <Arduino.h>
#include "config.h"

// ==================== RGB Color Structure ====================

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

// ==================== Predefined Colors ====================

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

// ==================== Function Prototypes ====================

// System
void hub75_init(void);
void hub75_set_brightness(uint8_t brightness);
void hub75_refresh(void);
void hub75_swap_buffers(void);

// Camera System
extern int camera_x;
extern int camera_y;
void hub75_set_camera(int x, int y);

// Basic Drawing
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

// Text
void hub75_draw_char(int x, int y, char c, rgb_t color, int size);
void hub75_draw_string(int x, int y, const char *str, rgb_t color, int size);
int hub75_string_width(const char *str, int size);

// Images & Sprites
void hub75_draw_image(int x, int y, const uint16_t *bitmap, int w, int h);
void hub75_draw_sprite(int x, int y, const uint16_t *bitmap, int w, int h, uint16_t transparent_color = 0xF81F);

// ==================== Inline Color Helpers ====================

// Create RGB color (Raw values, no gamma compression to respect the 127 threshold)
inline rgb_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    rgb_t c = {r, g, b};
    return c;
}

// Create RGB color from a standard Web HEX code (e.g., 0xFF5733)
inline rgb_t hex_to_rgb(uint32_t hex) {
    return rgb((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

#endif // HUB75_DRIVER_H