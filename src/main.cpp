#include <Arduino.h>
#include "config.h"
#include "hub75_driver.h"

// ==================== Game of Life Configuration ====================
#define CELL_W TOTAL_WIDTH   // 64
#define CELL_H TOTAL_HEIGHT  // 64

static uint8_t current[CELL_W][CELL_H];
static uint8_t next[CELL_W][CELL_H];

static unsigned long generation = 0;
static unsigned long last_changed_gen = 0; 
volatile bool engine_ready = false; 

// Global tracker to know exactly when the board dies
int current_population = 0;

// ==================== The Pattern Jukebox Library (50 Patterns) ====================
const char* const PATTERN_LIBRARY[] = {
    // --- METHUSELAHS (Exploders) ---
    "bo5b$3bo3b$2o2b3o!", "6bo$2o$bo3b3o!", "b2o$2o$bo!", "3o$obo$obo!", "o2b$2o2b$obo$2bo!",
    "b2o$2ob$bo!", "3ob$bobo$b2ob$2b2o!", "b3o$3ob$b2o!", "bo$o2b$b2o!", "o$3o$obo$2bo!",
    "o$o$3o!", "2o$b2o$2b2o!", "2b2o$o2b2o$b3o!", "o$2o$b3o$3bo!", 

    // --- SPACESHIPS (Travelers) ---
    "bob$2bo$3o!", "bo2bo$o4b$o3bo$4o!", "3bobo$o5b$o4bo$5o!", "3b2obo$o6b$o5bo$6o!",
    "bobo$o$bo$o3bo$2ob2o!", "b2o$o2bo$bobo$2bo!", 
    
    // --- GUNS & FACTORIES ---
    "24bo$22bobo$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o!",
    
    // --- OSCILLATORS (Looping Animations) ---
    "3o!", "b3o$3o!", "2o2b$o3b$3bo$2b2o!", "2bo$o2bo$obo$2bo!", "2bo4bo2b$2ob4ob2o$2bo4bo2b!",
    "2b3o3b3o$b$o4bobo4bo$o4bobo4bo$o4bobo4bo$2b3o3b3o$b$2b3o3b3o$o4bobo4bo$o4bobo4bo$o4bobo4bo$b$2b3o3b3o!",
    "2o$2o$2b2o$2b2o!", "b2ob2o$b2ob2o$ob2obo$o4bo$o4bo!", "2bo$obob$bob2o$2bo!",
    "3b2o$2bo2bo$bo4bo$o6bo$o6bo$bo4bo$2bo2bo$3b2o!", "2b2o$b2ob2o$o4bo$b2ob2o$2b2o!", "2o$obobo$2o$2o$obobo$2o!", 
    
    // --- STILL LIFES (Stable Structures) ---
    "2o$2o!", "b2ob$o2bo$b2ob!", "b2ob$o2bo$bobo$2bo!", "2ob$obo$b2o!", "bob$obo$bob!",
    "2o$obo$2o!", "bo$obo$obo$bo!", "bo$obo$obo$b2o!", "bo$obo$o2bo$b2o!", "2o$o2bo$o2bo$2o!",
    "2o$obo$2bo$2b2o!", "3o$o$b2o$2b2o!", "2o$obo$b2o!", "2o$o$b2o!", "2o$o2bo$2b2o!", "2o$o2bo$2o!", "2o$o$3o!"
};

const int NUM_PATTERNS = sizeof(PATTERN_LIBRARY) / sizeof(PATTERN_LIBRARY[0]);

// ==================== Core Functions ====================

void clear_board() {
    for (int y = 0; y < CELL_H; y++) {
        for (int x = 0; x < CELL_W; x++) {
            current[x][y] = 0;
            next[x][y] = 0;
        }
    }
    generation = 0;
    last_changed_gen = 0;
    current_population = 0;
}

void safe_spawn(int x, int y) {
    if (x >= 0 && x < CELL_W && y >= 0 && y < CELL_H) {
        current[x][y] = 1;
    }
}

void spawn_from_rle(int start_x, int start_y, const char* rle) {
    int x = 0, y = 0, count = 0;
    while (*rle) {
        if (*rle >= '0' && *rle <= '9') {
            count = count * 10 + (*rle - '0'); 
        } else if (*rle == 'b') { 
            if (count == 0) count = 1;
            x += count; count = 0;
        } else if (*rle == 'o') { 
            if (count == 0) count = 1;
            for (int i = 0; i < count; i++) { safe_spawn(start_x + x, start_y + y); x++; }
            count = 0;
        } else if (*rle == '$') { 
            if (count == 0) count = 1;
            y += count; x = 0; count = 0;
        } else if (*rle == '!') { break; }
        rle++;
    }
}

