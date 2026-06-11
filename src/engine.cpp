#include "engine.h"
#include <math.h>

// Virtual resolution to sync physics parity with the 256x256 JS simulator
const float V_WIDTH = 256.0f;
const float V_HEIGHT = 256.0f;

// Sharp, 1-pixel thin line algorithm (Bresenham) to replace anti-aliased glowing lines
static void drawThinLine(int x0, int y0, int x1, int y1, rgb_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        hub75_set_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

const rgb_t basePalettes[40][5] = {
    { {255,0,0}, {255,127,0}, {255,255,0}, {255,0,255}, {255,255,255} }, // Plasma
    { {0,255,255}, {0,136,255}, {0,0,255}, {255,255,255}, {136,204,255} }, // Ice
    { {255,0,0}, {255,68,0}, {255,136,0}, {255,204,0}, {34,0,0} }, // Fire
    { {255,0,255}, {0,255,255}, {43,0,255}, {255,0,85}, {0,0,34} }, // Synthwave
    { {252,238,10}, {255,0,60}, {0,240,255}, {0,0,0}, {17,17,17} }, // Cyberpunk
    { {0,68,0}, {0,136,0}, {34,204,34}, {136,255,136}, {34,17,0} }, // Forest
    { {0,17,51}, {0,51,102}, {0,102,153}, {51,153,204}, {255,255,255} }, // Ocean
    { {51,0,51}, {102,0,51}, {255,51,51}, {255,153,51}, {255,255,102} }, // Sunset
    { {0,0,0}, {34,0,68}, {68,0,136}, {136,0,255}, {255,255,255} }, // Galaxy
    { {0,0,0}, {17,51,0}, {51,102,0}, {102,204,0}, {204,255,0} }, // Toxic
    { {255,204,136}, {221,170,221}, {170,136,102}, {136,68,34}, {68,34,17} }, // Desert
    { {255,204,204}, {255,153,204}, {255,102,204}, {255,51,204}, {255,255,255} }, // Candy
    { {0,0,0}, {51,51,51}, {136,136,136}, {204,204,204}, {255,255,255} }, // Mono
    { {217,91,67}, {192,41,66}, {84,36,55}, {83,119,122}, {236,208,120} }, // Retro
    { {0,0,0}, {0,51,0}, {0,102,0}, {0,255,0}, {204,255,204} }, // Matrix
    { {255,113,206}, {1,205,254}, {5,255,161}, {185,103,255}, {255,251,150} }, // Vapor
    { {34,17,0}, {102,68,0}, {170,136,0}, {221,204,0}, {255,255,170} }, // Gold
    { {17,0,0}, {68,0,0}, {136,0,0}, {204,0,0}, {255,0,0} }, // Blood
    { {0,0,0}, {0,34,17}, {0,102,68}, {0,255,136}, {170,255,204} }, // Aurora
    { {0,0,0}, {8,8,24}, {16,16,48}, {48,48,96}, {255,255,255} }, // Space
    { {0,0,0}, {34,0,0}, {170,0,0}, {255,68,0}, {255,255,0} }, // Lava
    { {0,0,34}, {0,0,136}, {0,68,255}, {0,170,255}, {255,255,255} }, // Electric
    { {255,179,186}, {255,223,186}, {255,255,186}, {186,255,201}, {186,225,255} }, // Pastel
    { {10,10,42}, {26,26,74}, {42,42,106}, {74,74,138}, {170,170,170} }, // Midnight
    { {255,0,0}, {0,255,0}, {0,0,255}, {255,255,0}, {255,0,255} }, // Neon
    { {255,0,255}, {0,255,0}, {255,0,0}, {0,0,255}, {255,255,0} }, // Kaotic
    { {20,0,0}, {80,0,0}, {150,0,0}, {220,0,0}, {255,100,100} }, // Fade Red
    { {0,20,0}, {0,80,0}, {0,150,0}, {0,220,0}, {100,255,100} }, // Fade Green
    { {0,0,20}, {0,0,80}, {0,0,150}, {0,0,220}, {100,100,255} }, // Fade Blue
    { {0,0,0}, {10,10,10}, {25,25,25}, {10,10,10}, {0,0,0} }, // Void
    { {200,200,255}, {220,220,255}, {240,240,255}, {255,255,255}, {200,200,255} }, // Ghost
    { {50,0,20}, {150,0,60}, {255,0,100}, {255,100,150}, {255,200,220} }, // Hot Pink
    { {0,0,0}, {100,255,0}, {200,0,255}, {0,255,200}, {255,255,255} }, // Acid
    { {100,30,0}, {150,60,0}, {200,100,0}, {255,150,0}, {255,200,50} }, // Autumn
    { {255,0,0}, {0,255,255}, {0,255,0}, {255,0,255}, {0,0,255} }, // Glitch
    { {255,0,128}, {128,255,0}, {0,128,255}, {255,255,0}, {0,255,255} }, // LSD
    { {255,255,255}, {255,0,0}, {255,255,0}, {0,255,0}, {0,255,255} }, // Flashy
    { {200,0,255}, {0,255,100}, {255,100,0}, {0,100,255}, {255,200,0} }, // Psychedelic
    { {0,255,50}, {255,0,150}, {50,0,255}, {255,255,0}, {255,0,0} }, // Rave
    { {255,0,0}, {255,127,0}, {255,255,0}, {0,255,0}, {0,0,255} } // Rainbow
};

SuperArtEngine::SuperArtEngine(int width, int height) : width(width), height(height), time(0), frameCounter(0) {
    initLUT();
    initPalettes();
}

SuperArtEngine::~SuperArtEngine() {}

void SuperArtEngine::init() {
    time = 0;
    frameCounter = 0;
    lastSwitchTime = millis();
}

void SuperArtEngine::loadConfig(const ArtConfig& config) {
    currentAnim = config;
    resetState();
}

void SuperArtEngine::initLUT() {
    for (int i = 0; i < LUT_SIZE; i++) {
        float angle = ((float)i / LUT_SIZE) * M_PI * 2.0f;
        sinLUT[i] = sin(angle);
        cosLUT[i] = cos(angle);
    }
}

void SuperArtEngine::initPalettes() {
    for (int p = 0; p < 40; p++) {
        for (int i = 0; i < 256; i++) {
            float t = i / 255.0f;
            float segment = t * 4;
            int idx1 = floor(segment);
            int idx2 = idx1 + 1 > 4 ? 4 : idx1 + 1;
            float localT = segment - idx1;
            rgb_t c1 = basePalettes[p][idx1];
            rgb_t c2 = basePalettes[p][idx2];
            
            palettes[p].colors[i].r = c1.r + (c2.r - c1.r) * localT;
            palettes[p].colors[i].g = c1.g + (c2.g - c1.g) * localT;
            palettes[p].colors[i].b = c1.b + (c2.b - c1.b) * localT;
        }
    }
}

float SuperArtEngine::fastSin(float x) {
    float t = fmod(x, M_PI * 2.0f);
    if (t < 0) t += M_PI * 2.0f;
    const float LUT_SCALE = LUT_SIZE / (M_PI * 2.0f);
    int index = (int)(t * LUT_SCALE);
    if (index >= LUT_SIZE) index = 0;
    return sinLUT[index];
}

float SuperArtEngine::fastCos(float x) {
    float t = fmod(x, M_PI * 2.0f);
    if (t < 0) t += M_PI * 2.0f;
    const float LUT_SCALE = LUT_SIZE / (M_PI * 2.0f);
    int index = (int)(t * LUT_SCALE);
    if (index >= LUT_SIZE) index = 0;
    return cosLUT[index];
}

float SuperArtEngine::fastHypot(float dx, float dy) {
    float ax = fabs(dx);
    float ay = fabs(dy);
    if (ax > ay) {
        return ax + 0.4f * ay;
    } else {
        return ay + 0.4f * ax;
    }
}

rgb_t SuperArtEngine::getColor(float index) {
    // Decouple color timing from the animation's specific 'time' modifier and tie it to global time
    float finalIndex = index + (millis() * 0.001f * currentAnim.colorSpeed * 50.0f);
    int safeIdx = ((int)fabs(finalIndex)) % 256;
    int safePalette = currentAnim.palette % 40;
    if (safePalette < 0) safePalette = 0;
    return palettes[safePalette].colors[safeIdx];
}

void SuperArtEngine::resetState() {
    particles.clear();
    grid.clear();
    pegs.clear();
    time = 0;
    frameCounter = 0;
    lastSwitchTime = millis();
    lSystemString = "F";
    
    int algo = currentAnim.algo;
    int density = fmax(10, currentAnim.density);
    
    if (algo == 2 || algo == 5 || algo == 7 || algo == 11 || algo == 13 || algo == 16 || algo == 20 || algo == 21 || algo == 22 || algo == 24) {
        for (int i = 0; i < density; i++) {
            Particle p;
            p.x = random(0, V_WIDTH * 100) / 100.0f;
            p.y = random(0, V_HEIGHT * 100) / 100.0f;
            p.vx = (random(0, 200) / 100.0f - 1.0f) * 2.0f;
            p.vy = (random(0, 200) / 100.0f - 1.0f) * 2.0f;
            p.life = random(0, 100);
            p.colorIdx = random(0, 256);
            particles.push_back(p);
        }
    } else if (algo == 0 || algo == 6 || algo == 10 || algo == 12 || algo == 14 || algo == 15 || algo == 18 || algo == 19 || algo == 23) {
        int cols = fmax(4, sqrt(density));
        int rows = cols;
        grid.resize(cols);
        for (int i = 0; i < cols; i++) {
            grid[i].resize(rows);
            for (int j = 0; j < rows; j++) {
                grid[i][j] = random(0, 100) > 50 ? 1 : 0;
            }
        }
    } else if (algo == 4) {
        generateLSystem(fmax(1, ((int)currentAnim.mathC % 4) + 2));
    } else if (algo == 17) {
        int cols = fmax(3, sqrt(density / 10));
        for (int i = 1; i <= cols; i++) {
            for (int j = 1; j <= cols; j++) {
                Peg p;
                p.x = ((float)i / (cols + 1)) * V_WIDTH;
                p.y = ((float)j / (cols + 1)) * V_HEIGHT;
                p.active = random(0, 100) > 30;
                pegs.push_back(p);
            }
        }
    }
}

void SuperArtEngine::generateLSystem(int iters) {
    String axiom = "F";
    String rule = "F[+F]F[-F]F";
    for (int i = 0; i < iters; i++) {
        String next = "";
        for (int j = 0; j < axiom.length(); j++) {
            if (axiom[j] == 'F') next += rule;
            else next += axiom[j];
        }
        axiom = next;
    }
    lSystemString = axiom;
}

void SuperArtEngine::update() {
    // Treat 'speed' correctly across all animations irrespective of BPM (mimicking JS logic)
    float timeMod = currentAnim.speed * 0.025f;
    time += timeMod;
    frameCounter++;
    
    int algo = currentAnim.algo;
    if (algo == 0) updateGameOfLife();
    else if (algo == 2) updateSubstrate();
    else if (algo == 5) updateFlowFields();
    else if (algo == 7) updateNeuralMorph();
    else if (algo == 11) { /* purely drawn */ }
    else if (algo == 16) updateFlowFields();
    else if (algo == 20 || algo == 21 || algo == 22) updateNeuralMorph();
    else if (algo == 24) updateAIBoids();
}

void SuperArtEngine::updateGameOfLife() {
    int speedMod = fmax(1, 10 / currentAnim.speed);
    if (frameCounter % speedMod != 0) return;
    int cols = grid.size();
    if (cols == 0) return;
    int rows = grid[0].size();
    
    std::vector<std::vector<int>> next(cols, std::vector<int>(rows, 0));
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            int state = grid[i][j];
            int neighbors = 0;
            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    neighbors += grid[(i + x + cols) % cols][(j + y + rows) % rows];
                }
            }
            neighbors -= state;
            bool mutation = (random(0, 1000) / 1000.0f) < (currentAnim.mathA * 0.05f + currentAnim.chaos * 0.01f);
            if (state == 0 && (neighbors == 3 || mutation)) next[i][j] = 1;
            else if (state == 1 && (neighbors < 2 || neighbors > 3)) next[i][j] = 0;
            else next[i][j] = state;
        }
    }
    grid = next;
}

