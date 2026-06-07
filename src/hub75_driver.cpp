/*
 * hub75_driver.cpp - HUB75 LED Matrix Driver Implementation
 */


#include <string.h>
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
const rgb_t RGB_PURPLE    = {128, 0, 128};
const rgb_t RGB_BROWN     = {165, 42, 42};
const rgb_t RGB_PINK      = {255, 192, 203};
const rgb_t RGB_LIME      = {50, 205, 50};
const rgb_t RGB_TEAL      = {0, 128, 128};
const rgb_t RGB_NAVY      = {0, 0, 128};

const rgb_t hub75_palette[256] = {
    {0,0,0}, {0,0,170}, {0,170,0}, {0,170,170}, {170,0,0}, {170,0,170}, {170,85,0}, {170,170,170},
    {85,85,85}, {85,85,255}, {85,255,85}, {85,255,255}, {255,85,85}, {255,85,255}, {255,255,85}, {255,255,255},
    {16,16,16},
    {17,17,17},
    {18,18,18},
    {19,19,19},
    {20,20,20},
    {21,21,21},
    {22,22,22},
    {23,23,23},
    {24,24,24},
    {25,25,25},
    {26,26,26},
    {27,27,27},
    {28,28,28},
    {29,29,29},
    {30,30,30},
    {31,31,31},
    {32,32,32},
    {33,33,33},
    {34,34,34},
    {35,35,35},
    {36,36,36},
    {37,37,37},
    {38,38,38},
    {39,39,39},
    {40,40,40},
    {41,41,41},
    {42,42,42},
    {43,43,43},
    {44,44,44},
    {45,45,45},
    {46,46,46},
    {47,47,47},
    {48,48,48},
    {49,49,49},
    {50,50,50},
    {51,51,51},
    {52,52,52},
    {53,53,53},
    {54,54,54},
    {55,55,55},
    {56,56,56},
    {57,57,57},
    {58,58,58},
    {59,59,59},
    {60,60,60},
    {61,61,61},
    {62,62,62},
    {63,63,63},
    {64,64,64},
    {65,65,65},
    {66,66,66},
    {67,67,67},
    {68,68,68},
    {69,69,69},
    {70,70,70},
    {71,71,71},
    {72,72,72},
    {73,73,73},
    {74,74,74},
    {75,75,75},
    {76,76,76},
    {77,77,77},
    {78,78,78},
    {79,79,79},
    {80,80,80},
    {81,81,81},
    {82,82,82},
    {83,83,83},
    {84,84,84},
    {85,85,85},
    {86,86,86},
    {87,87,87},
    {88,88,88},
    {89,89,89},
    {90,90,90},
    {91,91,91},
    {92,92,92},
    {93,93,93},
    {94,94,94},
    {95,95,95},
    {96,96,96},
    {97,97,97},
    {98,98,98},
    {99,99,99},
    {100,100,100},
    {101,101,101},
    {102,102,102},
    {103,103,103},
    {104,104,104},
    {105,105,105},
    {106,106,106},
    {107,107,107},
    {108,108,108},
    {109,109,109},
    {110,110,110},
    {111,111,111},
    {112,112,112},
    {113,113,113},
    {114,114,114},
    {115,115,115},
    {116,116,116},
    {117,117,117},
    {118,118,118},
    {119,119,119},
    {120,120,120},
    {121,121,121},
    {122,122,122},
    {123,123,123},
    {124,124,124},
    {125,125,125},
    {126,126,126},
    {127,127,127},
    {128,128,128},
    {129,129,129},
    {130,130,130},
    {131,131,131},
    {132,132,132},
    {133,133,133},
    {134,134,134},
    {135,135,135},
    {136,136,136},
    {137,137,137},
    {138,138,138},
    {139,139,139},
    {140,140,140},
    {141,141,141},
    {142,142,142},
    {143,143,143},
    {144,144,144},
    {145,145,145},
    {146,146,146},
    {147,147,147},
    {148,148,148},
    {149,149,149},
    {150,150,150},
    {151,151,151},
    {152,152,152},
    {153,153,153},
    {154,154,154},
    {155,155,155},
    {156,156,156},
    {157,157,157},
    {158,158,158},
    {159,159,159},
    {160,160,160},
    {161,161,161},
    {162,162,162},
    {163,163,163},
    {164,164,164},
    {165,165,165},
    {166,166,166},
    {167,167,167},
    {168,168,168},
    {169,169,169},
    {170,170,170},
    {171,171,171},
    {172,172,172},
    {173,173,173},
    {174,174,174},
    {175,175,175},
    {176,176,176},
    {177,177,177},
    {178,178,178},
    {179,179,179},
    {180,180,180},
    {181,181,181},
    {182,182,182},
    {183,183,183},
    {184,184,184},
    {185,185,185},
    {186,186,186},
    {187,187,187},
    {188,188,188},
    {189,189,189},
    {190,190,190},
    {191,191,191},
    {192,192,192},
    {193,193,193},
    {194,194,194},
    {195,195,195},
    {196,196,196},
    {197,197,197},
    {198,198,198},
    {199,199,199},
    {200,200,200},
    {201,201,201},
    {202,202,202},
    {203,203,203},
    {204,204,204},
    {205,205,205},
    {206,206,206},
    {207,207,207},
    {208,208,208},
    {209,209,209},
    {210,210,210},
    {211,211,211},
    {212,212,212},
    {213,213,213},
    {214,214,214},
    {215,215,215},
    {216,216,216},
    {217,217,217},
    {218,218,218},
    {219,219,219},
    {220,220,220},
    {221,221,221},
    {222,222,222},
    {223,223,223},
    {224,224,224},
    {225,225,225},
    {226,226,226},
    {227,227,227},
    {228,228,228},
    {229,229,229},
    {230,230,230},
    {231,231,231},
    {232,232,232},
    {233,233,233},
    {234,234,234},
    {235,235,235},
    {236,236,236},
    {237,237,237},
    {238,238,238},
    {239,239,239},
    {240,240,240},
    {241,241,241},
    {242,242,242},
    {243,243,243},
    {244,244,244},
    {245,245,245},
    {246,246,246},
    {247,247,247},
    {248,248,248},
    {249,249,249},
    {250,250,250},
    {251,251,251},
    {252,252,252},
    {253,253,253},
    {254,254,254},
    {255,255,255}
};


