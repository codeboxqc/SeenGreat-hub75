#include <Arduino.h>
#include "config.h"
#include "hub75_driver.h"

// ==================== Game of Life Configuration ====================
// 1 logical cell = 1 pixel on the matrix
#define CELL_W TOTAL_WIDTH   // 64
#define CELL_H TOTAL_HEIGHT  // 64

// Grids for current and next generation (Tracks age: 0 = dead, >0 = alive)
static uint8_t current[CELL_W][CELL_H];
static uint8_t next[CELL_W][CELL_H];

static unsigned long generation = 0;
static unsigned long last_changed_gen = 0; // Tracks stagnation
volatile bool engine_ready = false; 

// ==================== The Pattern Jukebox Library ====================
// Paste your RLE strings here! The engine will pick from them randomly.
// To add more, just add a comma and paste the next string in quotes.
// ==================== The Pattern Jukebox Library (50 Patterns) ====================
// A massive collection of Methuselahs, Spaceships, Oscillators, and Still Lifes.
const char* const PATTERN_LIBRARY[] = {
    // --- METHUSELAHS (Exploders) ---
    "bo5b$3bo3b$2o2b3o!",      // 0: The Acorn (5200+ generations)
    "6bo$2o$bo3b3o!",           // 1: Diehard (Vanishes after 130)
    "b2o$2o$bo!",               // 2: R-Pentomino (1100+ generations)
    "3o$obo$obo!",              // 3: Pi-heptomino 
    "o2b$2o2b$obo$2bo!",        // 4: B-heptomino
    "b2o$2ob$bo!",              // 5: F-pentomino
    "3ob$bobo$b2ob$2b2o!",      // 6: Century
    "b3o$3ob$b2o!",             // 7: C-heptomino
    "bo$o2b$b2o!",              // 8: W-pentomino
    "o$3o$obo$2bo!",            // 9: Herschel
    "o$o$3o!",                  // 10: V-pentomino
    "2o$b2o$2b2o!",             // 11: Z-hexomino
    "2b2o$o2b2o$b3o!",          // 12: Bunnies
    "o$2o$b3o$3bo!",            // 13: Switch Engine precursor

    // --- SPACESHIPS (Travelers) ---
    "bob$2bo$3o!",              // 14: Glider
    "bo2bo$o4b$o3bo$4o!",       // 15: Lightweight Spaceship (LWSS)
    "3bobo$o5b$o4bo$5o!",       // 16: Middleweight Spaceship (MWSS)
    "3b2obo$o6b$o5bo$6o!",      // 17: Heavyweight Spaceship (HWSS)
    "bobo$o$bo$o3bo$2ob2o!",    // 18: Copperhead
    "b2o$o2bo$bobo$2bo!",       // 19: Loafer precursor
    
    // --- GUNS & FACTORIES ---
    "24bo$22bobo$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o!", // 20: Gosper Glider Gun
    
    // --- OSCILLATORS (Looping Animations) ---
    "3o!",                      // 21: Blinker (Period 2)
    "b3o$3o!",                  // 22: Toad (Period 2)
    "2o2b$o3b$3bo$2b2o!",       // 23: Beacon (Period 2)
    "2bo$o2bo$obo$2bo!",        // 24: Clock (Period 2)
    "2bo4bo2b$2ob4ob2o$2bo4bo2b!", // 25: Pentadecathlon (Period 15)
    "2b3o3b3o$b$o4bobo4bo$o4bobo4bo$o4bobo4bo$2b3o3b3o$b$2b3o3b3o$o4bobo4bo$o4bobo4bo$o4bobo4bo$b$2b3o3b3o!", // 26: Pulsar (Period 3)
    "2o$2o$2b2o$2b2o!",         // 27: Figure Eight (Period 8)
    "b2ob2o$b2ob2o$ob2obo$o4bo$o4bo!", // 28: Tumbler (Period 14)
    "2bo$obob$bob2o$2bo!",      // 29: Clock 2
    "3b2o$2bo2bo$bo4bo$o6bo$o6bo$bo4bo$2bo2bo$3b2o!", // 30: Octagon 2 (Period 5)
    "2b2o$b2ob2o$o4bo$b2ob2o$2b2o!", // 31: Honey Farm
    "2o$obobo$2o$2o$obobo$2o!", // 32: Spark Coil
    
    // --- STILL LIFES (Stable Structures) ---
    "2o$2o!",                   // 33: Block
    "b2ob$o2bo$b2ob!",          // 34: Beehive
    "b2ob$o2bo$bobo$2bo!",      // 35: Loaf
    "2ob$obo$b2o!",             // 36: Boat
    "bob$obo$bob!",             // 37: Tub
    "2o$obo$2o!",               // 38: Ship
    "bo$obo$obo$bo!",           // 39: Barge
    "bo$obo$obo$b2o!",          // 40: Long boat
    "bo$obo$o2bo$b2o!",         // 41: Mango
    "2o$o2bo$o2bo$2o!",         // 42: Pond
    "2o$obo$2bo$2b2o!",         // 43: Eater 1
    "3o$o$b2o$2b2o!",           // 44: Eater 2
    "2o$obo$b2o!",              // 45: Snake
    "2o$o$b2o!",                // 46: Carrier
    "2o$o2bo$2b2o!",            // 47: Aircraft carrier
    "2o$o2bo$2o!",              // 48: Elevener
    "2o$o$3o!"                  // 49: Schmoo
};

