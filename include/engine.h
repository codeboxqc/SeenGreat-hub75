#ifndef ENGINE_H
#define ENGINE_H

#include <Arduino.h>
#include "hub75_driver.h"
#include <vector>

struct ArtConfig {
    int id;
    int algo;
    int palette;
    int lightFx;
    int shape;
    int particleType;
    int symmetry;
    int bpm;
    int playDuration;
    float colorSpeed;
    float chaos;
    float speed;
    float density;
    float complexity;
    float trailFade;
    float mathA;
    float mathB;
    float mathC;
    float mathD;
    float mathE;
    float mathF;
};

struct Particle {
    float x, y, vx, vy, life;
    int colorIdx;
};

struct Peg {
    float x, y;
    bool active;
};

class SuperArtEngine {
public:
    SuperArtEngine(int width, int height);
    ~SuperArtEngine();

    void init();
    void loadConfig(const ArtConfig& config);
    void update();
    void draw();
    
    const ArtConfig& getCurrentAnim() const { return currentAnim; }

    // Mutate only the continuous math/physics parameters and refresh the
    // precomputed cache (pc).  Particles, time, and RNG state are untouched.
    void morphParams(const ArtConfig& target, float t);

private:
    int width;
    int height;
    float time;
    unsigned long frameCounter;
    unsigned long lastSwitchTime;
    
    ArtConfig currentAnim;
    
    static const int LUT_SIZE = 4096;
    float sinLUT[LUT_SIZE];
    float cosLUT[LUT_SIZE];

    struct Palette {
        rgb_t colors[256];
    };
    Palette palettes[40];

    // Fix 7: flat 1D grid — single pointer offset instead of two pointer dereferences,
    // much more cache-friendly for the tight GoL neighbour scan.
    static const int MAX_GRID_COLS = 32; // width/2 at min 2px/cell on 64px display
    static const int MAX_GRID_ROWS = 32;
    int grid[MAX_GRID_COLS * MAX_GRID_ROWS];
    int gridCols = 0;
    int gridRows = 0;

    std::vector<Peg> pegs;
    String lSystemString;

    // Fix 8: pre-baked L-system segment list — computed once in resetState,
    // re-used every frame. Eliminates per-frame trig and stack allocation in drawLSystem.
    struct LSegment { float x0, y0, x1, y1; int colorIdx; };
    static const int MAX_LSEG = 2048;
    LSegment lSystemSegments[MAX_LSEG];
    int lSystemSegCount = 0;

    static const int MAX_PARTICLES = 1000;
    Particle particles[MAX_PARTICLES];
    int particleCount = 0;

    // Vignette mask for drawPassersby — precomputed in resetState()
    static const int MAX_PIXELS = 64 * 64; // max display size
    bool vignetteOutside[MAX_PIXELS];

    // Per-config precomputed constants — derived once in resetState(), used every frame.
    // Avoids re-evaluating the same config-derived expressions in hot draw loops.
    struct PrecomputedConfig {
        // Scale factors (config-independent but cheap; stored for consistency)
        float scaleX, scaleY, scale; // width/V_WIDTH, height/V_HEIGHT, min(scaleX,scaleY)
        // getColor hot path
        int   safePalette;           // currentAnim.palette % 40
        float colorSpeedX50;         // colorSpeed * 50.0f  — multiplied by time each frame
        // drawShape hot path
        float mathD_shape;           // mathD * 0.2f + 0.8f  — size multiplier
        // NeuralMorph / Boids
        float maxDist;               // 50 * mathA * mathB
        float sepDist, alignDist, cohDist; // boid distances
        // Plasma
        float sinCThresh;            // fastSin(mathC) precomputed — constant in drawPlotter
        // Fidenza / Anadol
        float alphaAnadol;           // 0.3f * mathD * 255.0f  — Anadol alpha byte
        // Substrate line threshold
        float substrateLinkDist;     // 30 * mathC
        // Squiggle
        float squiggleAmp;           // mathD * 10
        // Ringers chaos offset
        float ringersChaos;          // chaos * 2
    } pc;
    void precompute(); // called at end of resetState()

    void initLUT();
    void initPalettes();
    
    float fastSin(float x);
    float fastCos(float x);
    float fastHypot(float dx, float dy);
    rgb_t getColor(float index);

    void resetState();
    void generateLSystem(int iters);

    void drawShape(float x, float y, float size, bool fill, float alpha, rgb_t c);
    void drawShapeInner(float s, int cat, int mod, bool fill);
    
    void updateGameOfLife();
    void updateSubstrate();
    void updateFlowFields();
    void updateAIBoids();
    void updateNeuralMorph();
    
    void drawAlgorithms(int algo);
    
    void drawGameOfLife();
    void drawMandelbrot();
    void drawSubstrate();
    void drawFractalFlame();
    void drawLSystem();
    void drawParticles();
    void drawPlasma();
    void drawNeuralMorph();
    void drawPlotter();
    void drawLEDPulse();
    void drawGeometric();
    void drawSquiggle();
    void drawMolnar();
    void drawNake();
    void drawNees();
    void drawLeWitt();
    void drawFidenza();
    void drawRingers();
    void drawAutoglyphs();
    void drawArchetype();
    void drawPassersby();
    void drawAnadol();
    void drawLearningToSee();
    void drawAvidLines();
};

#endif // ENGINE_H
