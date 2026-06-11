#include "token.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "engine.h"
#include "hub75_driver.h"

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

bool isTransitioning = false;
unsigned long transitionStartTime = 0;
const unsigned long transitionDuration = 10000; // 10 seconds total
const unsigned long fadeOutDuration = 5000; // 5 seconds fade out
bool tokenSwitchedDuringTransition = false;

void loadRandomToken() {
    if (numTokens == 0) {
        ArtConfig config = parseConfig(defaultJsonConfig);
        config.playDuration = 10;
        engine->loadConfig(config);
        animationStartTime = millis();
        return;
    }

    int randomIndex = random(numTokens);
    String filename = tokenFiles[randomIndex];

    File f = LittleFS.open("/token/" + filename, "r");
    if (!f) {
        Serial.println("Failed to open file: " + filename);
        return;
    }

    String jsonStr = f.readString();
    f.close();

    ArtConfig config = parseConfig(jsonStr.c_str());
    engine->loadConfig(config);
    animationStartTime = millis();
    Serial.println("Loaded new config: " + filename);
}

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
            delay(1000); // 1-second delay exactly as requested
        }
    }

    loadRandomToken();
}

void token_update() {
    if (isTransitioning) {
        unsigned long elapsed = millis() - transitionStartTime;
        if (elapsed < fadeOutDuration) {
            // Fade out
            float progress = (float)elapsed / fadeOutDuration;
            uint8_t brightness = 200 - (uint8_t)(progress * 200.0f);
            hub75_set_brightness(brightness);
        } else if (elapsed < transitionDuration) {
            // Switch token if we haven't already
            if (!tokenSwitchedDuringTransition) {
                loadRandomToken();
                // Override animationStartTime so that the new token's duration
                // begins ticking exactly when the full transition is over
                animationStartTime = transitionStartTime + transitionDuration;
                tokenSwitchedDuringTransition = true;
            }
            // Fade in
            float progress = (float)(elapsed - fadeOutDuration) / fadeOutDuration;
            uint8_t brightness = (uint8_t)(progress * 200.0f);
            hub75_set_brightness(brightness);
        } else {
            // End transition
            isTransitioning = false;
            hub75_set_brightness(200);
        }
    } else {
        // Normal playback checking
        if (millis() - animationStartTime > engine->getCurrentAnim().playDuration * 1000UL) {
            Serial.println("Animation playDuration reached. Starting 10 second transition.");
            isTransitioning = true;
            transitionStartTime = millis();
            tokenSwitchedDuringTransition = false;
        }
    }
}
