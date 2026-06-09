
/*
#include <Arduino.h>
#include "config.h"
#include "hub75_driver.h" 
#include "gps.h" 

volatile bool engine_ready = false; 
unsigned long last_redraw = 0; // Our internal stopwatch

// Custom colors matching your working lines
const rgb_t PURE_RED  = {255, 0, 0};
const rgb_t PURE_BLUE = {0, 0, 255};
const rgb_t PURPLE    = {255, 0, 255}; // Red + Blue


 


 


const rgb_t ORANGE   = {255, 165, 0};
const rgb_t YELLOW   = {255, 255, 0};
const rgb_t TEAL     = {0, 255, 200};
const rgb_t GOLD     = {255, 215, 0};



 

// ==================== Global Sprite ID ====================
int heart_id = -1;

// ==================== Animation State ====================
int scroll_x = 64;          // for scrolling text
int bounce_x = 10, bounce_y = 10;
int dx = 2, dy = 2;
uint16_t hue = 0;


// ==================== CORE 0 (Brain, GPS & Layout) ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    hub75_init();
    
    
    engine_ready = true; 
}

void loop() {
    static int frame = 0;
    static int x_pos = 0;
    static int y_pos = 32;
    static int dx = 2, dy = 2;
    
    // Clear screen
    hub75_clear();


     // ==================== 1. Gradient Background ====================
    // Draw a gradient from top-left (cyan) to bottom-right (purple)
  hub75_fill_rect_gradient(0, 0, 64, 64, TEAL, PURPLE, false);

  
    
 
    // 1. Animated bouncing rectangle (position changes)
    x_pos += dx;
    y_pos += dy;
    if (x_pos <= 0 || x_pos + 20 >= 64) dx = -dx;
    if (y_pos <= 0 || y_pos + 20 >= 64) dy = -dy;
    hub75_draw_rect(x_pos, y_pos, 20, 20, RGB_RED);
    hub75_fill_rect(x_pos+2, y_pos+2, 16, 16, RGB_BLUE);
    
    // 2. Color cycling text (different color each frame)
    rgb_t text_color;
    int hue = frame % 360;
    if (hue < 60) text_color = RGB_RED;
    else if (hue < 120) text_color = ORANGE;
    else if (hue < 180) text_color = RGB_YELLOW;
    else if (hue < 240) text_color = RGB_GREEN;
    else if (hue < 300) text_color = RGB_CYAN;
    else text_color = PURPLE;
    
    char msg[32];
    sprintf(msg, "Frame: %d", frame);
    hub75_draw_string(2, 2, msg, text_color, 1);
    
    // 3. A simple "pixel rain" effect (falling dots)
    static int rain_x[10] = {0};
    static int rain_y[10] = {0};
    for (int i = 0; i < 10; i++) {
        if (rain_y[i] >= 64) {
            rain_x[i] = random(0, 64);
            rain_y[i] = 0;
        }
        hub75_set_pixel(rain_x[i], rain_y[i], RGB_WHITE);
        rain_y[i] += 1;
    }
         
    
    // 4. Draw a border that "breathes" (changes thickness)
    int border = (frame / 10) % 5 + 1;
    hub75_draw_rect(0, 0, 64, 64, RGB_WHITE);
    if (border > 1) hub75_draw_rect(1, 1, 62, 62, RGB_WHITE);
    if (border > 2) hub75_draw_rect(2, 2, 60, 60, RGB_WHITE);
    if (border > 3) hub75_draw_rect(3, 3, 58, 58, RGB_WHITE);
    
    // Swap buffers and clear draw buffer
    hub75_swap_buffers();
    hub75_clear();
    
    frame++;
    delay(30);  // ~33 fps
}


void loopq() {
 
   


static unsigned long last_debug = 0;
 
   char satStr[16];


           hub75_draw_rect(1, 1, 63, 63,RGB_BLUE ) ;
          
            // 3. "SATS: X" at Y=32 (Bottom Half) - Printed in PURPLE
            sprintf(satStr, "test:TEST %d", 5);
            hub75_draw_string(2, 32, satStr, PURPLE, 1);
       
  
        // Push the drawing to the display buffer
        hub75_swap_buffers();
        
        // Wipe the draw buffer AFTER swapping so it is fresh for the next loop pass
        hub75_clear(); 
    
}

 

// ==================== CORE 1 (Screen Refresh) ====================
void setup1() {}

void loop1() {
    if (!engine_ready) return; 
    
    // Run your old engine's refresh system on Core 1
    hub75_refresh();
}


*/

 