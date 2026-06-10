#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "hub75_driver.h"
#include "engine.h"

SuperArtEngine* engine;
unsigned long animationStartTime = 0;
bool engine_ready = false;

const char* jsonConfig = R"=====(
{
    "id": 321,
    "name": "Neon Core 321",
    "algo": 9,
    "palette": 16,
    "lightFx": 80,
    "shape": 46,
    "particleType": 93,
    "symmetry": 4,
    "colorSpeed": 4.8,
    "bpm": 88,
    "playDuration": 58,
    "chaos": 8.24,
    "speed": 0.1,
    "density": 714,
    "complexity": 3.905,
    "trailFade": 0.439,
    "mathA": 1.936,
    "mathB": 1.747,
    "mathC": 7.122,
    "mathD": 5.085,
    "mathE": 2.712,
    "mathF": 0.161
}
)=====";

ArtConfig parseConfig(const char* jsonStr) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);

    ArtConfig config = {0};
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return config;
    }

    config.id = doc["id"] | 0;
    config.algo = doc["algo"] | 0;
    config.palette = doc["palette"] | 0;
    config.lightFx = doc["lightFx"] | 0;
    config.shape = doc["shape"] | 0;
    config.particleType = doc["particleType"] | 0;
    config.symmetry = doc["symmetry"] | 0;
    config.bpm = doc["bpm"] | 120;
    config.playDuration = doc["playDuration"] | 15;
    config.colorSpeed = doc["colorSpeed"] | 1.0f;
    config.chaos = doc["chaos"] | 0.0f;
    config.speed = doc["speed"] | 1.0f;
    config.density = doc["density"] | 100.0f;
    config.complexity = doc["complexity"] | 1.0f;
    config.trailFade = doc["trailFade"] | 1.0f;
    config.mathA = doc["mathA"] | 1.0f;
    config.mathB = doc["mathB"] | 1.0f;
    config.mathC = doc["mathC"] | 1.0f;
    config.mathD = doc["mathD"] | 1.0f;
    config.mathE = doc["mathE"] | 1.0f;
    config.mathF = doc["mathF"] | 1.0f;

    return config;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    hub75_init();
    hub75_set_brightness(200);

    randomSeed(micros());

    engine = new SuperArtEngine(TOTAL_WIDTH, TOTAL_HEIGHT);
    ArtConfig initialConfig = parseConfig(jsonConfig);
    engine->loadConfig(initialConfig);

    animationStartTime = millis();

    Serial.println("Super Art Engine Started");
    engine_ready = true;
}

void loop() {
    static unsigned long last_frame = 0;

    // Limit frame rate
    unsigned long now = millis();
    if (now - last_frame < 33) { // ~30 FPS
        delay(1);
        return;
    }
    last_frame = now;

    engine->update();
    engine->draw();

    hub75_swap_buffers();

    // Check playDuration to implement some sort of logic (for now, simply logging)
    // You could replace engine config randomly here similar to autoPlay
    if (millis() - animationStartTime > engine->getCurrentAnim().playDuration * 1000UL) {
        Serial.println("Animation playDuration reached.");
        animationStartTime = millis(); // restart loop
    }
}

void setup1() {}

void loop1() {
    if (!engine_ready) return;
    hub75_refresh();
}
