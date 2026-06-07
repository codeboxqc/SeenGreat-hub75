/*
 * hub75_driver.cpp - HUB75 LED Matrix Graphics Engine Implementation
 */

#include <string.h>
#include <Arduino.h>
#include "hardware/gpio.h" // Native Pico SDK
#include "hub75_driver.h"

// ==================== Predefined Colors ====================
const rgb_t RGB_BLACK     = {0, 0, 0};
const rgb_t RGB_WHITE     = {255, 255, 255};
const rgb_t RGB_RED       = {255, 0, 0};
const rgb_t RGB_GREEN     = {0, 255, 0};
const rgb_t RGB_BLUE      = {0, 0, 255};
const rgb_t RGB_YELLOW    = {255, 255, 0};
const rgb_t RGB_CYAN      = {0, 255, 255};
const rgb_t RGB_MAGENTA   = {255, 0, 255};
const rgb_t RGB_ORANGE    = {255, 165, 0}; 
const rgb_t RGB_GRAY      = {128, 128, 128}; 
const rgb_t RGB_DARK_GRAY = {64, 64, 64};    

// ==================== Engine State ====================
#define FB_SIZE (TOTAL_WIDTH * TOTAL_HEIGHT * 3)
static uint8_t framebuffer_a[FB_SIZE];
static uint8_t framebuffer_b[FB_SIZE];
static uint8_t *draw_buffer = framebuffer_a;
static uint8_t *display_buffer = framebuffer_b;

static uint8_t brightness = DEFAULT_BRIGHTNESS;

int camera_x = 0;
int camera_y = 0;

// ==================== 5x7 Font ====================
static const uint8_t font_5x7[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, // F
    0x3E, 0x41, 0x41, 0x51, 0x32, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x03, 0x04, 0x78, 0x04, 0x03, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // backslash
    0x41, 0x41, 0x7F, 0x00, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08, // ->
    0x08, 0x1C, 0x2A, 0x08, 0x08, // <-
};

// ==================== Hardware Macros ====================
// Using true hardware SDK. 
// __asm volatile("nop") tells the CPU processor to do literally nothing 
// for exactly 1 clock cycle (~6.6 nanoseconds on the Pico 2). 
// By stacking a few, we perfectly pad the signal without wasting time!

static inline void set_row_address(int row) {
    gpio_put(PIN_A, row & 0x01);
    gpio_put(PIN_B, (row >> 1) & 0x01);
    gpio_put(PIN_C, (row >> 2) & 0x01);
    gpio_put(PIN_D, (row >> 3) & 0x01);
    #if ADDR_BITS >= 5
    gpio_put(PIN_E, (row >> 4) & 0x01);
    #endif
}

static inline void clock_pulse() { 
    gpio_put(PIN_CLK, 1); 
    // 5 NOPs = ~33ns delay. Just enough for the shift registers to see the data!
    __asm volatile("nop\nnop\nnop\nnop\nnop\n"); 
    gpio_put(PIN_CLK, 0); 
}

static inline void latch_data()  { 
    gpio_put(PIN_LAT, 1); 
    __asm volatile("nop\nnop\nnop\nnop\nnop\n");
    gpio_put(PIN_LAT, 0); 
}

// ==================== System ====================

void hub75_init(void) {
    pinMode(PIN_R1, OUTPUT); pinMode(PIN_G1, OUTPUT); pinMode(PIN_B1, OUTPUT);
    pinMode(PIN_R2, OUTPUT); pinMode(PIN_G2, OUTPUT); pinMode(PIN_B2, OUTPUT);
    pinMode(PIN_A, OUTPUT);  pinMode(PIN_B, OUTPUT);  pinMode(PIN_C, OUTPUT);
    pinMode(PIN_D, OUTPUT);
    #if ADDR_BITS >= 5
    pinMode(PIN_E, OUTPUT);
    #endif
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_LAT, OUTPUT);
    pinMode(PIN_OE, OUTPUT);
    
    digitalWrite(PIN_OE, 1);
    digitalWrite(PIN_LAT, 0);
    digitalWrite(PIN_CLK, 0);
    
    memset(framebuffer_a, 0, FB_SIZE);
    memset(framebuffer_b, 0, FB_SIZE);
}

void hub75_set_camera(int x, int y) { camera_x = x; camera_y = y; }
void hub75_clear(void) { memset(draw_buffer, 0, FB_SIZE); }

// ==================== Core Drawing ====================

void hub75_fill(rgb_t color) {
    for (int i = 0; i < TOTAL_WIDTH * TOTAL_HEIGHT; i++) {
        draw_buffer[i * 3 + 0] = color.r;
        draw_buffer[i * 3 + 1] = color.g;
        draw_buffer[i * 3 + 2] = color.b;
    }
}

