/*
 * gps.h - NEO-6M GPS Engine Interface
 */


 /*
#ifndef GPS_ENGINE_H
#define GPS_ENGINE_H

#include <Arduino.h>

// Consolidated structure for all GPS telemetry
typedef struct {
    double latitude;
    double longitude;
    float speed_kmh;
    float altitude_m;
    uint32_t satellites;
    bool has_fix;
    
    // Time data (UTC)
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} gps_data_t;

// Engine Functions
void gps_init(uint8_t tx_pin, uint8_t rx_pin);
void gps_update(void);
gps_data_t gps_get_data(void);
void gps_debug(void);

#endif // GPS_ENGINE_H

*/