void SuperArtEngine::updateSubstrate() {
    float a = currentAnim.mathA;
    float b = currentAnim.mathB;
    float seedChance = 0.01f * currentAnim.mathD + (currentAnim.chaos * 0.005f);
    for (auto& p : particles) {
        float angle = fastSin(p.x * a) * fastCos(p.y * b) * M_PI * 2.0f + time;
        p.vx = fastCos(angle) * currentAnim.speed * currentAnim.mathC + ((random(0,100)/100.0f)*currentAnim.chaos - currentAnim.chaos/2.0f)*0.1f;
        p.vy = fastSin(angle) * currentAnim.speed * currentAnim.mathE + ((random(0,100)/100.0f)*currentAnim.chaos - currentAnim.chaos/2.0f)*0.1f;
        p.x += p.vx;
        p.y += p.vy;
        if ((random(0, 1000) / 1000.0f) < seedChance) {
            p.x = random(0, V_WIDTH * 100) / 100.0f;
            p.y = random(0, V_HEIGHT * 100) / 100.0f;
        }
    }
}

void SuperArtEngine::updateFlowFields() {
    float c = currentAnim.complexity;
    float a = currentAnim.mathA;
    float b = currentAnim.mathB;
    float chaos = currentAnim.chaos;
    int pt = currentAnim.particleType % 3;
    
    for (auto& p : particles) {
        float angle = fastSin(p.x * 0.01f * a * c) * fastCos(p.y * 0.01f * b * c) * M_PI * 2.0f + time * currentAnim.mathC;
        if (pt == 0) {
            p.vx = fastCos(angle) * currentAnim.speed + ((random(0,100)/100.0f)*chaos - chaos/2.0f)*0.2f;
            p.vy = fastSin(angle) * currentAnim.speed + ((random(0,100)/100.0f)*chaos - chaos/2.0f)*0.2f;
        } else if (pt == 1) {
            p.vx += ((random(0,100)/100.0f)-0.5f) * chaos * 0.5f;
            p.vy += ((random(0,100)/100.0f)-0.5f) * chaos * 0.5f;
            p.vx *= 0.95f; p.vy *= 0.95f;
        } else if (pt == 2) {
            p.vy += 0.1f * currentAnim.speed;
            p.vx += ((random(0,100)/100.0f)-0.5f) * chaos * 0.1f;
            if (p.y > V_HEIGHT) { p.vy *= -0.8f; p.y = V_HEIGHT; }
        }
        p.x += p.vx; p.y += p.vy;
        if (p.x < 0) p.x += V_WIDTH;
        if (p.x > V_WIDTH) p.x -= V_WIDTH;
        if (p.y < 0) p.y += V_HEIGHT;
        if (p.y > V_HEIGHT) p.y -= V_HEIGHT;
    }
}

