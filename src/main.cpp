#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "hub75_driver.h"

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

    // Initialize pins using the driver
    hub75_init();
    Serial.println("Pins initialized");

    // Clear display
    hub75_clear();
    hub75_swap_buffers();
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
        hub75_clear();

        rgb_t c = RGB_BLACK;
        const char* colorName = "";

        switch (colorIndex) {
            case 0: c = RGB_RED;     colorName = "RED";     break;
            case 1: c = RGB_GREEN;   colorName = "GREEN";   break;
            case 2: c = RGB_BLUE;    colorName = "BLUE";    break;
            case 3: c = RGB_YELLOW;  colorName = "YELLOW";  break;
            case 4: c = RGB_CYAN;    colorName = "CYAN";    break;
            case 5: c = RGB_MAGENTA; colorName = "MAGENTA"; break;
            case 6: c = RGB_WHITE;   colorName = "WHITE";   break;
        }

        // Draw "HELLO" in top half (rows 0-15)
        hub75_draw_string(17, 4, "HELLO", c, 1);

        // Draw "PICO2" in bottom half (rows 16-31)
        hub75_draw_string(17, 20, "PICO2", c, 1);

        // Draw border
        rgb_t half_c = { (uint8_t)(c.r/2), (uint8_t)(c.g/2), (uint8_t)(c.b/2) };
        hub75_draw_rect(0, 0, TOTAL_WIDTH, TOTAL_HEIGHT, half_c);

        // Draw an ellipse and round rect for testing the new primitives
        hub75_draw_ellipse(TOTAL_WIDTH/2, TOTAL_HEIGHT/2, 10, 5, RGB_PURPLE);
        hub75_fill_round_rect(2, 2, 8, 8, 2, RGB_LIME);

        hub75_swap_buffers();
        Serial.printf("Color: %s\n", colorName);
    }

    // Continuously refresh the display
    hub75_refresh();
}