/*
 * Pico 2 W + Seengreat HUB75 - MAX POWER DUAL CORE ENGINE
 * Core 0: Handles animations, math, and logic.
 * Core 1: Dedicated entirely to blasting pixels to the screen.
 */

#include <Arduino.h>
#include "config.h"
#include "hub75_driver.h" 

unsigned long lastColorChange = 0;
int colorIndex = -1; 

// ==================== CORE 0 (Brain & Animation) ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize the HUB75 driver memory and pins
    hub75_init();
    
    Serial.println("Dual-Core Engine Booted!");
}

void loop() {
    // Change color every 2 seconds
    if (millis() - lastColorChange > 2000 || colorIndex == -1) {
        lastColorChange = millis();
        colorIndex = (colorIndex + 1) % 7;
        
        hub75_clear();
        
        rgb_t c;
        switch (colorIndex) {
            case 0: c = RGB_RED;     break;
            case 1: c = RGB_GREEN;   break;
            case 2: c = RGB_BLUE;    break;
            case 3: c = RGB_YELLOW;  break;
            case 4: c = RGB_CYAN;    break;
            case 5: c = RGB_MAGENTA; break;
            case 6: c = RGB_WHITE;   break;
        }
        
        hub75_draw_string(17, 4, "HELLO", c, 1);
        hub75_draw_string(17, 20, "PICO2", c, 1);
        
        rgb_t border_color = rgb(c.r / 2, c.g / 2, c.b / 2);
        hub75_draw_rect(0, 0, TOTAL_WIDTH, TOTAL_HEIGHT, border_color);
        
        // Push drawing to the active display buffer
        hub75_swap_buffers();
    }
    
    // Core 0 is completely free to do other things here!
    // You can add Wi-Fi handling, read sensors, etc.


    // THE STRESS TEST: Force Core 0 to completely freeze for half a second!
    delay(500);
    
}

// ==================== CORE 1 (Screen Refresh) ====================
// This automatically runs on the second physical CPU core!

void setup1() {
    // Give Core 0 a tiny head start to initialize memory
    delay(10); 
}

void loop1() {
    // This loops infinitely on Core 1, pushing the display_buffer 
    // to the matrix at maximum hardware speed. No flickering, ever.
    hub75_refresh();
}