// Calculates exactly how many patterns are in your library dynamically
const int NUM_PATTERNS = sizeof(PATTERN_LIBRARY) / sizeof(PATTERN_LIBRARY[0]);

 

// ==================== Pattern Spawners ====================

// Clears the board completely
void clear_board() {
    for (int y = 0; y < CELL_H; y++) {
        for (int x = 0; x < CELL_W; x++) {
            current[x][y] = 0;
            next[x][y] = 0;
        }
    }
    generation = 0;
    last_changed_gen = 0;
}

// Safely sets a cell to alive, preventing crashes if patterns hit the boundary
void safe_spawn(int x, int y) {
    if (x >= 0 && x < CELL_W && y >= 0 && y < CELL_H) {
        current[x][y] = 1;
    }
}

// ==================== The RLE Parser ====================
// Reads the compressed text strings from the LifeWiki into actual matrix coordinates
void spawn_from_rle(int start_x, int start_y, const char* rle) {
    int x = 0, y = 0, count = 0;
    while (*rle) {
        if (*rle >= '0' && *rle <= '9') {
            count = count * 10 + (*rle - '0'); 
        } else if (*rle == 'b') { // Dead cell
            if (count == 0) count = 1;
            x += count;
            count = 0;
        } else if (*rle == 'o') { // Live cell
            if (count == 0) count = 1;
            for (int i = 0; i < count; i++) {
                safe_spawn(start_x + x, start_y + y);
                x++;
            }
            count = 0;
        } else if (*rle == '$') { // New row
            if (count == 0) count = 1;
            y += count;
            x = 0;
            count = 0;
        } else if (*rle == '!') { // End of pattern
            break;
        }
        rle++;
    }
}

// ==================== 50 Random Spawner ====================
// Sprinkles a precise number of completely random pixels (Mini Soup)
void spawn_random_soup(int num_cells) {
    for (int i = 0; i < num_cells; i++) {
        int rx = random(0, CELL_W);
        int ry = random(0, CELL_H);
        safe_spawn(rx, ry);
    }
}

// ==================== The Jukebox Engine ====================
// Randomly decides how to repopulate the board when it stagnates
void run_jukebox_spawner() {
    clear_board();
    
    // 33% chance to spawn 60-75 random pixels, 66% chance to use the RLE library
    if (random(100) < 33) {
        // Mode 1: Pure random soup (60 to 75 cells)
        int cell_count = random(60, 76);
        spawn_random_soup(cell_count);
    } else {
        // Mode 2: Spawn 1 to 3 famous patterns from our library at random spots
        int patterns_to_drop = random(1, 4); 
        for (int i = 0; i < patterns_to_drop; i++) {
            int selected_pattern = random(0, NUM_PATTERNS);
            // Keep spawns mostly on-screen by padding the random coordinates
            int spawn_x = random(5, CELL_W - 15);
            int spawn_y = random(5, CELL_H - 15);
            
            spawn_from_rle(spawn_x, spawn_y, PATTERN_LIBRARY[selected_pattern]);
        }
    }
}

// ==================== Helper: Count live neighbors (Hard Boundaries) ====================
int count_neighbors(int x, int y) {
    int live = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            
            int nx = x + dx;
            int ny = y + dy;
            
            // Hard boundaries: Only count if the neighbor is physically on the matrix
            if (nx >= 0 && nx < CELL_W && ny >= 0 && ny < CELL_H) {
                if (current[nx][ny] > 0) live++;
            }
        }
    }
    return live;
}