void hub75_set_pixel(int x, int y, rgb_t color) {
    int screen_x = x - camera_x;
    int screen_y = y - camera_y;
    
    if (screen_x < 0 || screen_x >= TOTAL_WIDTH || screen_y < 0 || screen_y >= TOTAL_HEIGHT) return;
    
    int idx = (screen_y * TOTAL_WIDTH + screen_x) * 3;
    draw_buffer[idx + 0] = color.r;
    draw_buffer[idx + 1] = color.g;
    draw_buffer[idx + 2] = color.b;
}

void hub75_blend_pixel(int x, int y, rgb_t new_color, uint8_t alpha) {
    int screen_x = x - camera_x;
    int screen_y = y - camera_y;
    
    if (screen_x < 0 || screen_x >= TOTAL_WIDTH || screen_y < 0 || screen_y >= TOTAL_HEIGHT) return;
    if (alpha == 0) return;
    
    int idx = (screen_y * TOTAL_WIDTH + screen_x) * 3;
    draw_buffer[idx + 0] = (new_color.r * alpha + draw_buffer[idx + 0] * (255 - alpha)) >> 8;
    draw_buffer[idx + 1] = (new_color.g * alpha + draw_buffer[idx + 1] * (255 - alpha)) >> 8;
    draw_buffer[idx + 2] = (new_color.b * alpha + draw_buffer[idx + 2] * (255 - alpha)) >> 8;
}

rgb_t hub75_get_pixel(int x, int y) {
    int screen_x = x - camera_x;
    int screen_y = y - camera_y;
    
    if (screen_x < 0 || screen_x >= TOTAL_WIDTH || screen_y < 0 || screen_y >= TOTAL_HEIGHT) return RGB_BLACK;
    
    int idx = (screen_y * TOTAL_WIDTH + screen_x) * 3;
    rgb_t c = { draw_buffer[idx + 0], draw_buffer[idx + 1], draw_buffer[idx + 2] };
    return c;
}

// ==================== Primitives ====================

void hub75_draw_hline(int x, int y, int width, rgb_t color) {
    for (int i = 0; i < width; i++) hub75_set_pixel(x + i, y, color);
}

void hub75_draw_vline(int x, int y, int height, rgb_t color) {
    for (int i = 0; i < height; i++) hub75_set_pixel(x, y + i, color);
}

void hub75_draw_line(int x0, int y0, int x1, int y1, rgb_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx - dy;
    
    while (1) {
        hub75_set_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void hub75_draw_rect(int x, int y, int width, int height, rgb_t color) {
    hub75_draw_hline(x, y, width, color);
    hub75_draw_hline(x, y + height - 1, width, color);
    hub75_draw_vline(x, y, height, color);
    hub75_draw_vline(x + width - 1, y, height, color);
}

void hub75_fill_rect(int x, int y, int width, int height, rgb_t color) {
    int screen_x = x - camera_x;
    int screen_y = y - camera_y;
    
    if (screen_x >= TOTAL_WIDTH || screen_y >= TOTAL_HEIGHT || screen_x + width <= 0 || screen_y + height <= 0) return;
    
    int start_x = max(0, screen_x);
    int start_y = max(0, screen_y);
    int end_x = min((int)TOTAL_WIDTH, screen_x + width);
    int end_y = min((int)TOTAL_HEIGHT, screen_y + height);

    for (int j = start_y; j < end_y; j++) {
        int row_offset = j * TOTAL_WIDTH;
        for (int i = start_x; i < end_x; i++) {
            int idx = (row_offset + i) * 3;
            draw_buffer[idx + 0] = color.r;
            draw_buffer[idx + 1] = color.g;
            draw_buffer[idx + 2] = color.b;
        }
    }
}

void hub75_draw_circle(int cx, int cy, int radius, rgb_t color) {
    int x = radius, y = 0, err = 0;
    while (x >= y) {
        hub75_set_pixel(cx + x, cy + y, color); hub75_set_pixel(cx + y, cy + x, color);
        hub75_set_pixel(cx - y, cy + x, color); hub75_set_pixel(cx - x, cy + y, color);
        hub75_set_pixel(cx - x, cy - y, color); hub75_set_pixel(cx - y, cy - x, color);
        hub75_set_pixel(cx + y, cy - x, color); hub75_set_pixel(cx + x, cy - y, color);
        y++;
        if (err <= 0) err += 2 * y + 1;
        if (err > 0) { x--; err -= 2 * x + 1; }
    }
}

void hub75_fill_circle(int cx, int cy, int radius, rgb_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) hub75_set_pixel(cx + x, cy + y, color);
        }
    }
}

void hub75_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, rgb_t color) {
    hub75_draw_line(x0, y0, x1, y1, color);
    hub75_draw_line(x1, y1, x2, y2, color);
    hub75_draw_line(x2, y2, x0, y0, color);
}

