 
 //Pico 2 W + Seengreat HUB75 Test
 // 
 // Simple test to verify HUB75 display works
 //Shows "HELLO" and cycles colors
 
 

 #include <Arduino.h>
#include <WiFi.h>
#include "config.h"
 #include "E:\PlatformIO\Seengrate\include\hub75_driver.h"
 
 
//#include "display_renderer.h"
#include <Arduino.h>


// Pico 2 W + Seengreat HUB75 Test
 //
 //Simple test to verify HUB75 display works
 //Shows "HELLO" and cycles colors
 

#include <Arduino.h>

// ==================== SEENGREAT PIN MAPPING ====================
#define PIN_R1   2
#define PIN_G1   3
#define PIN_B1   4
#define PIN_R2   5
#define PIN_G2   8
#define PIN_B2   9
#define PIN_A    10
#define PIN_B    16
#define PIN_C    18
#define PIN_D    20
#define PIN_E    22
#define PIN_CLK  11
#define PIN_LAT  12
#define PIN_OE   13

// Panel size
#define WIDTH  64
#define HEIGHT 32
#define ROWS   (HEIGHT / 2)  // 16 rows (top and bottom scanned together)

// Framebuffer: RGB for each pixel
uint8_t framebuffer[WIDTH * HEIGHT * 3];

// ==================== 5x7 FONT ====================
const uint8_t font5x7[] = {
    // Space
    0x00, 0x00, 0x00, 0x00, 0x00,
    // ! " # $ % & ' ( ) * + , - . /
    0x00, 0x00, 0x5F, 0x00, 0x00,  // !
    0x00, 0x07, 0x00, 0x07, 0x00,  // "
    0x14, 0x7F, 0x14, 0x7F, 0x14,  // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12,  // $
    0x23, 0x13, 0x08, 0x64, 0x62,  // %
    0x36, 0x49, 0x55, 0x22, 0x50,  // &
    0x00, 0x05, 0x03, 0x00, 0x00,  // '
    0x00, 0x1C, 0x22, 0x41, 0x00,  // (
    0x00, 0x41, 0x22, 0x1C, 0x00,  // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08,  // *
    0x08, 0x08, 0x3E, 0x08, 0x08,  // +
    0x00, 0x50, 0x30, 0x00, 0x00,  // ,
    0x08, 0x08, 0x08, 0x08, 0x08,  // -
    0x00, 0x60, 0x60, 0x00, 0x00,  // .
    0x20, 0x10, 0x08, 0x04, 0x02,  // /
    // 0-9
    0x3E, 0x51, 0x49, 0x45, 0x3E,  // 0
    0x00, 0x42, 0x7F, 0x40, 0x00,  // 1
    0x42, 0x61, 0x51, 0x49, 0x46,  // 2
    0x21, 0x41, 0x45, 0x4B, 0x31,  // 3
    0x18, 0x14, 0x12, 0x7F, 0x10,  // 4
    0x27, 0x45, 0x45, 0x45, 0x39,  // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30,  // 6
    0x01, 0x71, 0x09, 0x05, 0x03,  // 7
    0x36, 0x49, 0x49, 0x49, 0x36,  // 8
    0x06, 0x49, 0x49, 0x29, 0x1E,  // 9
    // : ; < = > ? @
    0x00, 0x36, 0x36, 0x00, 0x00,  // :
    0x00, 0x56, 0x36, 0x00, 0x00,  // ;
    0x00, 0x08, 0x14, 0x22, 0x41,  // <
    0x14, 0x14, 0x14, 0x14, 0x14,  // =
    0x41, 0x22, 0x14, 0x08, 0x00,  // >
    0x02, 0x01, 0x51, 0x09, 0x06,  // ?
    0x32, 0x49, 0x79, 0x41, 0x3E,  // @
    // A-Z
    0x7E, 0x11, 0x11, 0x11, 0x7E,  // A
    0x7F, 0x49, 0x49, 0x49, 0x36,  // B
    0x3E, 0x41, 0x41, 0x41, 0x22,  // C
    0x7F, 0x41, 0x41, 0x22, 0x1C,  // D
    0x7F, 0x49, 0x49, 0x49, 0x41,  // E
    0x7F, 0x09, 0x09, 0x01, 0x01,  // F
    0x3E, 0x41, 0x41, 0x51, 0x32,  // G
    0x7F, 0x08, 0x08, 0x08, 0x7F,  // H
    0x00, 0x41, 0x7F, 0x41, 0x00,  // I
    0x20, 0x40, 0x41, 0x3F, 0x01,  // J
    0x7F, 0x08, 0x14, 0x22, 0x41,  // K
    0x7F, 0x40, 0x40, 0x40, 0x40,  // L
    0x7F, 0x02, 0x04, 0x02, 0x7F,  // M
    0x7F, 0x04, 0x08, 0x10, 0x7F,  // N
    0x3E, 0x41, 0x41, 0x41, 0x3E,  // O
    0x7F, 0x09, 0x09, 0x09, 0x06,  // P
    0x3E, 0x41, 0x51, 0x21, 0x5E,  // Q
    0x7F, 0x09, 0x19, 0x29, 0x46,  // R
    0x46, 0x49, 0x49, 0x49, 0x31,  // S
    0x01, 0x01, 0x7F, 0x01, 0x01,  // T
    0x3F, 0x40, 0x40, 0x40, 0x3F,  // U
    0x1F, 0x20, 0x40, 0x20, 0x1F,  // V
    0x7F, 0x20, 0x18, 0x20, 0x7F,  // W
    0x63, 0x14, 0x08, 0x14, 0x63,  // X
    0x03, 0x04, 0x78, 0x04, 0x03,  // Y
    0x61, 0x51, 0x49, 0x45, 0x43,  // Z
};

