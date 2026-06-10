#include "engine.h"
#include <math.h>

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
    int index = floor((t / (M_PI * 2.0f)) * LUT_SIZE);
    if (index >= LUT_SIZE) index = 0;
    return sinLUT[index];
}

float SuperArtEngine::fastCos(float x) {
    float t = fmod(x, M_PI * 2.0f);
    if (t < 0) t += M_PI * 2.0f;
    int index = floor((t / (M_PI * 2.0f)) * LUT_SIZE);
    if (index >= LUT_SIZE) index = 0;
    return cosLUT[index];
}

float SuperArtEngine::fastHypot(float dx, float dy) {
    float ax = fabs(dx);
    float ay = fabs(dy);
    return fmax(ax, ay) + 0.4f * fmin(ax, ay);
}

rgb_t SuperArtEngine::getColor(float index) {
    float finalIndex = index + (time * currentAnim.colorSpeed * 50.0f);
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
            p.x = random(0, width * 100) / 100.0f;
            p.y = random(0, height * 100) / 100.0f;
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
                p.x = ((float)i / (cols + 1)) * width;
                p.y = ((float)j / (cols + 1)) * height;
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
    float timeMod = (currentAnim.bpm / 60.0f) * currentAnim.speed * 0.05f;
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
            p.x = random(0, width);
            p.y = random(0, height);
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
            if (p.y > height) { p.vy *= -0.8f; p.y = height; }
        }
        p.x += p.vx; p.y += p.vy;
        if (p.x < 0) p.x += width;
        if (p.x > width) p.x -= width;
        if (p.y < 0) p.y += height;
        if (p.y > height) p.y -= height;
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
        if (p.x < 0) p.x += width;
        if (p.x > width) p.x -= width;
        if (p.y < 0) p.y += height;
        if (p.y > height) p.y -= height;
    }
}

void SuperArtEngine::updateNeuralMorph() { updateFlowFields(); }

void SuperArtEngine::drawShape(float x, float y, float size, bool fill, float alpha) {
    int shapeID = currentAnim.shape;
    int cat = shapeID % 10;
    float s = fmax(0.5f, size * (currentAnim.mathD * 0.2f + 0.8f));
    rgb_t c = getColor(0); // Default color if not explicitly provided by algorithm

    if (fill) {
        if (cat == 0) {
            hub75_fill_circle(x, y, s, c);
        } else if (cat == 1) {
            hub75_fill_rect(x - s, y - s, s * 2, s * 2, c);
        } else {
            hub75_fill_circle(x, y, s, c);
        }
    } else {
        if (cat == 0) {
            hub75_draw_circle(x, y, s, c);
        } else if (cat == 1) {
            hub75_draw_rect(x - s, y - s, s * 2, s * 2, c);
        } else {
            hub75_draw_circle(x, y, s, c);
        }
    }
}

void SuperArtEngine::draw() {
    int algo = currentAnim.algo;
    float fade = currentAnim.trailFade;

    bool clearsFrame = (algo == 1 || algo == 3 || algo == 6 || algo == 8 || algo == 9 || algo == 10 || algo == 12 || algo == 13 || algo == 14 || algo == 15 || algo == 17 || algo == 18 || algo == 19 || algo == 23);
    if (clearsFrame) fade = 1.0f;

    // Simplification for fade: if 1.0, clear. Otherwise, we can simulate fade by drawing a black rect with alpha.
    // Since hub75 doesn't have native screen alpha fade easily, we can just clear if fade > 0.5 for now,
    // or rely on blending pixels... We will clear if fade == 1.0f.
    if (fade == 1.0f) {
        hub75_clear();
    }

    // Symmetry is hard without an offscreen buffer. We will just draw base algorithm for now.
    drawAlgorithms(algo);
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
                hub75_fill_circle(cx, cy, cellW / 2.0f, c);
            }
        }
    }
}

void SuperArtEngine::drawMandelbrot() {
    int maxIter = fmax(10, currentAnim.mathC);
    float zoom = 1.0f + fastSin(time * 0.1f) * 0.5f + currentAnim.mathB;
    float moveX = fastCos(time * 0.2f) * 0.5f + currentAnim.mathD - 5.0f;
    float moveY = fastSin(time * 0.2f) * 0.5f + currentAnim.mathE - 5.0f;
    int res = 4;
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
    for (auto& p : particles) {
        rgb_t c = getColor(p.colorIdx);
        hub75_fill_rect(p.x, p.y, 2, 2, c);
    }
    for (size_t i = 0; i < particles.size(); i += 10) {
        auto& p1 = particles[i];
        auto& p2 = particles[(i + 1) % particles.size()];
        float dist = fastHypot(p1.x - p2.x, p1.y - p2.y);
        if (dist < 30 * currentAnim.mathC) {
            rgb_t c = getColor(p1.colorIdx);
            hub75_draw_line(p1.x, p1.y, p2.x, p2.y, c);
        }
    }
}

