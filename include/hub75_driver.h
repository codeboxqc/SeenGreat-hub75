/*
 * hub75_driver.h - HUB75 LED Matrix Driver for RP2040/RP2350
 *
 * Bit-banged driver optimized for Pico/Pico 2
 * Supports configurable pin mapping via config.h
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
extern const rgb_t RGB_PURPLE;
extern const rgb_t RGB_BROWN;
extern const rgb_t RGB_PINK;
extern const rgb_t RGB_LIME;
extern const rgb_t RGB_TEAL;
extern const rgb_t RGB_NAVY;

// ==================== Color Palette ====================

extern const rgb_t hub75_palette[256];

// ==================== Function Prototypes ====================

/**
 * Initialize the HUB75 driver
 * Sets up pins and allocates framebuffer
 */
void hub75_init(void);

/**
 * Clear the display to black
 */
void hub75_clear(void);

/**
 * Fill entire display with a color
 */
void hub75_fill(rgb_t color);

/**
 * Set a single pixel
 */
void hub75_set_pixel(int x, int y, rgb_t color);

/**
 * Set a single pixel using palette index
 */
void hub75_set_pixel_palette(int x, int y, uint8_t palette_idx);

/**
 * Get a pixel's color
 */
rgb_t hub75_get_pixel(int x, int y);

/**
 * Draw a horizontal line
 */
void hub75_draw_hline(int x, int y, int width, rgb_t color);

/**
 * Draw a vertical line
 */
void hub75_draw_vline(int x, int y, int height, rgb_t color);

/**
 * Draw a line between two points
 */
void hub75_draw_line(int x0, int y0, int x1, int y1, rgb_t color);

/**
 * Draw rectangle outline
 */
void hub75_draw_rect(int x, int y, int width, int height, rgb_t color);

/**
 * Draw filled rectangle
 */
void hub75_fill_rect(int x, int y, int width, int height, rgb_t color);

/**
 * Draw circle outline
 */
void hub75_draw_circle(int cx, int cy, int radius, rgb_t color);

/**
 * Draw filled circle
 */
void hub75_fill_circle(int cx, int cy, int radius, rgb_t color);

/**
 * Draw an ellipse outline
 */
void hub75_draw_ellipse(int cx, int cy, int rx, int ry, rgb_t color);

/**
 * Draw a filled ellipse
 */
void hub75_fill_ellipse(int cx, int cy, int rx, int ry, rgb_t color);

/**
 * Draw a round rectangle outline
 */
void hub75_draw_round_rect(int x, int y, int w, int h, int r, rgb_t color);

/**
 * Draw a filled round rectangle
 */
void hub75_fill_round_rect(int x, int y, int w, int h, int r, rgb_t color);

/**
 * Draw triangle outline
 */
void hub75_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, rgb_t color);

/**
 * Draw a single character (5x7 font)
 */
void hub75_draw_char(int x, int y, char c, rgb_t color, int size);

/**
 * Draw a string
 */
void hub75_draw_string(int x, int y, const char *str, rgb_t color, int size);

/**
 * Get string width in pixels
 */
int hub75_string_width(const char *str, int size);

/**
 * Set display brightness (0-255)
 */
void hub75_set_brightness(uint8_t brightness);

/**
 * Refresh the display (call in loop)
 */
void hub75_refresh(void);

/**
 * Swap double buffer
 */
void hub75_swap_buffers(void);

/**
 * Draw an image from an array of RGB colors
 */
void hub75_putimage(int x, int y, int width, int height, const rgb_t *image);

/**
 * Draw an image from an array of palette indices
 */
void hub75_putimage_palette(int x, int y, int width, int height, const uint8_t *image);

/**
 * Create RGB color from components
 */
inline rgb_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    rgb_t c = {r, g, b};
    return c;
}

#endif // HUB75_DRIVER_H
