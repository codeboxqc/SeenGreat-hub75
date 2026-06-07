

/*
 * ESP32 TTGO T-Display Disaster Alert v2.0
 * (Converted from Pico 2 W + HUB75)
 *
 * FIX v1.3 carried over:
 *  sendToHeltec() fires AFTER http.end()
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ==================== WIFI ====================
const char* WIFI_SSID     = "demonoid";
const char* WIFI_PASSWORD = "lacasadepapel2019";

// ==================== TIMING ====================
#define FETCH_INTERVAL_MS   (5UL * 60UL * 1000UL)
#define DISPLAY_DURATION_MS (8UL * 1000UL)
#define WIFI_TIMEOUT_MS     30000

// ==================== TFT DISPLAY ====================
TFT_eSPI tft = TFT_eSPI();
#define TFT_BL_PIN 4  // TTGO T-Display Backlight

// ==================== MESHTASTIC ====================
#define MESH_TX_PIN 27    // TTGO GPIO 27 (TX) -> Heltec pin 48 (RX)
#define MESH_RX_PIN 25    // TTGO GPIO 25 (RX) <- Heltec pin 47 (TX)
#define MESH_BAUD   9600

// ==================== API ====================
const char* USGS_URL = "https://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/4.5_day.geojson";

// ==================== EEPROM ====================
#define EEPROM_SIZE    512
#define EEPROM_MAGIC   0xDA
#define EEPROM_VERSION 0x01
#define MAX_EVENTS     20
#define ID_LENGTH      24

char seenEvents[MAX_EVENTS][ID_LENGTH];
int  seenCount = 0;
int  seenIndex = 0;

// ==================== QUEUE ====================
struct DisasterEvent {
    char    id[ID_LENGTH];
    char    location[64]; // Increased size for TFT display
    float   magnitude;
    uint8_t alertLevel;
};

DisasterEvent displayQueue[5];
int queueHead  = 0;
int queueTail  = 0;
int queueCount = 0;

// ==================== LORA PENDING QUEUE ====================
#define LORA_QUEUE_SIZE 5
char loraQueue[LORA_QUEUE_SIZE][80];
int  loraQueueCount = 0;

// ==================== MESHTASTIC TX ====================

void sendToHeltec(const char* message) {
    if (!message || strlen(message) == 0) return;
    Serial.print("[MESH TX] ");
    Serial.println(message);
    Serial1.println(message);
    delay(100); 
}

void queueLoraMessage(const char* message) {
    if (loraQueueCount >= LORA_QUEUE_SIZE) return;
    strncpy(loraQueue[loraQueueCount], message, 79);
    loraQueue[loraQueueCount][79] = '\0';
    loraQueueCount++;
}

void flushLoraQueue() {
    if (loraQueueCount == 0) return;
    Serial.printf("[MESH] Flushing %d queued messages\n", loraQueueCount);
    delay(200);  
    for (int i = 0; i < loraQueueCount; i++) {
        sendToHeltec(loraQueue[i]);
        delay(500); 
    }
    loraQueueCount = 0;
}

// ==================== EEPROM ====================

void eeprom_load() {
    EEPROM.begin(EEPROM_SIZE);
    if (EEPROM.read(0) != EEPROM_MAGIC || EEPROM.read(1) != EEPROM_VERSION) {
        Serial.println("[EEPROM] Fresh start");
        seenCount = 0; seenIndex = 0; return;
    }
    seenCount = EEPROM.read(2) | (EEPROM.read(3) << 8);
    seenIndex = EEPROM.read(4) | (EEPROM.read(5) << 8);
    if (seenCount > MAX_EVENTS) seenCount = MAX_EVENTS;
    if (seenIndex >= MAX_EVENTS) seenIndex = 0;
    int addr = 6;
    for (int i=0;i<seenCount;i++)
        for (int j=0;j<ID_LENGTH;j++)
            seenEvents[i][j] = EEPROM.read(addr++);
    Serial.printf("[EEPROM] Loaded %d seen events\n", seenCount);
}

void eeprom_save() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(0, EEPROM_MAGIC);  EEPROM.write(1, EEPROM_VERSION);
    EEPROM.write(2, seenCount&0xFF); EEPROM.write(3,(seenCount>>8)&0xFF);
    EEPROM.write(4, seenIndex&0xFF); EEPROM.write(5,(seenIndex>>8)&0xFF);
    int addr = 6;
    for (int i=0;i<seenCount;i++)
        for (int j=0;j<ID_LENGTH;j++)
            EEPROM.write(addr++, seenEvents[i][j]);
    EEPROM.commit();
    Serial.printf("[EEPROM] Saved %d events\n", seenCount);
}

void eeprom_clear() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(0, 0x00);
    EEPROM.commit();
    seenCount = 0; seenIndex = 0;
    memset(seenEvents, 0, sizeof(seenEvents));
    Serial.println("[EEPROM] Cleared — all quakes will re-queue");
}

// ==================== SCREENS ====================
void showStartup()  { 
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM); // Middle center
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("DISASTER", 120, 50, 4); 
    tft.drawString("ALERT", 120, 85, 4);
}

void showFetching() { 
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("FETCHING", 120, 50, 4); 
    tft.drawString("DATA...", 120, 85, 4);
}

void showNoAlerts() { 
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(0, 0, 240, 135, TFT_GREEN);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("MONITORING", 120, 50, 4);
    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
    tft.drawString("NO ALERTS", 120, 85, 4);
}

void showConnecting(int dots) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("CONNECTING", 120, 50, 4);
    
    String d = "WIFI ";
    for(int i=0; i<(dots%5); i++) d += ".";
    tft.drawString(d, 120, 85, 4);
}

void showConnected() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("CONNECTED", 120, 50, 4);
    tft.drawString(WiFi.localIP().toString(), 120, 85, 4);
}

void showError(const char* msg) {
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(0, 0, 240, 135, TFT_RED);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("ERROR", 120, 45, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(msg, 120, 85, 2);
}

void showAlert(DisasterEvent* evt) {
    tft.fillScreen(TFT_BLACK);
    uint16_t c = TFT_GREEN;
    if(evt->alertLevel == 2) c = TFT_RED;
    else if(evt->alertLevel == 1) c = TFT_ORANGE;
    
    // Top colored bar
    tft.fillRect(0, 0, 240, 10, c);

    // Header (QUAKE + Magnitude)
    tft.setTextDatum(TL_DATUM); // Top left
    tft.setTextColor(c, TFT_BLACK);
    tft.drawString("QUAKE", 10, 20, 4); // Font 4 (26px)

    char mag[16]; 
    sprintf(mag, "M%.1f", evt->magnitude);
    tft.setTextDatum(TR_DATUM); // Top right
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(mag, 230, 20, 4);

    // Location (Wrapped text)
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 60);
    tft.setTextFont(2); // 16px Font
    tft.setTextSize(2); // Scale x2 = 32px roughly
    tft.setTextWrap(true, true);
    tft.print(evt->location);
}

// ==================== EVENT TRACKING ====================

bool isEventSeen(const char* id) {
    for(int i=0;i<seenCount;i++) if(strcmp(seenEvents[i],id)==0) return true;
    return false;
}

void markEventSeen(const char* id) {
    if(isEventSeen(id)) return;
    strncpy(seenEvents[seenIndex],id,ID_LENGTH-1);
    seenEvents[seenIndex][ID_LENGTH-1]='\0';
    seenIndex=(seenIndex+1)%MAX_EVENTS;
    if(seenCount<MAX_EVENTS) seenCount++;
    Serial.printf("[SEEN] %s (total:%d)\n",id,seenCount);
    static int n=0; if(++n>=5){eeprom_save();n=0;}
}

bool addToQueue(DisasterEvent* evt) {
    if(isEventSeen(evt->id)) return false;
    if(queueCount>=5){queueHead=(queueHead+1)%5;queueCount--;}
    memcpy(&displayQueue[queueTail],evt,sizeof(DisasterEvent));
    queueTail=(queueTail+1)%5; queueCount++;
    Serial.printf("[QUEUE] M%.1f %s (q:%d)\n",evt->magnitude,evt->location,queueCount);

    char msg[80];
    snprintf(msg,sizeof(msg),"QUAKE M%.1f %s",evt->magnitude,evt->location);
    queueLoraMessage(msg);
    return true;
}

bool getFromQueue(DisasterEvent* evt) {
    if(queueCount==0) return false;
    memcpy(evt,&displayQueue[queueHead],sizeof(DisasterEvent));
    queueHead=(queueHead+1)%5; queueCount--;
    markEventSeen(evt->id);
    return true;
}

// ==================== FETCH ====================

int fetchUSGS() {
    Serial.println("[USGS] Fetching...");
    loraQueueCount = 0;  // Clear LoRa staging buffer

    WiFiClientSecure client; client.setInsecure();
    HTTPClient http;
    http.begin(client,USGS_URL); http.setTimeout(15000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode=http.GET();
    int newEvents=0;
    Serial.printf("[USGS] HTTP %d\n",httpCode);

    if(httpCode==HTTP_CODE_OK){
        WiFiClient* stream=http.getStreamPtr();

        StaticJsonDocument<200> filter;
        filter["features"][0]["id"]=true;
        filter["features"][0]["properties"]["mag"]=true;
        filter["features"][0]["properties"]["place"]=true;

        StaticJsonDocument<4096> doc;
        DeserializationError err=deserializeJson(doc,*stream,
                                  DeserializationOption::Filter(filter));
        if(err){
            Serial.printf("[USGS] JSON error: %s\n",err.c_str());
        } else {
            JsonArray features=doc["features"];
            Serial.printf("[USGS] Parsed %d quakes\n",features.size());
            int count=0;
            for(JsonObject feature:features){
                if(++count>5) break;
                DisasterEvent evt; memset(&evt,0,sizeof(evt));
                
                const char* id=feature["id"]|"unknown";
                snprintf(evt.id,sizeof(evt.id),"usgs_%s",id);
                
                JsonObject props=feature["properties"];
                evt.magnitude=props["mag"]|0.0f;
                const char* place=props["place"]|"Unknown";
                const char* of=strstr(place," of ");
                strncpy(evt.location,of?(of+4):place,sizeof(evt.location)-1);
                
                if(evt.magnitude>=7.0)      evt.alertLevel=2;
                else if(evt.magnitude>=5.5) evt.alertLevel=1;
                else                        evt.alertLevel=0;
                
                if(addToQueue(&evt)) newEvents++;
            }
        }
    } else {
        Serial.printf("[USGS] HTTP error: %d\n",httpCode);
    }

    http.end();  

    flushLoraQueue();

    Serial.printf("[USGS] %d new events\n",newEvents);
    return newEvents;
}

// ==================== GLOBALS ====================
unsigned long lastFetchTime     = 0;
unsigned long lastDisplayChange = 0;
bool          wifiConnected     = false;
DisasterEvent currentEvent;
bool          showingAlert      = false;

// ==================== SETUP ====================

void setup() {
    Serial.begin(115200);
    
    // Init TFT
    tft.init();
    tft.setRotation(1); // Landscape (240x135)
    tft.fillScreen(TFT_BLACK);
    
    // Turn on TFT Backlight manually if needed
    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH);

    Serial.println("\n=================================");
    Serial.println("  Disaster Alert Display v2.0");
    Serial.println("  ESP32 TTGO T-Display + LoRa");
    Serial.println("=================================");
    Serial.println("  C = clear EEPROM + re-send LoRa");
    Serial.println("  T = test LoRa message");
    Serial.println("=================================\n");

    // Initialize UART 1 for Meshtastic
    Serial1.begin(MESH_BAUD, SERIAL_8N1, MESH_RX_PIN, MESH_TX_PIN);
    Serial.printf("[MESH] GPIO%d->Heltec(RX)  GPIO%d<-Heltec(TX)  %dbaud\n", MESH_TX_PIN, MESH_RX_PIN, MESH_BAUD);

    eeprom_load();
    showStartup();
    delay(2000);

    sendToHeltec("DisasterAlert v2.0 online");

    Serial.printf("[WIFI] Connecting to %s\n",WIFI_SSID);
    WiFi.mode(WIFI_STA); 
    WiFi.disconnect(); 
    delay(100);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start=millis(); 
    int dots=0;
    while(WiFi.status()!=WL_CONNECTED){
        Serial.printf("[WIFI] Status:%d\n",WiFi.status());
        showConnecting(dots++);
        delay(500);
        
        if(millis()-start>WIFI_TIMEOUT_MS){
            Serial.println("[WIFI] Timeout!");
            showError("WIFI FAIL");
            delay(5000);
            break;
        }
    }

    if(WiFi.status()==WL_CONNECTED){
        wifiConnected=true;
        Serial.print("[WIFI] IP: "); Serial.println(WiFi.localIP());
        showConnected(); 
        delay(1500);
        
        showFetching();  
        delay(500);
        
        int n=fetchUSGS();
        Serial.printf("[FETCH] %d new events\n",n);
        lastFetchTime=millis();
    }
    Serial.println("[MAIN] Ready");
}

// ==================== LOOP ====================

void loop() {
    if(Serial.available()){
        char cmd=Serial.read();
        if(cmd=='C'||cmd=='c'){
            eeprom_clear();
            if(wifiConnected){ fetchUSGS(); lastFetchTime=millis(); }
        }
        if(cmd=='T'||cmd=='t'){
            sendToHeltec("TEST DisasterAlert TTGO->Heltec");
        }
    }

    if(wifiConnected&&WiFi.status()!=WL_CONNECTED){
        wifiConnected=false; Serial.println("[WIFI] Lost!");
    }

    if(wifiConnected&&(millis()-lastFetchTime>=FETCH_INTERVAL_MS)){
        Serial.println("[FETCH] Periodic...");
        showFetching(); 
        delay(500);
        fetchUSGS(); 
        lastFetchTime=millis();
    }

    unsigned long now=millis();
    if(queueCount>0){
        if(!showingAlert||(now-lastDisplayChange>=DISPLAY_DURATION_MS)){
            if(getFromQueue(&currentEvent)){
                showAlert(&currentEvent);
                showingAlert=true; lastDisplayChange=now;
                Serial.printf("[DISPLAY] M%.1f %s\n",
                              currentEvent.magnitude,currentEvent.location);
            }
        }
    } else {
        if(showingAlert||(now-lastDisplayChange>=5000)){
            showNoAlerts(); showingAlert=false; lastDisplayChange=now;
        }
    }
}














/*
//Pico 2 W Disaster Alert Display v1.3
 * For Seengreat RGB Matrix Adapter + HUB75 Panel
 

 /*

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// ==================== WIFI ====================
const char* WIFI_SSID     = "demonoid";
const char* WIFI_PASSWORD = "lacasadepapel2019";

// ==================== TIMING ====================
#define FETCH_INTERVAL_MS   (5UL * 60UL * 1000UL)
#define DISPLAY_DURATION_MS (8UL * 1000UL)
#define WIFI_TIMEOUT_MS     30000

// ==================== PANEL ====================
#define WIDTH  64
#define HEIGHT 32
#define ROWS   (HEIGHT / 2)

// ==================== SEENGREAT PINS ====================
#define PIN_R1  2
#define PIN_G1  3
#define PIN_B1  4
#define PIN_R2  5
#define PIN_G2  8
#define PIN_B2  9
#define PIN_A   10
#define PIN_B   16
#define PIN_C   18
#define PIN_D   20
#define PIN_E   22
#define PIN_CLK 11
#define PIN_LAT 12
#define PIN_OE  13

// ==================== MESHTASTIC ====================
#define MESH_TX_PIN 0    // Pico GP0 (TX) -> Heltec pin 48 (RX)
#define MESH_RX_PIN 1    // Pico GP1 (RX) <- Heltec pin 47 (TX)
#define MESH_BAUD   9600

// ==================== API ====================
const char* USGS_URL = "https://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/4.5_day.geojson";

// ==================== EEPROM ====================
#define EEPROM_SIZE    512
#define EEPROM_MAGIC   0xDA
#define EEPROM_VERSION 0x01
#define MAX_EVENTS     20
#define ID_LENGTH      24

char seenEvents[MAX_EVENTS][ID_LENGTH];
int  seenCount = 0;
int  seenIndex = 0;

// ==================== QUEUE ====================
struct DisasterEvent {
    char    id[ID_LENGTH];
    char    location[32];
    float   magnitude;
    uint8_t alertLevel;
};

DisasterEvent displayQueue[5];
int queueHead  = 0;
int queueTail  = 0;
int queueCount = 0;

// ==================== LORA PENDING QUEUE ====================
// Holds messages to send AFTER WiFi HTTP is closed
#define LORA_QUEUE_SIZE 5
char loraQueue[LORA_QUEUE_SIZE][80];
int  loraQueueCount = 0;

// ==================== FRAMEBUFFER ====================
uint8_t framebuffer[WIDTH * HEIGHT * 3];

// ==================== FONT ====================
const uint8_t font5x7[] = {
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x5F,0x00,0x00, 0x00,0x07,0x00,0x07,0x00,
    0x14,0x7F,0x14,0x7F,0x14, 0x24,0x2A,0x7F,0x2A,0x12, 0x23,0x13,0x08,0x64,0x62,
    0x36,0x49,0x55,0x22,0x50, 0x00,0x05,0x03,0x00,0x00, 0x00,0x1C,0x22,0x41,0x00,
    0x00,0x41,0x22,0x1C,0x00, 0x08,0x2A,0x1C,0x2A,0x08, 0x08,0x08,0x3E,0x08,0x08,
    0x00,0x50,0x30,0x00,0x00, 0x08,0x08,0x08,0x08,0x08, 0x00,0x60,0x60,0x00,0x00,
    0x20,0x10,0x08,0x04,0x02, 0x3E,0x51,0x49,0x45,0x3E, 0x00,0x42,0x7F,0x40,0x00,
    0x42,0x61,0x51,0x49,0x46, 0x21,0x41,0x45,0x4B,0x31, 0x18,0x14,0x12,0x7F,0x10,
    0x27,0x45,0x45,0x45,0x39, 0x3C,0x4A,0x49,0x49,0x30, 0x01,0x71,0x09,0x05,0x03,
    0x36,0x49,0x49,0x49,0x36, 0x06,0x49,0x49,0x29,0x1E, 0x00,0x36,0x36,0x00,0x00,
    0x00,0x56,0x36,0x00,0x00, 0x00,0x08,0x14,0x22,0x41, 0x14,0x14,0x14,0x14,0x14,
    0x41,0x22,0x14,0x08,0x00, 0x02,0x01,0x51,0x09,0x06, 0x32,0x49,0x79,0x41,0x3E,
    0x7E,0x11,0x11,0x11,0x7E, 0x7F,0x49,0x49,0x49,0x36, 0x3E,0x41,0x41,0x41,0x22,
    0x7F,0x41,0x41,0x22,0x1C, 0x7F,0x49,0x49,0x49,0x41, 0x7F,0x09,0x09,0x01,0x01,
    0x3E,0x41,0x41,0x51,0x32, 0x7F,0x08,0x08,0x08,0x7F, 0x00,0x41,0x7F,0x41,0x00,
    0x20,0x40,0x41,0x3F,0x01, 0x7F,0x08,0x14,0x22,0x41, 0x7F,0x40,0x40,0x40,0x40,
    0x7F,0x02,0x04,0x02,0x7F, 0x7F,0x04,0x08,0x10,0x7F, 0x3E,0x41,0x41,0x41,0x3E,
    0x7F,0x09,0x09,0x09,0x06, 0x3E,0x41,0x51,0x21,0x5E, 0x7F,0x09,0x19,0x29,0x46,
    0x46,0x49,0x49,0x49,0x31, 0x01,0x01,0x7F,0x01,0x01, 0x3F,0x40,0x40,0x40,0x3F,
    0x1F,0x20,0x40,0x20,0x1F, 0x7F,0x20,0x18,0x20,0x7F, 0x63,0x14,0x08,0x14,0x63,
    0x03,0x04,0x78,0x04,0x03, 0x61,0x51,0x49,0x45,0x43,
};

// ==================== MESHTASTIC TX ====================

void sendToHeltec(const char* message) {
    if (!message || strlen(message) == 0) return;
    Serial.print("[MESH TX] ");
    Serial.println(message);
    Serial1.println(message);
    delay(100);  // Give Meshtastic time to process between messages
}

// Queue a LoRa message to send later (after WiFi HTTP closes)
void queueLoraMessage(const char* message) {
    if (loraQueueCount >= LORA_QUEUE_SIZE) return;
    strncpy(loraQueue[loraQueueCount], message, 79);
    loraQueue[loraQueueCount][79] = '\0';
    loraQueueCount++;
}

// Flush all queued LoRa messages — call this AFTER http.end()
void flushLoraQueue() {
    if (loraQueueCount == 0) return;
    Serial.printf("[MESH] Flushing %d queued messages (WiFi now idle)\n", loraQueueCount);
    delay(200);  // Let WiFi chip settle after HTTP close
    for (int i = 0; i < loraQueueCount; i++) {
        sendToHeltec(loraQueue[i]);
        delay(500);  // Space out messages so Meshtastic can handle each one
    }
    loraQueueCount = 0;
}

// ==================== EEPROM ====================

void eeprom_load() {
    EEPROM.begin(EEPROM_SIZE);
    if (EEPROM.read(0) != EEPROM_MAGIC || EEPROM.read(1) != EEPROM_VERSION) {
        Serial.println("[EEPROM] Fresh start");
        seenCount = 0; seenIndex = 0; return;
    }
    seenCount = EEPROM.read(2) | (EEPROM.read(3) << 8);
    seenIndex = EEPROM.read(4) | (EEPROM.read(5) << 8);
    if (seenCount > MAX_EVENTS) seenCount = MAX_EVENTS;
    if (seenIndex >= MAX_EVENTS) seenIndex = 0;
    int addr = 6;
    for (int i=0;i<seenCount;i++)
        for (int j=0;j<ID_LENGTH;j++)
            seenEvents[i][j] = EEPROM.read(addr++);
    Serial.printf("[EEPROM] Loaded %d seen events\n", seenCount);
}

void eeprom_save() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(0, EEPROM_MAGIC);  EEPROM.write(1, EEPROM_VERSION);
    EEPROM.write(2, seenCount&0xFF); EEPROM.write(3,(seenCount>>8)&0xFF);
    EEPROM.write(4, seenIndex&0xFF); EEPROM.write(5,(seenIndex>>8)&0xFF);
    int addr = 6;
    for (int i=0;i<seenCount;i++)
        for (int j=0;j<ID_LENGTH;j++)
            EEPROM.write(addr++, seenEvents[i][j]);
    EEPROM.commit();
    Serial.printf("[EEPROM] Saved %d events\n", seenCount);
}

void eeprom_clear() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(0, 0x00);
    EEPROM.commit();
    seenCount = 0; seenIndex = 0;
    memset(seenEvents, 0, sizeof(seenEvents));
    Serial.println("[EEPROM] Cleared — all quakes will re-queue");
}

// ==================== DISPLAY ====================

void initPins() {
    pinMode(PIN_R1,OUTPUT); pinMode(PIN_G1,OUTPUT); pinMode(PIN_B1,OUTPUT);
    pinMode(PIN_R2,OUTPUT); pinMode(PIN_G2,OUTPUT); pinMode(PIN_B2,OUTPUT);
    pinMode(PIN_A,OUTPUT);  pinMode(PIN_B,OUTPUT);  pinMode(PIN_C,OUTPUT);
    pinMode(PIN_D,OUTPUT);  pinMode(PIN_E,OUTPUT);
    pinMode(PIN_CLK,OUTPUT); pinMode(PIN_LAT,OUTPUT); pinMode(PIN_OE,OUTPUT);
    digitalWrite(PIN_OE,HIGH); digitalWrite(PIN_LAT,LOW); digitalWrite(PIN_CLK,LOW);
}

void setRowAddress(int row) {
    digitalWrite(PIN_A,row&1); digitalWrite(PIN_B,(row>>1)&1);
    digitalWrite(PIN_C,(row>>2)&1); digitalWrite(PIN_D,(row>>3)&1);
}

void refreshDisplay() {
    for (int row=0; row<ROWS; row++) {
        digitalWrite(PIN_OE,HIGH); setRowAddress(row);
        for (int x=0; x<WIDTH; x++) {
            int t=(row*WIDTH+x)*3, b=((row+ROWS)*WIDTH+x)*3;
            digitalWrite(PIN_R1,framebuffer[t+0]>128); digitalWrite(PIN_G1,framebuffer[t+1]>128);
            digitalWrite(PIN_B1,framebuffer[t+2]>128); digitalWrite(PIN_R2,framebuffer[b+0]>128);
            digitalWrite(PIN_G2,framebuffer[b+1]>128); digitalWrite(PIN_B2,framebuffer[b+2]>128);
            digitalWrite(PIN_CLK,HIGH); digitalWrite(PIN_CLK,LOW);
        }
        digitalWrite(PIN_LAT,HIGH); digitalWrite(PIN_LAT,LOW);
        digitalWrite(PIN_OE,LOW); delayMicroseconds(100);
    }
}

void clearDisplay() { memset(framebuffer,0,sizeof(framebuffer)); }

void setPixel(int x,int y,uint8_t r,uint8_t g,uint8_t b) {
    if(x<0||x>=WIDTH||y<0||y>=HEIGHT) return;
    int i=(y*WIDTH+x)*3; framebuffer[i]=r; framebuffer[i+1]=g; framebuffer[i+2]=b;
}
void fillRect(int x,int y,int w,int h,uint8_t r,uint8_t g,uint8_t b) {
    for(int j=0;j<h;j++) for(int i=0;i<w;i++) setPixel(x+i,y+j,r,g,b);
}
void drawRect(int x,int y,int w,int h,uint8_t r,uint8_t g,uint8_t b) {
    for(int i=0;i<w;i++){setPixel(x+i,y,r,g,b);setPixel(x+i,y+h-1,r,g,b);}
    for(int i=0;i<h;i++){setPixel(x,y+i,r,g,b);setPixel(x+w-1,y+i,r,g,b);}
}
void drawChar(int x,int y,char c,uint8_t r,uint8_t g,uint8_t b) {
    if(c>='a'&&c<='z') c-=32;
    if(c<32||c>90) c='?';
    int idx=(c-32)*5;
    for(int col=0;col<5;col++){
        uint8_t line=font5x7[idx+col];
        for(int row=0;row<7;row++) if(line&(1<<row)) setPixel(x+col,y+row,r,g,b);
    }
}
void drawString(int x,int y,const char* s,uint8_t r,uint8_t g,uint8_t b) {
    while(*s){if(*s==' ')x+=4;else{drawChar(x,y,*s,r,g,b);x+=6;}s++;}
}
void drawStringTrunc(int x,int y,const char* s,int max,uint8_t r,uint8_t g,uint8_t b) {
    int n=0; while(*s&&n<max){if(*s==' ')x+=4;else{drawChar(x,y,*s,r,g,b);x+=6;}s++;n++;}
}

// ==================== SCREENS ====================
void showStartup()  { clearDisplay(); drawString(4,4,"DISASTER",0,255,255); drawString(10,20,"ALERT",0,255,255); }
void showFetching() { clearDisplay(); drawString(1,4,"FETCHING",255,165,0); drawString(10,20,"DATA",255,165,0); }
void showNoAlerts() { clearDisplay(); drawRect(0,0,WIDTH,HEIGHT,0,255,0); drawString(1,4,"MONITORING",0,255,0); drawString(1,20,"NO ALERTS",0,200,200); }
void showConnecting(int dots) {
    clearDisplay(); drawString(1,4,"CONNECTING",255,255,0); drawString(4,20,"WIFI",255,255,0);
    for(int i=0;i<(dots%5);i++) fillRect(10+i*11,22,6,4,255,255,0);
}
void showConnected() {
    clearDisplay(); drawString(4,4,"CONNECTED",0,255,0);
    IPAddress ip=WiFi.localIP(); char s[16]; sprintf(s,"%d.%d",ip[2],ip[3]);
    drawString(4,20,s,0,255,0);
}
void showError(const char* msg) {
    clearDisplay(); drawRect(0,0,WIDTH,HEIGHT,255,0,0);
    drawString(10,4,"ERROR",255,0,0); drawStringTrunc(2,20,msg,10,255,255,255);
}
void showAlert(DisasterEvent* evt) {
    clearDisplay();
    uint8_t r=0,g=255,b=0;
    if(evt->alertLevel==2){r=255;g=0;b=0;}
    else if(evt->alertLevel==1){r=255;g=165;b=0;}
    fillRect(0,0,WIDTH,3,r,g,b);
    drawString(2,5,"QUAKE",r,g,b);
    char mag[8]; sprintf(mag,"M%.1f",evt->magnitude);
    drawString(38,5,mag,255,255,0);
    drawStringTrunc(2,20,evt->location,10,255,255,255);
}

// ==================== EVENT TRACKING ====================

bool isEventSeen(const char* id) {
    for(int i=0;i<seenCount;i++) if(strcmp(seenEvents[i],id)==0) return true;
    return false;
}
void markEventSeen(const char* id) {
    if(isEventSeen(id)) return;
    strncpy(seenEvents[seenIndex],id,ID_LENGTH-1);
    seenEvents[seenIndex][ID_LENGTH-1]='\0';
    seenIndex=(seenIndex+1)%MAX_EVENTS;
    if(seenCount<MAX_EVENTS) seenCount++;
    Serial.printf("[SEEN] %s (total:%d)\n",id,seenCount);
    static int n=0; if(++n>=5){eeprom_save();n=0;}
}

// ✅ KEY FIX: addToQueue only queues the LoRa message, does NOT send yet
// Actual send happens in fetchUSGS() AFTER http.end()
bool addToQueue(DisasterEvent* evt) {
    if(isEventSeen(evt->id)) return false;
    if(queueCount>=5){queueHead=(queueHead+1)%5;queueCount--;}
    memcpy(&displayQueue[queueTail],evt,sizeof(DisasterEvent));
    queueTail=(queueTail+1)%5; queueCount++;
    Serial.printf("[QUEUE] M%.1f %s (q:%d)\n",evt->magnitude,evt->location,queueCount);

    // Stage LoRa message — will be sent after http.end()
    char msg[80];
    snprintf(msg,sizeof(msg),"QUAKE M%.1f %s",evt->magnitude,evt->location);
    queueLoraMessage(msg);

    return true;
}

bool getFromQueue(DisasterEvent* evt) {
    if(queueCount==0) return false;
    memcpy(evt,&displayQueue[queueHead],sizeof(DisasterEvent));
    queueHead=(queueHead+1)%5; queueCount--;
    markEventSeen(evt->id);
    return true;
}

// ==================== FETCH ====================

int fetchUSGS() {
    Serial.println("[USGS] Fetching...");
    loraQueueCount = 0;  // Clear LoRa staging buffer

    WiFiClientSecure client; client.setInsecure();
    HTTPClient http;
    http.begin(client,USGS_URL); http.setTimeout(15000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode=http.GET();
    int newEvents=0;
    Serial.printf("[USGS] HTTP %d\n",httpCode);

    if(httpCode==HTTP_CODE_OK){
        WiFiClient* stream=http.getStreamPtr();

        StaticJsonDocument<200> filter;
        filter["features"][0]["id"]=true;
        filter["features"][0]["properties"]["mag"]=true;
        filter["features"][0]["properties"]["place"]=true;

        StaticJsonDocument<4096> doc;
        DeserializationError err=deserializeJson(doc,*stream,
                                  DeserializationOption::Filter(filter));
        if(err){
            Serial.printf("[USGS] JSON error: %s\n",err.c_str());
        } else {
            JsonArray features=doc["features"];
            Serial.printf("[USGS] Parsed %d quakes\n",features.size());
            int count=0;
            for(JsonObject feature:features){
                if(++count>5) break;
                DisasterEvent evt; memset(&evt,0,sizeof(evt));
                const char* id=feature["id"]|"unknown";
                snprintf(evt.id,sizeof(evt.id),"usgs_%s",id);
                JsonObject props=feature["properties"];
                evt.magnitude=props["mag"]|0.0f;
                const char* place=props["place"]|"Unknown";
                const char* of=strstr(place," of ");
                strncpy(evt.location,of?(of+4):place,sizeof(evt.location)-1);
                if(evt.magnitude>=7.0)      evt.alertLevel=2;
                else if(evt.magnitude>=5.5) evt.alertLevel=1;
                else                        evt.alertLevel=0;
                if(addToQueue(&evt)) newEvents++;
            }
        }
    } else {
        Serial.printf("[USGS] HTTP error: %d\n",httpCode);
    }

    http.end();  // ✅ Close WiFi FIRST

    // ✅ NOW send to Heltec — WiFi chip is idle, UART is clean
    flushLoraQueue();

    Serial.printf("[USGS] %d new events\n",newEvents);
    return newEvents;
}

// ==================== GLOBALS ====================
unsigned long lastFetchTime     = 0;
unsigned long lastDisplayChange = 0;
bool          wifiConnected     = false;
DisasterEvent currentEvent;
bool          showingAlert      = false;

// ==================== SETUP ====================

void setup() {
    Serial.begin(115200);
    delay(3000);

    Serial.println("\n=================================");
    Serial.println("  Disaster Alert Display v1.3");
    Serial.println("  Pico 2 W + Seengreat + HUB75");
    Serial.println("=================================");
    Serial.println("  C = clear EEPROM + re-send LoRa");
    Serial.println("  T = test LoRa message");
    Serial.println("=================================\n");

    Serial1.setTX(MESH_TX_PIN); Serial1.setRX(MESH_RX_PIN); Serial1.begin(MESH_BAUD);
    Serial.printf("[MESH] GP%d->Heltec48(RX)  GP%d<-Heltec47(TX)  %dbaud\n",
                  MESH_TX_PIN,MESH_RX_PIN,MESH_BAUD);

    eeprom_load();
    initPins(); clearDisplay(); showStartup();
    for(int i=0;i<100;i++){refreshDisplay();delay(10);}

    // Boot message — no WiFi active yet so this should work fine
    sendToHeltec("DisasterAlert v1.3 online");

    Serial.printf("[WIFI] Connecting to %s\n",WIFI_SSID);
    WiFi.mode(WIFI_STA); WiFi.disconnect(); delay(100);
    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);

    unsigned long start=millis(); int dots=0;
    while(WiFi.status()!=WL_CONNECTED){
        Serial.printf("[WIFI] Status:%d\n",WiFi.status());
        showConnecting(dots++);
        for(int i=0;i<50;i++){refreshDisplay();delay(10);}
        if(millis()-start>WIFI_TIMEOUT_MS){
            Serial.println("[WIFI] Timeout!");
            showError("WIFI FAIL");
            for(int i=0;i<200;i++){refreshDisplay();delay(10);}
            break;
        }
    }

    if(WiFi.status()==WL_CONNECTED){
        wifiConnected=true;
        Serial.print("[WIFI] IP: "); Serial.println(WiFi.localIP());
        showConnected(); for(int i=0;i<150;i++){refreshDisplay();delay(10);}
        showFetching();  for(int i=0;i<50;i++) {refreshDisplay();delay(10);}
        int n=fetchUSGS();
        Serial.printf("[FETCH] %d new events\n",n);
        lastFetchTime=millis();
    }
    Serial.println("[MAIN] Ready");
}

// ==================== LOOP ====================

void loop() {
    refreshDisplay();

    if(Serial.available()){
        char cmd=Serial.read();
        if(cmd=='C'||cmd=='c'){
            eeprom_clear();
            if(wifiConnected){ fetchUSGS(); lastFetchTime=millis(); }
        }
        if(cmd=='T'||cmd=='t'){
            sendToHeltec("TEST DisasterAlert Pico->Heltec");
        }
    }

    if(wifiConnected&&WiFi.status()!=WL_CONNECTED){
        wifiConnected=false; Serial.println("[WIFI] Lost!");
    }

    if(wifiConnected&&(millis()-lastFetchTime>=FETCH_INTERVAL_MS)){
        Serial.println("[FETCH] Periodic...");
        showFetching(); for(int i=0;i<30;i++) refreshDisplay();
        fetchUSGS(); lastFetchTime=millis();
    }

    unsigned long now=millis();
    if(queueCount>0){
        if(!showingAlert||(now-lastDisplayChange>=DISPLAY_DURATION_MS)){
            if(getFromQueue(&currentEvent)){
                showAlert(&currentEvent);
                showingAlert=true; lastDisplayChange=now;
                Serial.printf("[DISPLAY] M%.1f %s\n",
                              currentEvent.magnitude,currentEvent.location);
            }
        }
    } else {
        if(showingAlert||(now-lastDisplayChange>=5000)){
            showNoAlerts(); showingAlert=false; lastDisplayChange=now;
        }
    }
}


 
*/