void SuperArtEngine::updateAIBoids() {
    float chaos = currentAnim.chaos;
    float speed = currentAnim.speed;
    float sepDist = 20 * currentAnim.mathA;
    float alignDist = 40 * currentAnim.mathB;
    float cohDist = 40 * currentAnim.mathC;
    
    for (size_t i = 0; i < particles.size(); i++) {
        auto& p = particles[i];
        float sepX = 0, sepY = 0;
        float aliX = 0, aliY = 0, aliCount = 0;
        float cohX = 0, cohY = 0, cohCount = 0;
        
        for (size_t j = 0; j < particles.size(); j++) {
            if (i == j) continue;
            auto& other = particles[j];
            float d = fastHypot(p.x - other.x, p.y - other.y);
            
            if (d < sepDist && d > 0) {
                sepX += (p.x - other.x) / d; sepY += (p.y - other.y) / d;
            }
            if (d < alignDist) {
                aliX += other.vx; aliY += other.vy; aliCount++;
            }
            if (d < cohDist) {
                cohX += other.x; cohY += other.y; cohCount++;
            }
        }
        
        p.vx += sepX * 0.1f; p.vy += sepY * 0.1f;
        if (aliCount > 0) {
            p.vx += ((aliX / aliCount) - p.vx) * 0.05f;
            p.vy += ((aliY / aliCount) - p.vy) * 0.05f;
        }
        if (cohCount > 0) {
            p.vx += (((cohX / cohCount) - p.x) * 0.01f);
            p.vy += (((cohY / cohCount) - p.y) * 0.01f);
        }
        
        p.vx += ((random(0,100)/100.0f)-0.5f) * chaos * 0.2f;
        p.vy += ((random(0,100)/100.0f)-0.5f) * chaos * 0.2f;
        
        float mag = fastHypot(p.vx, p.vy) + 0.01f;
        if (mag > speed * 2) {
            p.vx = (p.vx / mag) * speed * 2;
            p.vy = (p.vy / mag) * speed * 2;
        }
        
        p.x += p.vx; p.y += p.vy;
        if (p.x < 0) p.x += V_WIDTH;
        if (p.x > V_WIDTH) p.x -= V_WIDTH;
        if (p.y < 0) p.y += V_HEIGHT;
        if (p.y > V_HEIGHT) p.y -= V_HEIGHT;
    }
}

