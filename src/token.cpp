#include "token.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "engine.h"
#include "hub75_driver.h"
#include "config.h"

extern SuperArtEngine* engine;
extern unsigned long animationStartTime;
extern ArtConfig parseConfig(const char* jsonStr);

const char* defaultJsonConfig = R"=====(
{
    "id": 313,
    "name": "Hyper Matrix 313",
    "algo": 24,
    "palette": 4,
    "lightFx": 95,
    "shape": 50,
    "particleType": 69,
    "symmetry": 0,
    "colorSpeed": 0.6,
    "bpm": 182,
    "playDuration": 45,
    "chaos": 9.514,
    "speed": 2.653,
    "density": 10,
    "complexity": 2.761,
    "trailFade": 0.496,
    "mathA": 1.924,
    "mathB": 1.702,
    "mathC": 6.619,
    "mathD": 4.27,
    "mathE": 1.587,
    "mathF": 1.416
}
)=====";

int numTokens = 0;
String tokenFiles[256];

// ─── Morph transition state ─────────────────────────────────────────────────
bool isTransitioning = false;
unsigned long transitionStartTime = 0;
const unsigned long MORPH_DURATION_MS = 10000UL; // 10-second morph

ArtConfig morphFrom;   // snapshot of the token we are leaving
ArtConfig morphTo;     // full config of the token we are going to

// ─── Helpers ────────────────────────────────────────────────────────────────
static inline float lerpF(float a, float b, float t) {
    return a + (b - a) * t;
}

// Build a live interpolated config and push it to the engine.
// t == 0.0 => pure morphFrom, t == 1.0 => pure morphTo.
// Integer fields (algo, palette, lightFx, shape, particleType, symmetry, bpm)
// switch at t >= 0.5 so the visual "character" flips at the midpoint.
static void applyMorph(float t) {
    ArtConfig live = morphTo; // start from B so non-interpolated fields are correct

    // Switch integer/identity fields at the midpoint
    if (t < 0.5f) {
        live.id            = morphFrom.id;
        live.algo          = morphFrom.algo;
        live.palette       = morphFrom.palette;
        live.lightFx       = morphFrom.lightFx;
        live.shape         = morphFrom.shape;
        live.particleType  = morphFrom.particleType;
        live.symmetry      = morphFrom.symmetry;
        live.bpm           = morphFrom.bpm;
    }
    // playDuration stays as B so timing is correct after the transition

    // Smoothstep easing for a more organic feel
    float s = t * t * (3.0f - 2.0f * t);

    // Interpolate all continuous math/physics parameters
    live.colorSpeed   = lerpF(morphFrom.colorSpeed,  morphTo.colorSpeed,  s);
    live.chaos        = lerpF(morphFrom.chaos,        morphTo.chaos,       s);
    live.speed        = lerpF(morphFrom.speed,        morphTo.speed,       s);
    live.density      = lerpF(morphFrom.density,      morphTo.density,     s);
    live.complexity   = lerpF(morphFrom.complexity,   morphTo.complexity,  s);
    live.trailFade    = lerpF(morphFrom.trailFade,    morphTo.trailFade,   s);
    live.mathA        = lerpF(morphFrom.mathA,        morphTo.mathA,       s);
    live.mathB        = lerpF(morphFrom.mathB,        morphTo.mathB,       s);
    live.mathC        = lerpF(morphFrom.mathC,        morphTo.mathC,       s);
    live.mathD        = lerpF(morphFrom.mathD,        morphTo.mathD,       s);
    live.mathE        = lerpF(morphFrom.mathE,        morphTo.mathE,       s);
    live.mathF        = lerpF(morphFrom.mathF,        morphTo.mathF,       s);

    engine->loadConfig(live);
}

// ─── Token loading ───────────────────────────────────────────────────────────
static ArtConfig loadConfigFromRandom() {
    if (numTokens == 0) {
        ArtConfig cfg = parseConfig(defaultJsonConfig);
        cfg.playDuration = 10;
        return cfg;
    }

    int randomIndex = random(numTokens);
    String filename = tokenFiles[randomIndex];

    File f = LittleFS.open("/token/" + filename, "r");
    if (!f) {
        Serial.println("Failed to open file: " + filename);
        return parseConfig(defaultJsonConfig);
    }
    String jsonStr = f.readString();
    f.close();
    Serial.println("Next token: " + filename);
    return parseConfig(jsonStr.c_str());
}

void loadRandomToken() {
    ArtConfig config = loadConfigFromRandom();
    engine->loadConfig(config);
    animationStartTime = millis();
}

// ─── Public API ──────────────────────────────────────────────────────────────
void token_init() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
    }

    Dir dir = LittleFS.openDir("/token");
    while (dir.next()) {
        if (dir.fileName().endsWith(".json")) {
            if (numTokens < 256) {
                tokenFiles[numTokens] = dir.fileName();
                numTokens++;
            }
        }
    }

    if (numTokens == 0) {
        Serial.println("token 0 10 second start");
    } else {
        Serial.print("token ");
        Serial.println(numTokens);
        for (int i = 0; i < numTokens; ++i) {
            Serial.print("token ");
            Serial.println(i + 1);

            hub75_clear();
            char tokenText[16];
            sprintf(tokenText, "Token %d", i + 1);
            hub75_draw_string(2, TOTAL_HEIGHT / 2 - 4, tokenText, {0, 255, 0}, 1);
            hub75_swap_buffers();

            delay(50);
        }
    }

    loadRandomToken();
}

void token_update() {
    unsigned long now = millis();

    if (isTransitioning) {
        unsigned long elapsed = now - transitionStartTime;

        if (elapsed >= MORPH_DURATION_MS) {
            // ── Transition finished: lock to token B and resume normal play ──
            isTransitioning = false;
            engine->loadConfig(morphTo);
            // animationStartTime was already set at transition start so that
            // the full morph window counts against token B's playDuration.
            animationStartTime = transitionStartTime + MORPH_DURATION_MS;
            hub75_set_brightness(200);
            Serial.println("Morph complete. Playing token B normally.");
        } else {
            // ── Still morphing: push interpolated config every frame ──
            float t = (float)elapsed / (float)MORPH_DURATION_MS;
            applyMorph(t);
            // Keep brightness full — no dimming during morph
            hub75_set_brightness(200);
        }

    } else {
        // ── Normal playback: check if current token's time is up ──
        unsigned long playMs = (unsigned long)engine->getCurrentAnim().playDuration * 1000UL;
        if (now - animationStartTime >= playMs) {
            Serial.println("playDuration reached. Starting 10-second morph to next token.");

            // Snapshot the current live config as the "from" state
            morphFrom = engine->getCurrentAnim();

            // Pick and pre-load the destination token
            morphTo = loadConfigFromRandom();

            isTransitioning = true;
            transitionStartTime = now;
            // animationStartTime will be updated when the morph finishes
        }
    }
}