/*
 //Pico 2 W + Seengreat HUB75 Test
 // 
 // Simple test to verify HUB75 display works
 //Shows "HELLO" and cycles colors
 


 #include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "hub75_driver.h"
#include "event_tracker.h"
#include "json_parser.h"
#include "display_renderer.h"
#include <Arduino.h>


// Pico 2 W + Seengreat HUB75 Test
 //
 //Simple test to verify HUB75 display works
 //Shows "HELLO" and cycles colors
 

#include <Arduino.h>

// ==================== SEENGREAT PIN MAPPING ====================
#define PIN_R1   2
#define PIN_G1   3
#define PIN_B1   4
#define PIN_R2   5
#define PIN_G2   8
#define PIN_B2   9
#define PIN_A    10
#define PIN_B    16
#define PIN_C    18
#define PIN_D    20
#define PIN_E    22
#define PIN_CLK  11
#define PIN_LAT  12
#define PIN_OE   13

// Panel size
#define WIDTH  64
#define HEIGHT 32
#define ROWS   (HEIGHT / 2)  // 16 rows (top and bottom scanned together)

// Framebuffer: RGB for each pixel
uint8_t framebuffer[WIDTH * HEIGHT * 3];

// ==================== 5x7 FONT ====================
const uint8_t font5x7[] = {
    // Space
    0x00, 0x00, 0x00, 0x00, 0x00,
    // ! " # $ % & ' ( ) * + , - . /
    0x00, 0x00, 0x5F, 0x00, 0x00,  // !
    0x00, 0x07, 0x00, 0x07, 0x00,  // "
    0x14, 0x7F, 0x14, 0x7F, 0x14,  // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12,  // $
    0x23, 0x13, 0x08, 0x64, 0x62,  // %
    0x36, 0x49, 0x55, 0x22, 0x50,  // &
    0x00, 0x05, 0x03, 0x00, 0x00,  // '
    0x00, 0x1C, 0x22, 0x41, 0x00,  // (
    0x00, 0x41, 0x22, 0x1C, 0x00,  // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08,  // *
    0x08, 0x08, 0x3E, 0x08, 0x08,  // +
    0x00, 0x50, 0x30, 0x00, 0x00,  // ,
    0x08, 0x08, 0x08, 0x08, 0x08,  // -
    0x00, 0x60, 0x60, 0x00, 0x00,  // .
    0x20, 0x10, 0x08, 0x04, 0x02,  // /
    // 0-9
    0x3E, 0x51, 0x49, 0x45, 0x3E,  // 0
    0x00, 0x42, 0x7F, 0x40, 0x00,  // 1
    0x42, 0x61, 0x51, 0x49, 0x46,  // 2
    0x21, 0x41, 0x45, 0x4B, 0x31,  // 3
    0x18, 0x14, 0x12, 0x7F, 0x10,  // 4
    0x27, 0x45, 0x45, 0x45, 0x39,  // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30,  // 6
    0x01, 0x71, 0x09, 0x05, 0x03,  // 7
    0x36, 0x49, 0x49, 0x49, 0x36,  // 8
    0x06, 0x49, 0x49, 0x29, 0x1E,  // 9
    // : ; < = > ? @
    0x00, 0x36, 0x36, 0x00, 0x00,  // :
    0x00, 0x56, 0x36, 0x00, 0x00,  // ;
    0x00, 0x08, 0x14, 0x22, 0x41,  // <
    0x14, 0x14, 0x14, 0x14, 0x14,  // =
    0x41, 0x22, 0x14, 0x08, 0x00,  // >
    0x02, 0x01, 0x51, 0x09, 0x06,  // ?
    0x32, 0x49, 0x79, 0x41, 0x3E,  // @
    // A-Z
    0x7E, 0x11, 0x11, 0x11, 0x7E,  // A
    0x7F, 0x49, 0x49, 0x49, 0x36,  // B
    0x3E, 0x41, 0x41, 0x41, 0x22,  // C
    0x7F, 0x41, 0x41, 0x22, 0x1C,  // D
    0x7F, 0x49, 0x49, 0x49, 0x41,  // E
    0x7F, 0x09, 0x09, 0x01, 0x01,  // F
    0x3E, 0x41, 0x41, 0x51, 0x32,  // G
    0x7F, 0x08, 0x08, 0x08, 0x7F,  // H
    0x00, 0x41, 0x7F, 0x41, 0x00,  // I
    0x20, 0x40, 0x41, 0x3F, 0x01,  // J
    0x7F, 0x08, 0x14, 0x22, 0x41,  // K
    0x7F, 0x40, 0x40, 0x40, 0x40,  // L
    0x7F, 0x02, 0x04, 0x02, 0x7F,  // M
    0x7F, 0x04, 0x08, 0x10, 0x7F,  // N
    0x3E, 0x41, 0x41, 0x41, 0x3E,  // O
    0x7F, 0x09, 0x09, 0x09, 0x06,  // P
    0x3E, 0x41, 0x51, 0x21, 0x5E,  // Q
    0x7F, 0x09, 0x19, 0x29, 0x46,  // R
    0x46, 0x49, 0x49, 0x49, 0x31,  // S
    0x01, 0x01, 0x7F, 0x01, 0x01,  // T
    0x3F, 0x40, 0x40, 0x40, 0x3F,  // U
    0x1F, 0x20, 0x40, 0x20, 0x1F,  // V
    0x7F, 0x20, 0x18, 0x20, 0x7F,  // W
    0x63, 0x14, 0x08, 0x14, 0x63,  // X
    0x03, 0x04, 0x78, 0x04, 0x03,  // Y
    0x61, 0x51, 0x49, 0x45, 0x43,  // Z
};

// ==================== DISPLAY FUNCTIONS ====================

void initPins() {
    pinMode(PIN_R1, OUTPUT);
    pinMode(PIN_G1, OUTPUT);
    pinMode(PIN_B1, OUTPUT);
    pinMode(PIN_R2, OUTPUT);
    pinMode(PIN_G2, OUTPUT);
    pinMode(PIN_B2, OUTPUT);
    pinMode(PIN_A, OUTPUT);
    pinMode(PIN_B, OUTPUT);
    pinMode(PIN_C, OUTPUT);
    pinMode(PIN_D, OUTPUT);
    pinMode(PIN_E, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_LAT, OUTPUT);
    pinMode(PIN_OE, OUTPUT);
    
    digitalWrite(PIN_OE, HIGH);  // Display off
    digitalWrite(PIN_LAT, LOW);
    digitalWrite(PIN_CLK, LOW);
}

void setRowAddress(int row) {
    digitalWrite(PIN_A, row & 1);
    digitalWrite(PIN_B, (row >> 1) & 1);
    digitalWrite(PIN_C, (row >> 2) & 1);
    digitalWrite(PIN_D, (row >> 3) & 1);
    // PIN_E for 64x64 panels only
}

void clockPulse() {
    digitalWrite(PIN_CLK, HIGH);
    digitalWrite(PIN_CLK, LOW);
}

void latchData() {
    digitalWrite(PIN_LAT, HIGH);
    digitalWrite(PIN_LAT, LOW);
}

void clearFramebuffer() {
    memset(framebuffer, 0, sizeof(framebuffer));
}

void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    int idx = (y * WIDTH + x) * 3;
    framebuffer[idx + 0] = r;
    framebuffer[idx + 1] = g;
    framebuffer[idx + 2] = b;
}

void fillRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            setPixel(x + i, y + j, r, g, b);
        }
    }
}

void drawChar(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b) {
    if (c < 32 || c > 90) c = '?';
    int idx = (c - 32) * 5;
    
    for (int col = 0; col < 5; col++) {
        uint8_t line = font5x7[idx + col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                setPixel(x + col, y + row, r, g, b);
            }
        }
    }
}

void drawString(int x, int y, const char* str, uint8_t r, uint8_t g, uint8_t b) {
    while (*str) {
        drawChar(x, y, *str, r, g, b);
        x += 6;
        str++;
    }
}

// Refresh display - call this frequently!
void refreshDisplay() {
    for (int row = 0; row < ROWS; row++) {
        // Disable output during row switch
        digitalWrite(PIN_OE, HIGH);
        
        // Set row address
        setRowAddress(row);
        
        // Shift out pixel data for this row
        for (int x = 0; x < WIDTH; x++) {
            // Top half pixel
            int idxTop = (row * WIDTH + x) * 3;
            // Bottom half pixel
            int idxBot = ((row + ROWS) * WIDTH + x) * 3;
            
            // Get colors (threshold at 128 for 1-bit color)
            digitalWrite(PIN_R1, framebuffer[idxTop + 0] > 128);
            digitalWrite(PIN_G1, framebuffer[idxTop + 1] > 128);
            digitalWrite(PIN_B1, framebuffer[idxTop + 2] > 128);
            digitalWrite(PIN_R2, framebuffer[idxBot + 0] > 128);
            digitalWrite(PIN_G2, framebuffer[idxBot + 1] > 128);
            digitalWrite(PIN_B2, framebuffer[idxBot + 2] > 128);
            
            clockPulse();
        }
        
        // Latch the data
        latchData();
        
        // Enable output
        digitalWrite(PIN_OE, LOW);
        
        // Hold row active (adjust for brightness)
        delayMicroseconds(100);
    }
}

// ==================== SETUP & LOOP ====================

unsigned long lastColorChange = 0;
int colorIndex = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("================================");
    Serial.println("  HUB75 Test - Seengreat Adapter");
    Serial.println("================================");
    Serial.println();
    Serial.println("Pin Configuration:");
    Serial.printf("  R1=%d G1=%d B1=%d\n", PIN_R1, PIN_G1, PIN_B1);
    Serial.printf("  R2=%d G2=%d B2=%d\n", PIN_R2, PIN_G2, PIN_B2);
    Serial.printf("  A=%d B=%d C=%d D=%d E=%d\n", PIN_A, PIN_B, PIN_C, PIN_D, PIN_E);
    Serial.printf("  CLK=%d LAT=%d OE=%d\n", PIN_CLK, PIN_LAT, PIN_OE);
    Serial.println();
    
    // Initialize pins
    initPins();
    Serial.println("Pins initialized");
    
    // Clear display
    clearFramebuffer();
    Serial.println("Framebuffer cleared");
    
    Serial.println();
    Serial.println("Display should show 'HELLO' now!");
    Serial.println("Colors will cycle every 2 seconds.");
    Serial.println();
}

void loop() {
    // Change color every 2 seconds
    if (millis() - lastColorChange > 2000) {
        lastColorChange = millis();
        colorIndex = (colorIndex + 1) % 7;
        
        // Clear and redraw
        clearFramebuffer();
        
        uint8_t r = 0, g = 0, b = 0;
        const char* colorName = "";
        
        switch (colorIndex) {
            case 0: r = 255; g = 0;   b = 0;   colorName = "RED";     break;
            case 1: r = 0;   g = 255; b = 0;   colorName = "GREEN";   break;
            case 2: r = 0;   g = 0;   b = 255; colorName = "BLUE";    break;
            case 3: r = 255; g = 255; b = 0;   colorName = "YELLOW";  break;
            case 4: r = 0;   g = 255; b = 255; colorName = "CYAN";    break;
            case 5: r = 255; g = 0;   b = 255; colorName = "MAGENTA"; break;
            case 6: r = 255; g = 255; b = 255; colorName = "WHITE";   break;
        }
        
        // Draw "HELLO" in top half (rows 0-15)
        drawString(17, 4, "HELLO", r, g, b);
        
        // Draw "PICO2" in bottom half (rows 16-31)
        // Must start at row 16 or below to be fully in bottom half
        drawString(17, 20, "PICO2", r, g, b);
        
        // Draw border
        for (int x = 0; x < WIDTH; x++) {
            setPixel(x, 0, r/2, g/2, b/2);
            setPixel(x, HEIGHT-1, r/2, g/2, b/2);
        }
        for (int y = 0; y < HEIGHT; y++) {
            setPixel(0, y, r/2, g/2, b/2);
            setPixel(WIDTH-1, y, r/2, g/2, b/2);
        }
        
        Serial.printf("Color: %s\n", colorName);
    }
    
    // Continuously refresh the display
    refreshDisplay();
}

*/