void SuperArtEngine::updateNeuralMorph() { updateFlowFields(); }

void SuperArtEngine::drawShape(float x, float y, float size, bool fill, float alpha, rgb_t c) {
    int shapeID = currentAnim.shape;
    int cat = shapeID % 10;
    int mod = shapeID / 10;
    float s = size * (currentAnim.mathD * 0.2f + 0.8f);

    // Perfect 1:4 aspect scale resolution mapping
    // Sub-pixel or exact 1 pixel fallback for fine dust (below 1 scaled pixel)
    if (s <= 0.5f) {
        hub75_set_pixel(x, y, c);
        return;
    }

    int radius = (int)(s + 0.5f);
    if (radius < 1) radius = 1;

    if (fill) {
        if (cat == 0 || cat == 3 || cat == 4 || cat == 7) {
            if (radius == 1) {
                // True 1x1 pixel mapping instead of forcing 2x2 blocks for thin points
                hub75_set_pixel(x, y, c);
            } else {
                hub75_fill_circle(x, y, radius, c);
            }
        } else if (cat == 1) {
            if (radius == 1) {
                hub75_set_pixel(x, y, c);
            } else {
                hub75_fill_rect(x - radius, y - radius, radius * 2, radius * (mod > 5 ? 1 : 2), c);
            }
        } else if (cat == 2) {
            drawThinLine(x, y - radius, x + radius, y + radius, c);
            drawThinLine(x + radius, y + radius, x - radius, y + radius, c);
            drawThinLine(x - radius, y + radius, x, y - radius, c);
        } else if (cat == 5) {
            if (radius == 1) hub75_set_pixel(x, y, c);
            else {
                hub75_draw_circle(x, y, radius, c);
                hub75_draw_circle(x, y, radius/2, c);
            }
        } else if (cat == 6) {
            drawThinLine(x - radius, y, x + radius, y, c);
            drawThinLine(x, y - radius, x, y + radius, c);
        } else {
            if (radius == 1) hub75_set_pixel(x, y, c);
            else hub75_fill_circle(x, y, radius, c);
        }
    } else {
        if (cat == 0 || cat == 3 || cat == 4 || cat == 7) {
            if (radius == 1) hub75_set_pixel(x, y, c);
            else hub75_draw_circle(x, y, radius, c);
        } else if (cat == 1) {
            if (radius == 1) hub75_set_pixel(x, y, c);
            else hub75_draw_rect(x - radius, y - radius, radius * 2, radius * (mod > 5 ? 1 : 2), c);
        } else if (cat == 2) {
            drawThinLine(x, y - radius, x + radius, y + radius, c);
            drawThinLine(x + radius, y + radius, x - radius, y + radius, c);
            drawThinLine(x - radius, y + radius, x, y - radius, c);
        } else if (cat == 5) {
            if (radius == 1) hub75_set_pixel(x, y, c);
            else {
                hub75_draw_circle(x, y, radius, c);
                if (radius > 1) hub75_draw_circle(x, y, radius/2, c);
            }
        } else if (cat == 6) {
            drawThinLine(x - radius, y, x + radius, y, c);
            drawThinLine(x, y - radius, x, y + radius, c);
        } else {
            if (radius == 1) hub75_set_pixel(x, y, c);
            else hub75_draw_circle(x, y, radius, c);
        }
    }
}

void SuperArtEngine::draw() {
    int algo = currentAnim.algo;
    float fade = currentAnim.trailFade;
    
    bool clearsFrame = (algo == 1 || algo == 3 || algo == 6 || algo == 8 || algo == 9 || algo == 10 || algo == 12 || algo == 13 || algo == 14 || algo == 15 || algo == 17 || algo == 18 || algo == 19 || algo == 23);
    if (clearsFrame) fade = 1.0f;
    
    if (fade == 1.0f) {
        hub75_clear();
    }
    
    drawAlgorithms(algo);

    // --- ZERO-RAM SYMMETRY POST-PROCESSING ---
    // Instead of using a heavy off-screen buffer, we copy the pixels 
    // from the primary quadrants directly on the hardware buffer!
    int sym = currentAnim.symmetry;
    
    if (sym == 1) { // Horizontal
        for (int y = 0; y < height / 2; y++) {
            for (int x = 0; x < width; x++) {
                rgb_t c = hub75_get_pixel(x, y);
                hub75_set_pixel(x, height - 1 - y, c);
            }
        }
    } 
    else if (sym == 2) { // Vertical
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width / 2; x++) {
                rgb_t c = hub75_get_pixel(x, y);
                hub75_set_pixel(width - 1 - x, y, c);
            }
        }
    } 
    else if (sym == 3) { // Quad
        for (int y = 0; y < height / 2; y++) {
            for (int x = 0; x < width / 2; x++) {
                rgb_t c = hub75_get_pixel(x, y);
                hub75_set_pixel(width - 1 - x, y, c);                 // Top Right
                hub75_set_pixel(x, height - 1 - y, c);                // Bottom Left
                hub75_set_pixel(width - 1 - x, height - 1 - y, c);    // Bottom Right
            }
        }
    } 
    else if (sym == 4) { // Kaleidoscope (8 slices)
        // 1. Diagonal mirror inside the top-left quadrant
        for (int y = 0; y < height / 2; y++) {
            for (int x = 0; x <= y; x++) {
                rgb_t c = hub75_get_pixel(x, y);
                hub75_set_pixel(y, x, c); 
            }
        }
        // 2. Quad mirror the result
        for (int y = 0; y < height / 2; y++) {
            for (int x = 0; x < width / 2; x++) {
                rgb_t c = hub75_get_pixel(x, y);
                hub75_set_pixel(width - 1 - x, y, c);                 
                hub75_set_pixel(x, height - 1 - y, c);                
                hub75_set_pixel(width - 1 - x, height - 1 - y, c);    
            }
        }
    }
}

