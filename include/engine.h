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

    std::vector<Particle> particles;
    std::vector<std::vector<int>> grid;
    std::vector<Peg> pegs;
    String lSystemString;

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
