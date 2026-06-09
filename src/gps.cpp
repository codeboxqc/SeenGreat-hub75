/*
 * gps.cpp - NEO-6M GPS Engine Implementation
 */


 /*
#include "gps.h"
#include <TinyGPS++.h>
 

// Internal state
static TinyGPSPlus neo6m;
static gps_data_t current_gps_data = {0};


void gps_init(uint8_t tx_pin, uint8_t rx_pin) {
    Serial1.setTX(tx_pin);
    Serial1.setRX(rx_pin);
    
    // Try the common 9600 baud first
    Serial1.begin(9600);
    delay(100);
    
    // If that fails, the module might be set to 4800 (common in older units)
    if (!Serial1.available()) {
        Serial1.end();
        Serial1.begin(4800);
    }
}

void gps_update(void) {

    int bytes_read = 0;
    while (Serial1.available() > 0) {
        neo6m.encode(Serial1.read());
        bytes_read++;
    }

    // Satellite count: update unconditionally whenever valid
    if (neo6m.satellites.isValid()) {
        current_gps_data.satellites = neo6m.satellites.value();
    }

    if (neo6m.location.isUpdated() || neo6m.time.isUpdated()) {
        current_gps_data.has_fix = neo6m.location.isValid();
        if (current_gps_data.has_fix) {
            current_gps_data.latitude   = neo6m.location.lat();
            current_gps_data.longitude  = neo6m.location.lng();
            current_gps_data.speed_kmh  = neo6m.speed.kmph();
            current_gps_data.altitude_m = neo6m.altitude.meters();
        }
        if (neo6m.time.isValid()) {
            current_gps_data.hour   = neo6m.time.hour();
            current_gps_data.minute = neo6m.time.minute();
            current_gps_data.second = neo6m.time.second();
        }
    }
}

gps_data_t gps_get_data(void) {
    // Return a copy of the latest locked data
    return current_gps_data;
}


// Add this to the end of gps.cpp
void gps_debug(void) {
    // Check if the serial buffer has data
    if (Serial1.available()) {
        Serial.print("GPS Data Detected: ");
        Serial.println(Serial1.available());
    }
    
    // Print internal status of the TinyGPS object
    Serial.print("Sats: ");
    Serial.print(current_gps_data.satellites);
    Serial.print(" | Has Fix: ");
    Serial.println(current_gps_data.has_fix ? "YES" : "NO");
    
    // Print raw location data if available
    if (current_gps_data.has_fix) {
        Serial.print("Lat: "); Serial.print(current_gps_data.latitude, 6);
        Serial.print(" Lon: "); Serial.println(current_gps_data.longitude, 6);
    }
}

*/