void SuperArtEngine::drawAlgorithms(int algo) {
    if (algo == 0) drawGameOfLife();
    else if (algo == 1) drawMandelbrot();
    else if (algo == 2) drawSubstrate();
    else if (algo == 3) drawFractalFlame();
    else if (algo == 4) drawLSystem();
    else if (algo == 5) drawParticles();
    else if (algo == 6) drawPlasma();
    else if (algo == 7) drawNeuralMorph();
    else if (algo == 8) drawPlotter();
    else if (algo == 9) drawLEDPulse();
    else if (algo == 10) drawGeometric();
    else if (algo == 11) drawSquiggle();
    else if (algo == 12) drawMolnar();
    else if (algo == 13) drawNake();
    else if (algo == 14) drawNees();
    else if (algo == 15) drawLeWitt();
    else if (algo == 16) drawFidenza();
    else if (algo == 17) drawRingers();
    else if (algo == 18) drawAutoglyphs();
    else if (algo == 19) drawArchetype();
    else if (algo == 20) drawPassersby();
    else if (algo == 21) drawAnadol();
    else if (algo == 22) drawLearningToSee();
    else if (algo == 23) drawAvidLines();
    else if (algo == 24) drawParticles();
}

void SuperArtEngine::drawGameOfLife() {
    int cols = grid.size();
    if (cols == 0) return;
    int rows = grid[0].size();
    
    float cellW = (float)width / cols;
    float cellH = (float)height / rows;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            if (grid[i][j] == 1) {
                rgb_t c = getColor((i / (float)cols + j / (float)rows) * 128.0f);
                float cx = i * cellW + cellW / 2.0f;
                float cy = j * cellH + cellH / 2.0f;
                drawShape(cx, cy, cellW / 2.0f, true, 1.0f, c);
            }
        }
    }
}

void SuperArtEngine::drawMandelbrot() {
    float scaleX = (float)width / V_WIDTH;
    int maxIter = fmax(10, currentAnim.mathC);
    float zoom = 1.0f + fastSin(time * 0.1f) * 0.5f + currentAnim.mathB;
    float moveX = fastCos(time * 0.2f) * 0.5f + currentAnim.mathD - 5.0f;
    float moveY = fastSin(time * 0.2f) * 0.5f + currentAnim.mathE - 5.0f;
    int res = fmax(1.0f, 4.0f * scaleX);
    float chaos = currentAnim.chaos * 0.1f;
    for (int x = 0; x < width; x += res) {
        for (int y = 0; y < height; y += res) {
            float pr = 1.5f * (x - width / 2.0f) / (0.5f * zoom * width) + moveX + ((random(0,100)/100.0f)*chaos);
            float pi = (y - height / 2.0f) / (0.5f * zoom * height) + moveY + ((random(0,100)/100.0f)*chaos);
            float newRe = 0, newIm = 0, oldRe = 0, oldIm = 0;
            int i;
            for (i = 0; i < maxIter; i++) {
                oldRe = newRe; oldIm = newIm;
                newRe = oldRe * oldRe - oldIm * oldIm + pr + currentAnim.mathA;
                newIm = 2 * oldRe * oldIm + pi;
                if ((newRe * newRe + newIm * newIm) > 4) break;
            }
            if (i < maxIter) {
                rgb_t c = getColor(i * 255.0f / maxIter);
                hub75_fill_rect(x, y, res, res, c);
            }
        }
    }
}

void SuperArtEngine::drawSubstrate() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    
    for (auto& p : particles) {
        rgb_t c = getColor(p.colorIdx);
        hub75_fill_rect(p.x * scaleX, p.y * scaleY, fmax(1.0f, 2*scaleX), fmax(1.0f, 2*scaleY), c);
    }
    for (size_t i = 0; i < particles.size(); i += 10) {
        auto& p1 = particles[i];
        auto& p2 = particles[(i + 1) % particles.size()];
        float dist = fastHypot(p1.x - p2.x, p1.y - p2.y);
        if (dist < 30 * currentAnim.mathC) {
            rgb_t c = getColor(p1.colorIdx);
            drawThinLine(p1.x * scaleX, p1.y * scaleY, p2.x * scaleX, p2.y * scaleY, c);
        }
    }
}