// ==================== Framebuffer ====================

#define FB_SIZE (TOTAL_WIDTH * TOTAL_HEIGHT * 3)

static uint8_t framebuffer_a[FB_SIZE];
static uint8_t framebuffer_b[FB_SIZE];
static uint8_t *draw_buffer = framebuffer_a;
static uint8_t *display_buffer = framebuffer_b;

static uint8_t brightness = DEFAULT_BRIGHTNESS;
static volatile bool refresh_active = false;

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

// ==================== Internal Functions ====================

static inline void set_row_address(int row) {
    uint32_t set_mask = 0;
    uint32_t clr_mask = 0;

    if (row & 0x01) set_mask |= (1ul << PIN_A); else clr_mask |= (1ul << PIN_A);
    if ((row >> 1) & 0x01) set_mask |= (1ul << PIN_B); else clr_mask |= (1ul << PIN_B);
    if ((row >> 2) & 0x01) set_mask |= (1ul << PIN_C); else clr_mask |= (1ul << PIN_C);
    if ((row >> 3) & 0x01) set_mask |= (1ul << PIN_D); else clr_mask |= (1ul << PIN_D);

    #if ADDR_BITS >= 5
    if ((row >> 4) & 0x01) set_mask |= (1ul << PIN_E); else clr_mask |= (1ul << PIN_E);
    #endif

    if (set_mask) gpio_set_mask(set_mask);
    if (clr_mask) gpio_clr_mask(clr_mask);
}

static inline void clock_pulse() {
    gpio_set_mask(1ul << PIN_CLK);
    gpio_clr_mask(1ul << PIN_CLK);
}

static inline void latch_data() {
    gpio_set_mask(1ul << PIN_LAT);
    gpio_clr_mask(1ul << PIN_LAT);
}

// ==================== Public Functions ====================