// ==================== DISPLAY FUNCTIONS ====================

void initPins() {
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
    pinMode(PIN_E, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_LAT, OUTPUT);
    pinMode(PIN_OE, OUTPUT);
    
    digitalWrite(PIN_OE, HIGH);  // Display off
    digitalWrite(PIN_LAT, LOW);
    digitalWrite(PIN_CLK, LOW);
}

void setRowAddress(int row) {
    digitalWrite(PIN_A, row & 1);
    digitalWrite(PIN_B, (row >> 1) & 1);
    digitalWrite(PIN_C, (row >> 2) & 1);
    digitalWrite(PIN_D, (row >> 3) & 1);
    // PIN_E for 64x64 panels only
}

void clockPulse() {
    digitalWrite(PIN_CLK, HIGH);
    digitalWrite(PIN_CLK, LOW);
}

void latchData() {
    digitalWrite(PIN_LAT, HIGH);
    digitalWrite(PIN_LAT, LOW);
}

void clearFramebuffer() {
    memset(framebuffer, 0, sizeof(framebuffer));
}

void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    int idx = (y * WIDTH + x) * 3;
    framebuffer[idx + 0] = r;
    framebuffer[idx + 1] = g;
    framebuffer[idx + 2] = b;
}

void fillRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            setPixel(x + i, y + j, r, g, b);
        }
    }
}

void drawChar(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b) {
    if (c < 32 || c > 90) c = '?';
    int idx = (c - 32) * 5;
    
    for (int col = 0; col < 5; col++) {
        uint8_t line = font5x7[idx + col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                setPixel(x + col, y + row, r, g, b);
            }
        }
    }
}

void drawString(int x, int y, const char* str, uint8_t r, uint8_t g, uint8_t b) {
    while (*str) {
        drawChar(x, y, *str, r, g, b);
        x += 6;
        str++;
    }
}