void SuperArtEngine::drawFractalFlame() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    int iters = floor(1000 * currentAnim.density / 100);
    float x = 0, y = 0;
    float a = currentAnim.mathA; float b = currentAnim.mathB; float mC = currentAnim.mathC;
    float shift = currentAnim.mathD * 20;
    float chaos = currentAnim.chaos * 0.05f;
    for (int i = 0; i < iters; i++) {
        float r = random(0, 100) / 100.0f;
        float nx, ny;
        if (r < 0.33f) {
            nx = fastSin(x*a) - fastCos(y*b) + ((random(0,100)/100.0f)*chaos - chaos/2.0f);
            ny = fastSin(y*a) - fastCos(x*b) + ((random(0,100)/100.0f)*chaos - chaos/2.0f);
        } else if (r < 0.66f) {
            nx = x*fastCos(time) - y*fastSin(time) + mC;
            ny = x*fastSin(time) + y*fastCos(time) - mC;
        } else {
            nx = x*a - y*a; ny = x*b + y*b;
        }
        x = nx; y = ny;
        float px = width/2.0f + x * 20 * scaleX * currentAnim.complexity;
        float py = height/2.0f + y * 20 * scaleY * currentAnim.complexity;
        rgb_t c = getColor(fmod(i, 256) + shift);
        hub75_fill_rect(px, py, 1, 1, c);
    }
}

void SuperArtEngine::drawLSystem() {
    float scaleY = (float)height / V_HEIGHT;
    float len = 5 * currentAnim.mathB * scaleY;
    int cIdx = 0;
    float cx = width / 2.0f;
    float cy = height;
    float currentAngle = -M_PI / 2.0f; 
    float angleStep = currentAnim.mathA * M_PI + fastSin(time*0.5f)*currentAnim.mathE + (currentAnim.chaos*0.01f);
    
    std::vector<std::pair<float, std::pair<float, float>>> stack;
    
    for (size_t i = 0; i < lSystemString.length(); i++) {
        char c = lSystemString[i];
        if (c == 'F') {
            rgb_t col = getColor(cIdx++);
            float nx = cx + cos(currentAngle) * len;
            float ny = cy + sin(currentAngle) * len;
            drawThinLine(cx, cy, nx, ny, col);
            cx = nx; cy = ny;
        } else if (c == '+') {
            currentAngle += angleStep;
        } else if (c == '-') {
            currentAngle -= angleStep;
        } else if (c == '[') {
            stack.push_back({currentAngle, {cx, cy}});
        } else if (c == ']') {
            if (!stack.empty()) {
                auto top = stack.back();
                stack.pop_back();
                currentAngle = top.first;
                cx = top.second.first;
                cy = top.second.second;
            }
        }
    }
}

void SuperArtEngine::drawParticles() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float scale = fmin(scaleX, scaleY);
    for (auto& p : particles) {
        rgb_t c = getColor(p.colorIdx);
        // Removing fmax() allows tiny particles to scale below 1 and trigger the single-pixel logic
        drawShape(p.x * scaleX, p.y * scaleY, 2.0f * scale, true, 1.0f, c);
    }
}

void SuperArtEngine::drawPlasma() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    int gridX = fmax(1, floor(4 * currentAnim.mathD));
    int gridY = fmax(1, floor(4 * currentAnim.mathE));
    float chaos = currentAnim.chaos * 0.1f;
    for (int y = 0; y < V_HEIGHT; y += gridY) {
        for (int x = 0; x < V_WIDTH; x += gridX) {
            float v = fastSin(x*0.01f*a + time) + fastSin(y*0.01f*b + time) + fastSin((x+y)*0.01f*c) + ((random(0,100)/100.0f)*chaos);
            rgb_t col = getColor((v + 3.0f) / 6.0f * 255.0f);
            hub75_fill_rect(x * scaleX, y * scaleY, fmax(1.0f, gridX * scaleX), fmax(1.0f, gridY * scaleY), col);
        }
    }
}

void SuperArtEngine::drawNeuralMorph() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float scale = fmin(scaleX, scaleY);
    float a = currentAnim.mathA; float b = currentAnim.mathB;
    
    for (auto& p : particles) {
        rgb_t c = getColor(p.colorIdx);
        float s = fmax(0.5f, fabs(fastSin(time + p.x)*5*currentAnim.mathE) + currentAnim.mathD) * scale;
        hub75_fill_circle(p.x * scaleX, p.y * scaleY, s, c);
    }
    
    float maxDist = 50 * a * b; 
    for (size_t i = 0; i < particles.size(); i++) {
        for (size_t j = i + 1; j < particles.size(); j++) {
            auto& p1 = particles[i]; auto& p2 = particles[j];
            float d = fastHypot(p1.x - p2.x, p1.y - p2.y);
            if (d < maxDist) {
                rgb_t c = getColor(p1.colorIdx);
                drawThinLine(p1.x * scaleX, p1.y * scaleY, p2.x * scaleX, p2.y * scaleY, c);
            }
        }
    }
}

void SuperArtEngine::drawPlotter() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    int step = fmax(5, 40 - currentAnim.density / 25);
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    float thresh = currentAnim.mathD; float amp = currentAnim.mathE;
    
    for (int x = 0; x < V_WIDTH; x += step) {
        for (int y = 0; y < V_HEIGHT; y += step) {
            float noise = fastSin(x*a + y*b + time);
            if (noise > fastSin(c) * thresh) {
                rgb_t col = getColor(((float)x/V_WIDTH)*128.0f + ((float)y/V_HEIGHT)*128.0f);
                hub75_draw_rect(x * scaleX + noise*5*amp*scaleX, y * scaleY, fmax(1.0f, (step-2)*scaleX), fmax(1.0f, (step-2)*scaleY), col);
            }
        }
    }
}