void spawn_random_soup(int num_cells) {
    for (int i = 0; i < num_cells; i++) {
        safe_spawn(random(0, CELL_W), random(0, CELL_H));
    }
}

// ==================== The Jukebox Engine ====================
void run_jukebox_spawner() {
    clear_board();
    
    // 33% chance for dense random soup, 66% chance for library patterns
    if (random(100) < 33) {
        // Drop 500-800 pixels (~15% density) so it survives!
        spawn_random_soup(random(500, 800)); 
    } else {
        // Spawn 1 to 3 famous patterns
        int patterns_to_drop = random(1, 4); 
        for (int i = 0; i < patterns_to_drop; i++) {
            int selected_pattern = random(0, NUM_PATTERNS);
            int spawn_x = random(5, CELL_W - 15);
            int spawn_y = random(5, CELL_H - 15);
            spawn_from_rle(spawn_x, spawn_y, PATTERN_LIBRARY[selected_pattern]);
        }
    }
}

// ==================== Game Logic ====================
int count_neighbors(int x, int y) {
    int live = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < CELL_W && ny >= 0 && ny < CELL_H) {
                if (current[nx][ny] > 0) live++;
            }
        }
    }
    return live;
}

bool compute_next_generation() {
    bool board_changed = false;
    current_population = 0; // Reset counter for this frame
    
    for (int y = 0; y < CELL_H; y++) {
        for (int x = 0; x < CELL_W; x++) {
            int neighbors = count_neighbors(x, y);
            uint8_t age = current[x][y];

            if (age > 0) {
                if (neighbors == 2 || neighbors == 3) {
                    next[x][y] = (age < 255) ? age + 1 : 255; 
                    if (age != next[x][y]) board_changed = true;
                } else {
                    next[x][y] = 0; board_changed = true;
                }
            } else {
                if (neighbors == 3) {
                    next[x][y] = 1; board_changed = true;
                } else {
                    next[x][y] = 0;
                }
            }
            
            // Track the population
            if (next[x][y] > 0) current_population++;
        }
    }
    
    memcpy(current, next, sizeof(current));
    generation++;
    return board_changed;
}

// ==================== Render ====================
void render_grid() {
    hub75_clear(); 
    for (int cy = 0; cy < CELL_H; cy++) {
        for (int cx = 0; cx < CELL_W; cx++) {
            uint8_t age = current[cx][cy];
            if (age > 0) {
                rgb_t color = (age == 1) ? RGB_WHITE : hsv_to_rgb((generation * 4 + age * 8) % 360, 255, 255);
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

    randomSeed(micros());
    run_jukebox_spawner();

    Serial.println("Conway's Game of Life (Ultimate Jukebox) started");
    engine_ready = true; 
}

// ==================== Main loop (Core 0) ====================
void loop() {
    static unsigned long last_frame = 0;

    // Limit frame rate to ~15 fps (66 ms per gen)
    unsigned long now = millis();
    if (now - last_frame < 66) { delay(1); return; }
    last_frame = now;

    // 1. Compute next generation
    bool changed = compute_next_generation();
    if (changed) last_changed_gen = generation;

    // 2. Cosmic Rays! Blow up stuck oscillators every 40 frames
    if (generation % 40 == 0 && current_population > 0) {
        safe_spawn(random(0, CELL_W), random(0, CELL_H));
        safe_spawn(random(0, CELL_W), random(0, CELL_H)); // Drop a second ray for good measure
    }

    // 3. Smart Triggers for Jukebox
    if (current_population == 0) {
        run_jukebox_spawner(); // Instant wipe if dead
    } else if (generation - last_changed_gen > 20) {
        run_jukebox_spawner(); // Wipe if truly stagnant
    } else if (generation > 400) {
        run_jukebox_spawner(); // Shorter hard limit (about 25 seconds) to keep it fresh
    }

    // 4. Render and Swap
    render_grid();
    hub75_swap_buffers();
}

// ==================== CORE 1 (Screen Refresh) ====================
void setup1() {}
void loop1() {
    if (!engine_ready) return; 
    hub75_refresh();
}