void SuperArtEngine::drawFractalFlame() {
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
        float px = width/2 + x * 20 * currentAnim.complexity;
        float py = height/2 + y * 20 * currentAnim.complexity;
        rgb_t c = getColor(fmod(i, 256) + shift);
        hub75_fill_rect(px, py, 1, 1, c);
    }
}

void SuperArtEngine::drawLSystem() {
    // simplified straight drawing because we don't have turtle graphics easily
    // We will draw it mostly statically or just lines
    float len = 5 * currentAnim.mathB;
    int cIdx = 0;
    float cx = width / 2;
    float cy = height;
    float currentAngle = -M_PI / 2; // facing up
    float angleStep = currentAnim.mathA * M_PI + fastSin(time*0.5f)*currentAnim.mathE + (currentAnim.chaos*0.01f);

    std::vector<std::pair<float, std::pair<float, float>>> stack;

    for (size_t i = 0; i < lSystemString.length(); i++) {
        char c = lSystemString[i];
        if (c == 'F') {
            rgb_t col = getColor(cIdx++);
            float nx = cx + cos(currentAngle) * len;
            float ny = cy + sin(currentAngle) * len;
            hub75_draw_line(cx, cy, nx, ny, col);
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
    for (auto& p : particles) {
        rgb_t c = getColor(p.colorIdx);
        hub75_fill_circle(p.x, p.y, 2, c); // using circle as proxy
    }
}

void SuperArtEngine::drawPlasma() {
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    int gridX = fmax(1, floor(4 * currentAnim.mathD));
    int gridY = fmax(1, floor(4 * currentAnim.mathE));
    float chaos = currentAnim.chaos * 0.1f;
    for (int y = 0; y < height; y += gridY) {
        for (int x = 0; x < width; x += gridX) {
            float v = fastSin(x*0.01f*a + time) + fastSin(y*0.01f*b + time) + fastSin((x+y)*0.01f*c) + ((random(0,100)/100.0f)*chaos);
            rgb_t col = getColor((v + 3.0f) / 6.0f * 255.0f);
            hub75_fill_rect(x, y, gridX, gridY, col);
        }
    }
}

void SuperArtEngine::drawNeuralMorph() {
    float a = currentAnim.mathA; float b = currentAnim.mathB;
    for (auto& p : particles) {
        rgb_t c = getColor(p.colorIdx);
        float s = fmax(0.5f, fabs(fastSin(time + p.x)*5*currentAnim.mathE) + currentAnim.mathD);
        hub75_fill_circle(p.x, p.y, s, c);
    }
    float maxDist = 50 * a * b;
    for (size_t i = 0; i < particles.size(); i++) {
        for (size_t j = i + 1; j < particles.size(); j++) {
            auto& p1 = particles[i]; auto& p2 = particles[j];
            float d = fastHypot(p1.x - p2.x, p1.y - p2.y);
            if (d < maxDist) {
                rgb_t c = getColor(p1.colorIdx);
                // fake alpha by blending or just draw line
                hub75_draw_line(p1.x, p1.y, p2.x, p2.y, c);
            }
        }
    }
}

void SuperArtEngine::drawPlotter() {
    int step = fmax(5, 40 - currentAnim.density / 25);
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    float thresh = currentAnim.mathD; float amp = currentAnim.mathE;
    for (int x = 0; x < width; x += step) {
        for (int y = 0; y < height; y += step) {
            float noise = fastSin(x*a + y*b + time);
            if (noise > fastSin(c) * thresh) {
                rgb_t col = getColor(((float)x/width)*128.0f + ((float)y/height)*128.0f);
                hub75_draw_rect(x + noise*5*amp, y, fmax(1, step-2), fmax(1, step-2), col);
            }
        }
    }
}

void SuperArtEngine::drawLEDPulse() {
    float a = currentAnim.mathA;
    float b = currentAnim.mathB;
    float c = currentAnim.mathC;

    for (int x = 0; x < width; x += 10) {
        for (int y = 0; y < height; y += 10) {
            float pulse = (fastSin(time * a + x * 0.05f * b + y * 0.05f * c) + 1.0f) / 2.0f;
            rgb_t color = getColor(pulse * 255.0f);

            int cx = x + 5;
            int cy = y + 5;
            float r = pulse * 4.0f + 1.0f;

            // For hub75, we can't easily alpha-blend a circle shape without a custom blend circle.
            // We'll just draw the circle. Alpha effect is achieved by color darkness implicitly on LED matrix.
            color.r = (color.r * (int)(pulse*255)) >> 8;
            color.g = (color.g * (int)(pulse*255)) >> 8;
            color.b = (color.b * (int)(pulse*255)) >> 8;

            hub75_fill_circle(cx, cy, r, color);
        }
    }
}

void SuperArtEngine::drawGeometric() {
    int cells = currentAnim.complexity > 5 ? 8 : 4;
    float cellSize = width / cells;
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    float chaos = currentAnim.chaos * 0.5f;
    for (int cy = 0; cy < cells; cy++) {
        for (int cx = 0; cx < cells; cx++) {
            float n = fastSin(cx * a + cy * b + time * c) + ((random(0,100)/100.0f)*chaos);
            rgb_t col = getColor(fabs(n) * 255.0f);
            hub75_fill_circle(cx*cellSize + cellSize/2, cy*cellSize + cellSize/2, fmax(1, cellSize/2), col);
        }
    }
}

void SuperArtEngine::drawSquiggle() {
    float a = currentAnim.mathA; float b = currentAnim.mathB; float c = currentAnim.mathC;
    float amp = currentAnim.mathD * 10;
    float freq = currentAnim.mathE;
    float chaos = currentAnim.chaos;
    for (int yOffset = 0; yOffset < height; yOffset += 20) {
        rgb_t col = getColor((float)yOffset / height * 255.0f);
        float px = 0, py = yOffset + fastSin(time * b) * (amp * c) + ((random(0,100)/100.0f)*chaos);
        for (int x = 10; x <= width; x += 10) {
            float y = yOffset + fastSin(x * 0.05f * a * freq + time * b) * (amp * c) + ((random(0,100)/100.0f)*chaos);
            hub75_draw_line(px, py, x, y, col);
            px = x; py = y;
        }
    }
}

void SuperArtEngine::drawMolnar() {
    int cols = fmax(2, floor(16 * currentAnim.mathA));
    float cellW = width / cols; float cellH = height / cols;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < cols; j++) {
            rgb_t col = getColor((i*j)*10);
            float size = fmax(1, cellW * 0.8f * fmax(0.1f, currentAnim.mathC));
            // Just drawing rectangles since no rotation native primitives
            hub75_draw_rect(i*cellW + cellW/2 - size/2, j*cellH + cellH/2 - size/2, size, size, col);
        }
    }
}