// We will use this global variable to track if the screen goes totally black
int current_population = 0; 

// ==================== Compute next generation ====================
bool compute_next_generation() {
    bool board_changed = false;
    current_population = 0; // Reset the counter for this frame
    
    for (int y = 0; y < CELL_H; y++) {
        for (int x = 0; x < CELL_W; x++) {
            int neighbors = count_neighbors(x, y);
            uint8_t age = current[x][y];

            if (age > 0) {
                // Survival
                if (neighbors == 2 || neighbors == 3) {
                    next[x][y] = (age < 255) ? age + 1 : 255; 
                    if (age != next[x][y]) board_changed = true;
                } else {
                    // Death
                    next[x][y] = 0;
                    board_changed = true;
                }
            } else {
                // Birth
                if (neighbors == 3) {
                    next[x][y] = 1;
                    board_changed = true;
                } else {
                    next[x][y] = 0;
                }
            }
            
            // Count how many are alive for the instant-reset check!
            if (next[x][y] > 0) {
                current_population++;
            }
        }
    }
    
    memcpy(current, next, sizeof(current));
    generation++;
    
    return board_changed;
}


// ==================== Render current grid to display ====================
void render_grid() {
    hub75_clear();  // Clear the draw buffer

    for (int cy = 0; cy < CELL_H; cy++) {
        for (int cx = 0; cx < CELL_W; cx++) {
            uint8_t age = current[cx][cy];
            if (age > 0) {
                rgb_t color;
                
                // Color Explosion Logic!
                if (age == 1) {
                    // Newborn cells flash stark white for maximum contrast
                    color = RGB_WHITE; 
                } else {
                    // Older cells shift through the rainbow based on BOTH age and generation.
                    uint16_t hue = (generation * 4 + age * 8) % 360; 
                    color = hsv_to_rgb(hue, 255, 255);
                }

                // Draw a single pixel for 1:1 native resolution mapping
                hub75_set_pixel(cx, cy, color);
            }
        }
    }
}

// ==================== Setup (Core 0) ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    hub75_init();
    hub75_set_brightness(200);

    // Initialize the random number generator
    randomSeed(micros());

    // Kick off the first generation using the Jukebox!
    run_jukebox_spawner();

    Serial.println("Conway's Game of Life (Endless Jukebox Edition) started");
    engine_ready = true; 
}

// ==================== Main loop (Core 0) ====================
// ==================== Main loop (Core 0) ====================
void loop() {
    static unsigned long last_frame = 0;

    // Limit frame rate to ~15 fps (66 ms per gen)
    unsigned long now = millis();
    if (now - last_frame < 66) {
        delay(1);
        return;
    }
    last_frame = now;

    // 1. Compute the next generation
    bool changed = compute_next_generation();
    if (changed) {
        last_changed_gen = generation;
    }

    // --- NEW: THE COSMIC RAY DISRUPTOR ---
    // Every 40 frames, if the board is still alive, drop 1 random pixel on it.
    // If the board is stuck in an endless oscillator loop, this will blow it up!
    if (generation % 40 == 0 && current_population > 0) {
        safe_spawn(random(0, CELL_W), random(0, CELL_H));
    }

    // --- NEW: THE SMART AUTO-RESET ENGINE ---
    // Trigger 1: INSTANT RESET. If the board is totally empty, don't wait.
    if (current_population == 0) {
        run_jukebox_spawner();
    }
    // Trigger 2: TRUE STAGNATION. If absolutely nothing has moved in 20 frames.
    else if (generation - last_changed_gen > 50) {
        run_jukebox_spawner();
    }
    // Trigger 3: THE HARD LIMIT. Cut down to 800 frames so we cycle through 
    // the 50 patterns much faster and keep things visually exciting.
    else if (generation > 800) {
        run_jukebox_spawner(); 
    }

    // 3. Render the new generation into the draw buffer
    render_grid();

    // 4. Swap buffers – the display now shows the new generation
    hub75_swap_buffers();
}

// ==================== CORE 1 (Screen Refresh) ====================
void setup1() {}

void loop1() {
    if (!engine_ready) return; 
    
    // Run the HUB75 driver's refresh system purely on Core 1
    // This ensures your LED matrix never flickers while Core 0 does the heavy math!
    hub75_refresh();
}