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

// ─── Morph state ──────────────────────────────────────────────────────────────
//
// STRATEGY: never call loadConfig() during the morph.
//
// The transition works in two phases:
//
//  Phase 1  MORPH  (0 → MORPH_MS)
//    engine->morphParams() nudges the running math values frame-by-frame
//    toward token B's values.  Particles, time, and the visual algo keep
//    running uninterrupted — the animation literally transforms into B.
//
//  Phase 2  SNAP  (one-shot at MORPH_MS)
//    loadConfig(B) is called exactly once at the end.  By now the parameters
//    are already very close to B so there is no visible jump; loadConfig just
//    locks in the correct algo/palette/symmetry if B uses different ones, and
//    reseeds particle positions for the full B playthrough.
//
// Result: a living, breathing merge between A and B — no black frame, no pop.
//
static const unsigned long MORPH_MS = 10000UL;  // 10-second transition

static bool          morphing          = false;
static unsigned long morphStart        = 0;
static ArtConfig     morphTarget;            // full B config — used for param lerp + final snap

// ─── Token loading ────────────────────────────────────────────────────────────
static ArtConfig loadConfigFromRandom() {
    if (numTokens == 0) {
        ArtConfig cfg = parseConfig(defaultJsonConfig);
        cfg.playDuration = 10;
        return cfg;
    }
    int idx = random(numTokens);
    File f = LittleFS.open("/token/" + tokenFiles[idx], "r");
    if (!f) {
        Serial.println("Failed to open: " + tokenFiles[idx]);
        return parseConfig(defaultJsonConfig);
    }
    String json = f.readString();
    f.close();
    Serial.println("Next token: " + tokenFiles[idx]);
    return parseConfig(json.c_str());
}

void loadRandomToken() {
    engine->loadConfig(loadConfigFromRandom());
    animationStartTime = millis();
}

// ─── Public API ───────────────────────────────────────────────────────────────
void token_init() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
    }

    Dir dir = LittleFS.openDir("/token");
    while (dir.next()) {
        if (dir.fileName().endsWith(".json") && numTokens < 256) {
            tokenFiles[numTokens++] = dir.fileName();
        }
    }

    if (numTokens == 0) {
        Serial.println("No tokens found — using default");
    } else {
        Serial.printf("Found %d tokens\n", numTokens);
        for (int i = 0; i < numTokens; i++) {
            hub75_clear();
            char buf[16];
            sprintf(buf, "Token %d", i + 1);
            hub75_draw_string(2, TOTAL_HEIGHT / 2 - 4, buf, {0, 255, 0}, 1);
            hub75_swap_buffers();
            delay(50);
        }
    }

    loadRandomToken();
}

void token_update() {
    unsigned long now = millis();

    if (morphing) {
        unsigned long elapsed = now - morphStart;

        if (elapsed >= MORPH_MS) {
            // ── Phase 2: snap to B ──────────────────────────────────────────
            // Parameters have drifted all the way to B; a clean loadConfig now
            // causes no visible pop but correctly resets algo/palette/symmetry
            // and gives B fresh particle seeds for its full playthrough.
            morphing = false;
            engine->loadConfig(morphTarget);
            animationStartTime = now;
            hub75_set_brightness(200);
            Serial.println("Morph complete → playing token B");

        } else {
            // ── Phase 1: live morph ─────────────────────────────────────────
            // t goes 0→1 over MORPH_MS.  morphParams() lerps all math fields
            // and refreshes the engine's precomputed cache — no particle reset.
            float t = (float)elapsed / (float)MORPH_MS;
            engine->morphParams(morphTarget, t);
        }

    } else {
        // ── Normal playback ─────────────────────────────────────────────────
        unsigned long playMs = (unsigned long)engine->getCurrentAnim().playDuration * 1000UL;
        if (now - animationStartTime >= playMs) {
            Serial.println("playDuration ended → starting 10 s morph");

            // Pick B before we start so the morph targets it from frame 1
            morphTarget = loadConfigFromRandom();
            morphing    = true;
            morphStart  = now;
            // animationStartTime is updated in the snap step above
        }
    }
}