void SuperArtEngine::drawLEDPulse() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float scale = fmin(scaleX, scaleY);
    float a = currentAnim.mathA;
    float b = currentAnim.mathB;
    float c = currentAnim.mathC;
    
    for (int x = 0; x < V_WIDTH; x += 10) {
        for (int y = 0; y < V_HEIGHT; y += 10) {
            float pulse = (fastSin(time * a + x * 0.05f * b + y * 0.05f * c) + 1.0f) / 2.0f;
            rgb_t color = getColor(pulse * 255.0f);
            
            float cx = x * scaleX + 5 * scaleX;
            float cy = y * scaleY + 5 * scaleY;
            float r = (pulse * 4.0f + 1.0f) * scale;
            
            color.r = (color.r * (int)(pulse*255)) >> 8;
            color.g = (color.g * (int)(pulse*255)) >> 8;
            color.b = (color.b * (int)(pulse*255)) >> 8;
            
            drawShape(cx, cy, r, true, pulse, color);
        }
    }
}

void SuperArtEngine::drawGeometric() {
    int cells = currentAnim.complexity > 5 ? 8 : 4;
    float cellSizeX = (float)width / cells;
    float cellSizeY = (float)height / cells;
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    float chaos = currentAnim.chaos * 0.5f;
    for (int cy = 0; cy < cells; cy++) {
        for (int cx = 0; cx < cells; cx++) {
            float n = fastSin(cx * a + cy * b + time * c) + ((random(0,100)/100.0f)*chaos);
            rgb_t col = getColor(fabs(n) * 255.0f);
            drawShape(cx*cellSizeX + cellSizeX/2.0f, cy*cellSizeY + cellSizeY/2.0f, fmax(1.0f, fmin(cellSizeX, cellSizeY)/2.0f), true, 1.0f, col);
        }
    }
}

void SuperArtEngine::drawSquiggle() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    float amp = currentAnim.mathD * 10;
    float freq = currentAnim.mathE;
    float chaos = currentAnim.chaos;
    
    for (int yOffset = 0; yOffset < V_HEIGHT; yOffset += 20) {
        rgb_t col = getColor((float)yOffset / V_HEIGHT * 255.0f);
        float px = 0, py = yOffset + fastSin(time * b) * (amp * c) + ((random(0,100)/100.0f)*chaos);
        for (int x = 10; x <= V_WIDTH; x += 10) {
            float y = yOffset + fastSin(x * 0.05f * a * freq + time * b) * (amp * c) + ((random(0,100)/100.0f)*chaos);
            drawThinLine(px * scaleX, py * scaleY, x * scaleX, y * scaleY, col);
            px = x; py = y;
        }
    }
}

void SuperArtEngine::drawMolnar() {
    int cols = fmax(2, floor(16 * currentAnim.mathA));
    float cellW = (float)width / cols; 
    float cellH = (float)height / cols;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < cols; j++) {
            rgb_t col = getColor((i*j)*10);
            float size = fmax(1.0f, cellW * 0.8f * fmax(0.1f, currentAnim.mathC));
            hub75_draw_rect(i*cellW + cellW/2.0f - size/2.0f, j*cellH + cellH/2.0f - size/2.0f, size, size, col);
        }
    }
}

void SuperArtEngine::drawNake() {
    float scale = fmin((float)width/V_WIDTH, (float)height/V_HEIGHT);
    float len = 50 * currentAnim.mathA * scale;
    float chaos = currentAnim.mathB + currentAnim.chaos * 0.1f;
    int num = floor(currentAnim.density / 2);
    for (int i = 0; i < num; i++) {
        float x1 = (fastSin(time + i) * 0.5f + 0.5f) * width;
        float y1 = (fastCos(time + i*chaos) * 0.5f + 0.5f) * height;
        rgb_t col = getColor(i % 256);
        drawShape(x1, y1, fmax(1.0f, len/4.0f), false, 1.0f, col);
    }
}

void SuperArtEngine::drawNees() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    int cols = 10;
    float cellW = (float)width / cols; 
    float cellH = (float)height / cols;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < cols; j++) {
            float dx = ((random(0,100)/100.0f)-0.5f) * j * currentAnim.mathA * (1+currentAnim.chaos*0.1f) * scaleX;
            float dy = ((random(0,100)/100.0f)-0.5f) * j * currentAnim.mathB * (1+currentAnim.chaos*0.1f) * scaleY;
            rgb_t col = getColor(j*25);
            drawShape(i*cellW + dx + cellW/2.0f, j*cellH + dy + cellH/2.0f, fmax(1.0f, (cellW-4.0f*scaleX)/2.0f * fmax(0.1f, currentAnim.mathC)), false, 1.0f, col);
        }
    }
}

void SuperArtEngine::drawLeWitt() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    std::vector<std::pair<float, float>> pts;
    float seed = currentAnim.mathC;
    for (int i = 0; i < 50; i++) {
        float x = (fastSin(i*seed + time*0.1f)*0.5f+0.5f) * width * currentAnim.mathD;
        float y = (fastCos(i*seed + time*0.1f)*0.5f+0.5f) * height * currentAnim.mathE;
        x += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 10 * scaleX;
        y += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 10 * scaleY;
        pts.push_back({x, y});
    }
    for (size_t i = 0; i < pts.size() - 1; i++) {
        rgb_t col = getColor(i * 5);
        drawThinLine(pts[i].first, pts[i].second, pts[i+1].first, pts[i+1].second, col);
    }
}

