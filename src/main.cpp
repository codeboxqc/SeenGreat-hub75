

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
const char* WIFI_SSID     = "";
const char* WIFI_PASSWORD = "";

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











 