void SuperArtEngine::drawNake() {
    float len = 50 * currentAnim.mathA;
    float chaos = currentAnim.mathB + currentAnim.chaos * 0.1f;
    int num = floor(currentAnim.density / 2);
    for (int i = 0; i < num; i++) {
        float x1 = (fastSin(time + i) * 0.5f + 0.5f) * width;
        float y1 = (fastCos(time + i*chaos) * 0.5f + 0.5f) * height;
        rgb_t col = getColor(i % 256);
        hub75_draw_circle(x1, y1, fmax(1, len/4), col);
    }
}

void SuperArtEngine::drawNees() {
    int cols = 10;
    float cellW = width / cols; float cellH = height / cols;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < cols; j++) {
            float dx = ((random(0,100)/100.0f)-0.5f) * j * currentAnim.mathA * (1+currentAnim.chaos*0.1f);
            float dy = ((random(0,100)/100.0f)-0.5f) * j * currentAnim.mathB * (1+currentAnim.chaos*0.1f);
            rgb_t col = getColor(j*25);
            hub75_draw_circle(i*cellW + dx + cellW/2, j*cellH + dy + cellH/2, fmax(1, (cellW-4)/2 * fmax(0.1f, currentAnim.mathC)), col);
        }
    }
}

void SuperArtEngine::drawLeWitt() {
    std::vector<std::pair<float, float>> pts;
    float seed = currentAnim.mathC;
    for (int i = 0; i < 50; i++) {
        float x = (fastSin(i*seed + time*0.1f)*0.5f+0.5f) * width * currentAnim.mathD;
        float y = (fastCos(i*seed + time*0.1f)*0.5f+0.5f) * height * currentAnim.mathE;
        x += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 10;
        y += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 10;
        pts.push_back({x, y});
    }
    for (size_t i = 0; i < pts.size() - 1; i++) {
        rgb_t col = getColor(i * 5);
        hub75_draw_line(pts[i].first, pts[i].second, pts[i+1].first, pts[i+1].second, col);
    }
}