void SuperArtEngine::drawFidenza() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float scale = fmin(scaleX, scaleY);
    for (auto& p : particles) {
        rgb_t col = getColor(p.colorIdx);
        float baseW = currentAnim.mathC * 5 * scale;
        float varW = fastSin(p.x * 0.05f)*4 * currentAnim.mathD * scale;
        float w = baseW + varW;
        drawShape(p.x * scaleX, p.y * scaleY, w, true, 1.0f, col);
    }
}

void SuperArtEngine::drawRingers() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float scale = fmin(scaleX, scaleY);
    for (auto& p : pegs) {
        rgb_t col = getColor(0);
        hub75_fill_circle(p.x * scaleX, p.y * scaleY, fmax(1.0f, 4 * currentAnim.mathD * scale), col);
    }
    rgb_t col = getColor(128);
    bool first = true;
    float chaosOff = currentAnim.chaos * 2; 
    float px = 0, py = 0;
    for (auto& p : pegs) {
        if (p.active) {
            float ox = ((random(0,100)/100.0f)-0.5f) * chaosOff;
            float oy = ((random(0,100)/100.0f)-0.5f) * chaosOff;
            if (first) {
                px = p.x+ox; py = p.y+oy;
                first = false;
            } else {
                drawThinLine(px * scaleX, py * scaleY, (p.x+ox) * scaleX, (p.y+oy) * scaleY, col);
                px = p.x+ox; py = p.y+oy;
            }
        }
    }
}

void SuperArtEngine::drawAutoglyphs() {
    int cols = fmax(8, floor(16 * currentAnim.mathA));
    float cellW = (float)width / cols; 
    float cellH = (float)height / cols;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < cols; j++) {
            float v = fastSin(i * j * currentAnim.mathB);
            if ((random(0,100)/100.0f) < currentAnim.chaos * 0.05f) v *= -1;
            rgb_t col = getColor((i+j)*10);
            if (v > 0.5f * currentAnim.mathC) {
                drawThinLine(i*cellW, j*cellH, (i+1)*cellW, (j+1)*cellH, col);
            } else if (v < -0.5f * currentAnim.mathC) {
                drawThinLine((i+1)*cellW, j*cellH, i*cellW, (j+1)*cellH, col);
            } else {
                drawThinLine(i*cellW + cellW/2.0f, j*cellH, i*cellW + cellW/2.0f, (j+1)*cellH, col);
            }
        }
    }
}

void SuperArtEngine::drawArchetype() {
    float scaleX = (float)width / V_WIDTH;
    float divX = width * (0.3f + fastSin(time)*0.2f * currentAnim.mathA);
    float divY = height * (0.5f + fastCos(time)*0.3f);
    float pad = currentAnim.mathD * 10 * scaleX;
    float gap = currentAnim.mathE * 5 * scaleX;
    hub75_fill_rect(pad, pad, divX-gap, divY-gap, getColor(50));
    hub75_fill_rect(divX+gap, pad, width-divX-pad-gap, divY-gap, getColor(100));
    hub75_fill_rect(pad, divY+gap, divX-gap, height-divY-pad-gap, getColor(150));
    hub75_fill_rect(divX+gap, divY+gap, width-divX-pad-gap, height-divY-pad-gap, getColor(200));
}

void SuperArtEngine::drawPassersby() {
    drawNeuralMorph();
    float focus = currentAnim.mathB;
    rgb_t dark = {0,0,0};
    hub75_fill_circle(width/2.0f, height/2.0f, (width/2.5f) * focus, dark); 
}

void SuperArtEngine::drawAnadol() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float scale = fmin(scaleX, scaleY);
    for (auto& p : particles) {
        float rad = fmax(1.0f, (10 + fastSin(p.x * 0.05f + time)*5) * currentAnim.mathC * scale);
        rgb_t c = getColor(p.colorIdx);
        c.r = (c.r * (int)(0.3f * currentAnim.mathD * 255)) >> 8;
        c.g = (c.g * (int)(0.3f * currentAnim.mathD * 255)) >> 8;
        c.b = (c.b * (int)(0.3f * currentAnim.mathD * 255)) >> 8;
        drawShape(p.x * scaleX, p.y * scaleY, rad, true, 0.3f * currentAnim.mathD, c);
    }
}

void SuperArtEngine::drawLearningToSee() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    float scale = fmin(scaleX, scaleY);
    for (auto& p : particles) {
        float r = fmax(1.0f, fastSin(p.x*0.1f + p.y*0.1f + time) > (0.5f * currentAnim.mathD) ? 5 * scale : 1 * scale);
        rgb_t c = getColor(p.colorIdx);
        hub75_fill_rect(p.x * scaleX, p.y * scaleY, r, r, c);
    }
}

void SuperArtEngine::drawAvidLines() {
    float scaleX = (float)width / V_WIDTH;
    float scaleY = (float)height / V_HEIGHT;
    int lines = fmax(1, floor(10 * fmax(0.1f, currentAnim.mathA)));
    for (int l = 0; l < lines; l++) {
        rgb_t c = getColor(l * 20);
        float px = 0;
        float py = ((float)l/lines)*V_HEIGHT * currentAnim.mathD + fastSin(0 + l*0.5f + time)*20*currentAnim.mathB;
        py += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 2;
        for (int x = 5; x <= V_WIDTH; x += 5) {
            float y = ((float)l/lines)*V_HEIGHT * currentAnim.mathD + fastSin(x*0.02f + l*0.5f + time)*20*currentAnim.mathB;
            y += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 2;
            drawThinLine(px * scaleX, py * scaleY, x * scaleX, y * scaleY, c);
            px = x; py = y;
        }
    }
}

 