// Refresh display - call this frequently!
void refreshDisplay() {
    for (int row = 0; row < ROWS; row++) {
        // Disable output during row switch
        digitalWrite(PIN_OE, HIGH);
        
        // Set row address
        setRowAddress(row);
        
        // Shift out pixel data for this row
        for (int x = 0; x < WIDTH; x++) {
            // Top half pixel
            int idxTop = (row * WIDTH + x) * 3;
            // Bottom half pixel
            int idxBot = ((row + ROWS) * WIDTH + x) * 3;
            
            // Get colors (threshold at 128 for 1-bit color)
            digitalWrite(PIN_R1, framebuffer[idxTop + 0] > 128);
            digitalWrite(PIN_G1, framebuffer[idxTop + 1] > 128);
            digitalWrite(PIN_B1, framebuffer[idxTop + 2] > 128);
            digitalWrite(PIN_R2, framebuffer[idxBot + 0] > 128);
            digitalWrite(PIN_G2, framebuffer[idxBot + 1] > 128);
            digitalWrite(PIN_B2, framebuffer[idxBot + 2] > 128);
            
            clockPulse();
        }
        
        // Latch the data
        latchData();
        
        // Enable output
        digitalWrite(PIN_OE, LOW);
        
        // Hold row active (adjust for brightness)
        delayMicroseconds(100);
    }
}

// ==================== SETUP & LOOP ====================

unsigned long lastColorChange = 0;
int colorIndex = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("================================");
    Serial.println("  HUB75 Test - Seengreat Adapter");
    Serial.println("================================");
    Serial.println();
    Serial.println("Pin Configuration:");
    Serial.printf("  R1=%d G1=%d B1=%d\n", PIN_R1, PIN_G1, PIN_B1);
    Serial.printf("  R2=%d G2=%d B2=%d\n", PIN_R2, PIN_G2, PIN_B2);
    Serial.printf("  A=%d B=%d C=%d D=%d E=%d\n", PIN_A, PIN_B, PIN_C, PIN_D, PIN_E);
    Serial.printf("  CLK=%d LAT=%d OE=%d\n", PIN_CLK, PIN_LAT, PIN_OE);
    Serial.println();
    
    // Initialize pins
    initPins();
    Serial.println("Pins initialized");
    
    // Clear display
    clearFramebuffer();
    Serial.println("Framebuffer cleared");
    
    Serial.println();
    Serial.println("Display should show 'HELLO' now!");
    Serial.println("Colors will cycle every 2 seconds.");
    Serial.println();
}

void loop() {
    // Change color every 2 seconds
    if (millis() - lastColorChange > 2000) {
        lastColorChange = millis();
        colorIndex = (colorIndex + 1) % 7;
        
        // Clear and redraw
        clearFramebuffer();
        
        uint8_t r = 0, g = 0, b = 0;
        const char* colorName = "";
        
        switch (colorIndex) {
            case 0: r = 255; g = 0;   b = 0;   colorName = "RED";     break;
            case 1: r = 0;   g = 255; b = 0;   colorName = "GREEN";   break;
            case 2: r = 0;   g = 0;   b = 255; colorName = "BLUE";    break;
            case 3: r = 255; g = 255; b = 0;   colorName = "YELLOW";  break;
            case 4: r = 0;   g = 255; b = 255; colorName = "CYAN";    break;
            case 5: r = 255; g = 0;   b = 255; colorName = "MAGENTA"; break;
            case 6: r = 255; g = 255; b = 255; colorName = "WHITE";   break;
        }
        
        // Draw "HELLO" in top half (rows 0-15)
        drawString(17, 4, "HELLO", r, g, b);
        
        // Draw "PICO2" in bottom half (rows 16-31)
        // Must start at row 16 or below to be fully in bottom half
        drawString(17, 20, "PICO2", r, g, b);
        
        // Draw border
        for (int x = 0; x < WIDTH; x++) {
            setPixel(x, 0, r/2, g/2, b/2);
            setPixel(x, HEIGHT-1, r/2, g/2, b/2);
        }
        for (int y = 0; y < HEIGHT; y++) {
            setPixel(0, y, r/2, g/2, b/2);
            setPixel(WIDTH-1, y, r/2, g/2, b/2);
        }
        
        Serial.printf("Color: %s\n", colorName);
    }
    
    // Continuously refresh the display
    refreshDisplay();
}

 