void hub75_init(void) {
    Serial.printf("[HUB75] Initializing for %s board\n", BOARD_NAME);
    Serial.printf("[HUB75] Panel: %dx%d\n", TOTAL_WIDTH, TOTAL_HEIGHT);
    Serial.printf("[HUB75] Pins R1:%d G1:%d B1:%d R2:%d G2:%d B2:%d\n",
                  PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2);
    Serial.printf("[HUB75] Pins A:%d B:%d C:%d D:%d E:%d\n",
                  PIN_A, PIN_B, PIN_C, PIN_D, PIN_E);
    Serial.printf("[HUB75] Pins CLK:%d LAT:%d OE:%d\n", PIN_CLK, PIN_LAT, PIN_OE);

    // Initialize pins
    pinMode(PIN_R1, OUTPUT);
    pinMode(PIN_G1, OUTPUT);
    pinMode(PIN_B1, OUTPUT);
    pinMode(PIN_R2, OUTPUT);
    pinMode(PIN_G2, OUTPUT);
    pinMode(PIN_B2, OUTPUT);
    pinMode(PIN_A, OUTPUT);
    pinMode(PIN_B, OUTPUT);
    pinMode(PIN_C, OUTPUT);
    pinMode(PIN_D, OUTPUT);
    #if ADDR_BITS >= 5
    pinMode(PIN_E, OUTPUT);
    #endif
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_LAT, OUTPUT);
    pinMode(PIN_OE, OUTPUT);

    // Start with display off
    digitalWrite(PIN_OE, HIGH);
    digitalWrite(PIN_LAT, LOW);
    digitalWrite(PIN_CLK, LOW);

    // Clear framebuffers
    memset(framebuffer_a, 0, FB_SIZE);
    memset(framebuffer_b, 0, FB_SIZE);

    Serial.println("[HUB75] Initialized OK");
}

void hub75_clear(void) {
    memset(draw_buffer, 0, FB_SIZE);
}

void hub75_fill(rgb_t color) {
    for (int i = 0; i < TOTAL_WIDTH * TOTAL_HEIGHT; i++) {
        draw_buffer[i * 3 + 0] = color.r;
        draw_buffer[i * 3 + 1] = color.g;
        draw_buffer[i * 3 + 2] = color.b;
    }
}

void hub75_set_pixel(int x, int y, rgb_t color) {
    if (x < 0 || x >= TOTAL_WIDTH || y < 0 || y >= TOTAL_HEIGHT) return;

    int idx = (y * TOTAL_WIDTH + x) * 3;
    draw_buffer[idx + 0] = color.r;
    draw_buffer[idx + 1] = color.g;
    draw_buffer[idx + 2] = color.b;
}

void hub75_set_pixel_palette(int x, int y, uint8_t palette_idx) {
    hub75_set_pixel(x, y, hub75_palette[palette_idx]);
}

rgb_t hub75_get_pixel(int x, int y) {
    if (x < 0 || x >= TOTAL_WIDTH || y < 0 || y >= TOTAL_HEIGHT) {
        return RGB_BLACK;
    }

    int idx = (y * TOTAL_WIDTH + x) * 3;
    rgb_t c = {
        draw_buffer[idx + 0],
        draw_buffer[idx + 1],
        draw_buffer[idx + 2]
    };
    return c;
}

void hub75_draw_hline(int x, int y, int width, rgb_t color) {
    for (int i = 0; i < width; i++) {
        hub75_set_pixel(x + i, y, color);
    }
}

void hub75_draw_vline(int x, int y, int height, rgb_t color) {
    for (int i = 0; i < height; i++) {
        hub75_set_pixel(x, y + i, color);
    }
}

void hub75_draw_line(int x0, int y0, int x1, int y1, rgb_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
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
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            hub75_set_pixel(x + i, y + j, color);
        }
    }
}

void hub75_draw_circle(int cx, int cy, int radius, rgb_t color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        hub75_set_pixel(cx + x, cy + y, color);
        hub75_set_pixel(cx + y, cy + x, color);
        hub75_set_pixel(cx - y, cy + x, color);
        hub75_set_pixel(cx - x, cy + y, color);
        hub75_set_pixel(cx - x, cy - y, color);
        hub75_set_pixel(cx - y, cy - x, color);
        hub75_set_pixel(cx + y, cy - x, color);
        hub75_set_pixel(cx + x, cy - y, color);

        y++;
        if (err <= 0) err += 2 * y + 1;
        if (err > 0) { x--; err -= 2 * x + 1; }
    }
}