// ==================== Text ====================

void hub75_draw_char(int x, int y, char c, rgb_t color, int size) {
    if (c < 32 || c > 127) c = '?';
    int idx = (c - 32) * 5;
    for (int col = 0; col < 5; col++) {
        uint8_t line = pgm_read_byte(&font_5x7[idx + col]);
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                if (size == 1) hub75_set_pixel(x + col, y + row, color);
                else hub75_fill_rect(x + col * size, y + row * size, size, size, color);
            }
        }
    }
}

void hub75_draw_string(int x, int y, const char *str, rgb_t color, int size) {
    int cursor_x = x;
    while (*str) {
        if (*str == '\n') { cursor_x = x; y += 8 * size; } 
        else { hub75_draw_char(cursor_x, y, *str, color, size); cursor_x += 6 * size; }
        str++;
    }
}

int hub75_string_width(const char *str, int size) {
    int width = 0;
    while (*str) { if (*str != '\n') width += 6 * size; str++; }
    return width > 0 ? width - size : 0;
}

// ==================== Images & Sprites ====================

void hub75_draw_image(int x, int y, const uint16_t *bitmap, int w, int h) {
    int screen_x = x - camera_x;
    int screen_y = y - camera_y;

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (screen_x + i >= 0 && screen_x + i < TOTAL_WIDTH && screen_y + j >= 0 && screen_y + j < TOTAL_HEIGHT) {
                uint16_t color565 = bitmap[j * w + i];
                rgb_t c = rgb((color565 >> 8) & 0xF8, (color565 >> 3) & 0xFC, (color565 << 3) & 0xF8);
                
                int idx = ((screen_y + j) * TOTAL_WIDTH + (screen_x + i)) * 3;
                draw_buffer[idx + 0] = c.r;
                draw_buffer[idx + 1] = c.g;
                draw_buffer[idx + 2] = c.b;
            }
        }
    }
}

void hub75_draw_sprite(int x, int y, const uint16_t *bitmap, int w, int h, uint16_t transparent_color) {
    int screen_x = x - camera_x;
    int screen_y = y - camera_y;

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            uint16_t color565 = bitmap[j * w + i];
            if (color565 == transparent_color) continue;
            
            if (screen_x + i >= 0 && screen_x + i < TOTAL_WIDTH && screen_y + j >= 0 && screen_y + j < TOTAL_HEIGHT) {
                rgb_t c = rgb((color565 >> 8) & 0xF8, (color565 >> 3) & 0xFC, (color565 << 3) & 0xF8);
                
                int idx = ((screen_y + j) * TOTAL_WIDTH + (screen_x + i)) * 3;
                draw_buffer[idx + 0] = c.r;
                draw_buffer[idx + 1] = c.g;
                draw_buffer[idx + 2] = c.b;
            }
        }
    }
}

// ==================== Display Output ====================

void hub75_set_brightness(uint8_t b) { brightness = b; }

void hub75_refresh(void) {
    int half_height = TOTAL_HEIGHT / 2;
    
    for (int row = 0; row < half_height; row++) {
        digitalWrite(PIN_OE, 1); 
        set_row_address(row);
        
        for (int x = 0; x < TOTAL_WIDTH; x++) {
            int idx_top = (row * TOTAL_WIDTH + x) * 3;
            int idx_bot = ((row + half_height) * TOTAL_WIDTH + x) * 3;
            
            uint8_t r1 = (display_buffer[idx_top + 0] * brightness) >> 8;
            uint8_t g1 = (display_buffer[idx_top + 1] * brightness) >> 8;
            uint8_t b1 = (display_buffer[idx_top + 2] * brightness) >> 8;
            uint8_t r2 = (display_buffer[idx_bot + 0] * brightness) >> 8;
            uint8_t g2 = (display_buffer[idx_bot + 1] * brightness) >> 8;
            uint8_t b2 = (display_buffer[idx_bot + 2] * brightness) >> 8;
            
          // Fast SDK pushes
            gpio_put(PIN_R1, r1 > 127);
            gpio_put(PIN_G1, g1 > 127);
            gpio_put(PIN_B1, b1 > 127);
            gpio_put(PIN_R2, r2 > 127);
            gpio_put(PIN_G2, g2 > 127);
            gpio_put(PIN_B2, b2 > 127);
            
            clock_pulse();
        }
        
        latch_data();
        digitalWrite(PIN_OE, 0); 
        delayMicroseconds(100); 
    }
}

void hub75_swap_buffers(void) {
    uint8_t *temp = draw_buffer;
    draw_buffer = display_buffer;
    display_buffer = temp;
    memcpy(draw_buffer, display_buffer, FB_SIZE);
}