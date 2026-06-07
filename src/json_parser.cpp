/*
 * json_parser.cpp - HTTP Fetcher and JSON Parser Implementation
 * 
 * Uses WiFi and HTTPClient for fetching
 * Uses ArduinoJson for parsing
 */

#include "json_parser.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ==================== HTTP Functions ====================

void http_init(void) {
    // Nothing special needed for Arduino HTTPClient
    Serial.println("[HTTP] HTTP client ready");
}

bool is_wifi_connected(void) {
    return WiFi.status() == WL_CONNECTED;
}

const char* get_wifi_status(void) {
    switch (WiFi.status()) {
        case WL_CONNECTED:    return "Connected";
        case WL_NO_SHIELD:    return "No WiFi";
        case WL_IDLE_STATUS:  return "Idle";
        case WL_NO_SSID_AVAIL: return "No SSID";
        case WL_SCAN_COMPLETED: return "Scan Done";
        case WL_CONNECT_FAILED: return "Failed";
        case WL_CONNECTION_LOST: return "Lost";
        case WL_DISCONNECTED: return "Disconnected";
        default:              return "Unknown";
    }
}

// ==================== USGS Parser ====================

int fetch_usgs_data(void) {
    if (!is_wifi_connected()) {
        Serial.println("[USGS] WiFi not connected");
        return 0;
    }
    
    Serial.println("[USGS] Fetching earthquake data...");
    
    HTTPClient http;
    http.begin(USGS_URL);
    http.setTimeout(HTTP_TIMEOUT_MS);
    
    int httpCode = http.GET();
    int newEvents = 0;
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("[USGS] Received %d bytes\n", payload.length());
        
        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.printf("[USGS] JSON error: %s\n", error.c_str());
        } else {
            JsonArray features = doc["features"];
            Serial.printf("[USGS] Found %d earthquakes\n", features.size());
            
            int count = 0;
            for (JsonObject feature : features) {
                if (++count > 5) break;  // Limit to 5
                
                disaster_event_t event = {0};
                strcpy(event.type, "EQ");
                
                // Get ID
                const char* id = feature["id"] | "";
                snprintf(event.id, sizeof(event.id), "usgs_%s", id);
                
                // Get properties
                JsonObject props = feature["properties"];
                
                event.magnitude = props["mag"] | 0.0f;
                
                const char* place = props["place"] | "Unknown";
                strncpy(event.title, place, sizeof(event.title) - 1);
                
                // Extract country from place
                const char* of = strstr(place, " of ");
                if (of) {
                    strncpy(event.country, of + 4, sizeof(event.country) - 1);
                } else {
                    strncpy(event.country, place, sizeof(event.country) - 1);
                }
                
                // Set alert level based on magnitude
                if (event.magnitude >= 7.0) event.alert_level = 2;
                else if (event.magnitude >= 5.5) event.alert_level = 1;
                else event.alert_level = 0;
                
                // Try to add to queue
                if (add_event_to_queue(&event)) {
                    newEvents++;
                    Serial.printf("[USGS] NEW: M%.1f - %s\n", 
                                  event.magnitude, event.title);
                }
            }
        }
    } else {
        Serial.printf("[USGS] HTTP error: %d\n", httpCode);
    }
    
    http.end();
    Serial.printf("[USGS] Found %d new events\n", newEvents);
    return newEvents;
}

// ==================== GDACS Parser ====================

int fetch_gdacs_data(void) {
    if (!is_wifi_connected()) {
        Serial.println("[GDACS] WiFi not connected");
        return 0;
    }
    
    Serial.println("[GDACS] Fetching disaster data...");
    
    HTTPClient http;
    http.begin(GDACS_URL);
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Accept", "application/json");
    
    int httpCode = http.GET();
    int newEvents = 0;
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("[GDACS] Received %d bytes\n", payload.length());
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.printf("[GDACS] JSON error: %s\n", error.c_str());
        } else {
            JsonArray features = doc["features"];
            Serial.printf("[GDACS] Found %d events\n", features.size());
            
            int count = 0;
            for (JsonObject feature : features) {
                if (++count > 5) break;
                
                disaster_event_t event = {0};
                
                JsonObject props = feature["properties"];
                
                // Get event ID
                int eventId = props["eventid"] | 0;
                snprintf(event.id, sizeof(event.id), "gdacs_%d", eventId);
                
                // Get event type
                const char* eventType = props["eventtype"] | "UNK";
                strncpy(event.type, eventType, sizeof(event.type) - 1);
                
                // Get name
                const char* name = props["name"] | "Unknown Event";
                strncpy(event.title, name, sizeof(event.title) - 1);
                
                // Get country
                const char* country = props["country"] | "Unknown";
                strncpy(event.country, country, sizeof(event.country) - 1);
                
                // Get alert level
                const char* alertLevel = props["alertlevel"] | "Green";
                if (strcmp(alertLevel, "Red") == 0) event.alert_level = 2;
                else if (strcmp(alertLevel, "Orange") == 0) event.alert_level = 1;
                else event.alert_level = 0;
                
                // Get severity/magnitude if available
                JsonObject severity = props["severitydata"];
                event.magnitude = severity["severity"] | 0.0f;
                
                // Try to add to queue
                if (add_event_to_queue(&event)) {
                    newEvents++;
                    Serial.printf("[GDACS] NEW: %s - %s (%s)\n",
                                  event.type, event.title, event.country);
                }
            }
        }
    } else {
        Serial.printf("[GDACS] HTTP error: %d\n", httpCode);
    }
    
    http.end();
    Serial.printf("[GDACS] Found %d new events\n", newEvents);
    return newEvents;
}

// ==================== Fetch All ====================

int fetch_all_data(void) {
    int total = 0;
    
    total += fetch_usgs_data();
    delay(2000);  // Rate limiting
    
    total += fetch_gdacs_data();
    
    Serial.printf("[FETCH] Total new events: %d\n", total);
    return total;
}