void hub75_fill_circle(int cx, int cy, int radius, rgb_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                hub75_set_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void hub75_draw_ellipse(int cx, int cy, int rx, int ry, rgb_t color) {
    int x = 0, y = ry;
    long rx2 = (long)rx * rx;
    long ry2 = (long)ry * ry;
    long p1 = ry2 - (rx2 * ry) + (0.25 * rx2);
    long dx = 2 * ry2 * x;
    long dy = 2 * rx2 * y;

    while (dx < dy) {
        hub75_set_pixel(cx + x, cy + y, color);
        hub75_set_pixel(cx - x, cy + y, color);
        hub75_set_pixel(cx + x, cy - y, color);
        hub75_set_pixel(cx - x, cy - y, color);
        if (p1 < 0) {
            x++;
            dx += 2 * ry2;
            p1 += dx + ry2;
        } else {
            x++;
            y--;
            dx += 2 * ry2;
            dy -= 2 * rx2;
            p1 += dx - dy + ry2;
        }
    }

    long p2 = ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    while (y >= 0) {
        hub75_set_pixel(cx + x, cy + y, color);
        hub75_set_pixel(cx - x, cy + y, color);
        hub75_set_pixel(cx + x, cy - y, color);
        hub75_set_pixel(cx - x, cy - y, color);
        if (p2 > 0) {
            y--;
            dy -= 2 * rx2;
            p2 += rx2 - dy;
        } else {
            y--;
            x++;
            dx += 2 * ry2;
            dy -= 2 * rx2;
            p2 += dx - dy + rx2;
        }
    }
}

void hub75_fill_ellipse(int cx, int cy, int rx, int ry, rgb_t color) {
    for (int y = -ry; y <= ry; y++) {
        for (int x = -rx; x <= rx; x++) {
            if (x*x*ry*ry + y*y*rx*rx <= rx*rx*ry*ry) {
                hub75_set_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void hub75_draw_round_rect(int x, int y, int w, int h, int r, rgb_t color) {
    hub75_draw_hline(x + r, y, w - 2 * r, color);
    hub75_draw_hline(x + r, y + h - 1, w - 2 * r, color);
    hub75_draw_vline(x, y + r, h - 2 * r, color);
    hub75_draw_vline(x + w - 1, y + r, h - 2 * r, color);

    int cx = r;
    int cy = 0;
    int err = 0;

    while (cx >= cy) {
        hub75_set_pixel(x + w - 1 - r + cx, y + h - 1 - r + cy, color);
        hub75_set_pixel(x + w - 1 - r + cy, y + h - 1 - r + cx, color);
        hub75_set_pixel(x + r - cx, y + h - 1 - r + cy, color);
        hub75_set_pixel(x + r - cy, y + h - 1 - r + cx, color);
        hub75_set_pixel(x + r - cx, y + r - cy, color);
        hub75_set_pixel(x + r - cy, y + r - cx, color);
        hub75_set_pixel(x + w - 1 - r + cy, y + r - cx, color);
        hub75_set_pixel(x + w - 1 - r + cx, y + r - cy, color);

        cy++;
        if (err <= 0) err += 2 * cy + 1;
        if (err > 0) { cx--; err -= 2 * cx + 1; }
    }
}

void hub75_fill_round_rect(int x, int y, int w, int h, int r, rgb_t color) {
    hub75_fill_rect(x + r, y, w - 2 * r, h, color);
    for (int j = 0; j < r; j++) {
        for (int i = 0; i < r; i++) {
            if ((i - r)*(i - r) + (j - r)*(j - r) <= r*r) {
                hub75_set_pixel(x + i, y + j, color);
                hub75_set_pixel(x + w - 1 - i, y + j, color);
                hub75_set_pixel(x + i, y + h - 1 - j, color);
                hub75_set_pixel(x + w - 1 - i, y + h - 1 - j, color);
            }
        }
    }
    hub75_fill_rect(x, y + r, r, h - 2 * r, color);
    hub75_fill_rect(x + w - r, y + r, r, h - 2 * r, color);
}

void hub75_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, rgb_t color) {
    hub75_draw_line(x0, y0, x1, y1, color);
    hub75_draw_line(x1, y1, x2, y2, color);
    hub75_draw_line(x2, y2, x0, y0, color);
}

void hub75_draw_char(int x, int y, char c, rgb_t color, int size) {
    if (c < 32 || c > 127) c = '?';

    int idx = (c - 32) * 5;

    for (int col = 0; col < 5; col++) {
        uint8_t line = pgm_read_byte(&font_5x7[idx + col]);
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                if (size == 1) {
                    hub75_set_pixel(x + col, y + row, color);
                } else {
                    hub75_fill_rect(x + col * size, y + row * size, size, size, color);
                }
            }
        }
    }
}

void hub75_draw_string(int x, int y, const char *str, rgb_t color, int size) {
    int cursor_x = x;

    while (*str) {
        if (*str == '\n') {
            cursor_x = x;
            y += 8 * size;
        } else {
            hub75_draw_char(cursor_x, y, *str, color, size);
            cursor_x += 6 * size;
        }
        str++;
    }
}

int hub75_string_width(const char *str, int size) {
    int width = 0;
    while (*str) {
        if (*str != '\n') width += 6 * size;
        str++;
    }
    return width > 0 ? width - size : 0;
}

void hub75_set_brightness(uint8_t b) {
    brightness = b;
}

void hub75_refresh(void) {
    int half_height = TOTAL_HEIGHT / 2;

    for (int row = 0; row < half_height; row++) {
        // Disable output during row switch
        gpio_set_mask(1ul << PIN_OE);

        // Set row address
        set_row_address(row);

        // Shift out pixel data
        for (int x = 0; x < TOTAL_WIDTH; x++) {
            int idx_top = (row * TOTAL_WIDTH + x) * 3;
            int idx_bot = ((row + half_height) * TOTAL_WIDTH + x) * 3;

            uint8_t r1 = (display_buffer[idx_top + 0] * brightness) >> 8;
            uint8_t g1 = (display_buffer[idx_top + 1] * brightness) >> 8;
            uint8_t b1 = (display_buffer[idx_top + 2] * brightness) >> 8;
            uint8_t r2 = (display_buffer[idx_bot + 0] * brightness) >> 8;
            uint8_t g2 = (display_buffer[idx_bot + 1] * brightness) >> 8;
            uint8_t b2 = (display_buffer[idx_bot + 2] * brightness) >> 8;

            uint32_t set_mask = 0;
            uint32_t clr_mask = 0;

            if (r1 > 127) set_mask |= (1ul << PIN_R1); else clr_mask |= (1ul << PIN_R1);
            if (g1 > 127) set_mask |= (1ul << PIN_G1); else clr_mask |= (1ul << PIN_G1);
            if (b1 > 127) set_mask |= (1ul << PIN_B1); else clr_mask |= (1ul << PIN_B1);
            if (r2 > 127) set_mask |= (1ul << PIN_R2); else clr_mask |= (1ul << PIN_R2);
            if (g2 > 127) set_mask |= (1ul << PIN_G2); else clr_mask |= (1ul << PIN_G2);
            if (b2 > 127) set_mask |= (1ul << PIN_B2); else clr_mask |= (1ul << PIN_B2);

            if (set_mask) gpio_set_mask(set_mask);
            if (clr_mask) gpio_clr_mask(clr_mask);

            clock_pulse();
        }

        // Latch data
        latch_data();

        // Enable output
        gpio_clr_mask(1ul << PIN_OE);

        // Hold row active
        delayMicroseconds(100);
    }
}

void hub75_swap_buffers(void) {
    uint8_t *temp = draw_buffer;
    draw_buffer = display_buffer;
    display_buffer = temp;
    memcpy(draw_buffer, display_buffer, FB_SIZE);
}

void hub75_putimage(int x, int y, int width, int height, const rgb_t *image) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            hub75_set_pixel(x + i, y + j, image[j * width + i]);
        }
    }
}

void hub75_putimage_palette(int x, int y, int width, int height, const uint8_t *image) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            hub75_set_pixel_palette(x + i, y + j, image[j * width + i]);
        }
    }
}