void SuperArtEngine::drawFidenza() {
    for (auto& p : particles) {
        rgb_t col = getColor(p.colorIdx);
        float baseW = currentAnim.mathC * 5;
        float varW = fastSin(p.x * 0.05f)*4 * currentAnim.mathD;
        float w = fmax(1, baseW + varW);
        hub75_fill_circle(p.x, p.y, w, col);
    }
}

void SuperArtEngine::drawRingers() {
    for (auto& p : pegs) {
        rgb_t col = getColor(0);
        hub75_fill_circle(p.x, p.y, fmax(1, 4 * currentAnim.mathD), col);
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
                hub75_draw_line(px, py, p.x+ox, p.y+oy, col);
                px = p.x+ox; py = p.y+oy;
            }
        }
    }
}

void SuperArtEngine::drawAutoglyphs() {
    int cols = fmax(8, floor(16 * currentAnim.mathA));
    float cellW = width / cols; float cellH = height / cols;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < cols; j++) {
            float v = fastSin(i * j * currentAnim.mathB);
            if ((random(0,100)/100.0f) < currentAnim.chaos * 0.05f) v *= -1;
            rgb_t col = getColor((i+j)*10);
            if (v > 0.5f * currentAnim.mathC) {
                hub75_draw_line(i*cellW, j*cellH, (i+1)*cellW, (j+1)*cellH, col);
            } else if (v < -0.5f * currentAnim.mathC) {
                hub75_draw_line((i+1)*cellW, j*cellH, i*cellW, (j+1)*cellH, col);
            } else {
                hub75_draw_line(i*cellW + cellW/2, j*cellH, i*cellW + cellW/2, (j+1)*cellH, col);
            }
        }
    }
}

void SuperArtEngine::drawArchetype() {
    float divX = width * (0.3f + fastSin(time)*0.2f * currentAnim.mathA);
    float divY = height * (0.5f + fastCos(time)*0.3f);
    float pad = currentAnim.mathD * 10;
    float gap = currentAnim.mathE * 5;
    hub75_fill_rect(pad, pad, divX-gap, divY-gap, getColor(50));
    hub75_fill_rect(divX+gap, pad, width-divX-pad-gap, divY-gap, getColor(100));
    hub75_fill_rect(pad, divY+gap, divX-gap, height-divY-pad-gap, getColor(150));
    hub75_fill_rect(divX+gap, divY+gap, width-divX-pad-gap, height-divY-pad-gap, getColor(200));
}

void SuperArtEngine::drawPassersby() {
    drawNeuralMorph();
    // simulate shadow hole
    float focus = currentAnim.mathB;
    rgb_t dark = {0,0,0};
    hub75_fill_circle(width/2, height/2, (width/2.5f) * focus, dark); // fake alpha with black
}

void SuperArtEngine::drawAnadol() {
    for (auto& p : particles) {
        float rad = fmax(1, (10 + fastSin(p.x * 0.05f + time)*5) * currentAnim.mathC);
        rgb_t c = getColor(p.colorIdx);
        // fake alpha
        c.r = (c.r * (int)(0.3f * currentAnim.mathD * 255)) >> 8;
        c.g = (c.g * (int)(0.3f * currentAnim.mathD * 255)) >> 8;
        c.b = (c.b * (int)(0.3f * currentAnim.mathD * 255)) >> 8;
        hub75_fill_circle(p.x, p.y, rad, c);
    }
}

void SuperArtEngine::drawLearningToSee() {
    float a = currentAnim.mathA;
    for (auto& p : particles) {
        float r = fmax(1, fastSin(p.x*0.1f + p.y*0.1f + time) > (0.5f * currentAnim.mathD) ? 5 : 1);
        rgb_t c = getColor(p.colorIdx);
        hub75_fill_rect(p.x, p.y, r, r, c);
    }
}

void SuperArtEngine::drawAvidLines() {
    int lines = fmax(1, floor(10 * fmax(0.1f, currentAnim.mathA)));
    for (int l = 0; l < lines; l++) {
        rgb_t c = getColor(l * 20);
        float px = 0;
        float py = ((float)l/lines)*height * currentAnim.mathD + fastSin(0 + l*0.5f + time)*20*currentAnim.mathB;
        py += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 2;
        for (int x = 5; x <= width; x += 5) {
            float y = ((float)l/lines)*height * currentAnim.mathD + fastSin(x*0.02f + l*0.5f + time)*20*currentAnim.mathB;
            y += ((random(0,100)/100.0f)-0.5f) * currentAnim.chaos * 2;
            hub75_draw_line(px, py, x, y, c);
            px = x; py = y;
        }
    }
}
