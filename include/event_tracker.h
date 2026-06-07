/*
 * event_tracker.h - Event Tracking and Deduplication
 * 
 * Tracks seen events in EEPROM to persist across reboots
 * Only shows NEW events on the display
 */

#ifndef EVENT_TRACKER_H
#define EVENT_TRACKER_H

#include <Arduino.h>
#include "config.h"
#include "hub75_driver.h"

// ==================== Event Structure ====================

typedef struct {
    char id[MAX_EVENT_ID_LEN];      // Unique event identifier
    char type[MAX_TYPE_LEN];        // Event type code (EQ, TC, VO, FL, etc.)
    char title[MAX_TITLE_LEN];      // Event title/description
    char country[MAX_COUNTRY_LEN];  // Country/location
    float magnitude;                 // Magnitude (for earthquakes)
    float latitude;                  // Location latitude
    float longitude;                 // Location longitude
    uint8_t alert_level;            // 0=Green, 1=Orange, 2=Red
    bool is_new;                     // Flag for display purposes
} disaster_event_t;

// ==================== Function Prototypes ====================

/**
 * Initialize the event tracker
 * Loads previously seen events from EEPROM
 */
void event_tracker_init(void);

/**
 * Check if an event has been seen before
 */
bool is_event_seen(const char *event_id);

/**
 * Mark an event as seen (saves to EEPROM)
 */
void mark_event_seen(const char *event_id);

/**
 * Add an event to the display queue
 * Returns true if event was new and added
 */
bool add_event_to_queue(disaster_event_t *event);

/**
 * Get the next event from display queue
 * Returns true if an event was available
 */
bool get_next_event_from_queue(disaster_event_t *event);

/**
 * Get number of events in display queue
 */
int get_display_queue_count(void);

/**
 * Clear the display queue
 */
void clear_display_queue(void);

/**
 * Get number of tracked (seen) events
 */
int get_seen_event_count(void);

/**
 * Clear all seen events (reset)
 */
void clear_all_seen_events(void);

/**
 * Save seen events to EEPROM
 */
void save_seen_events(void);

/**
 * Get event type name from code
 */
const char* get_event_type_name(const char *code);

/**
 * Get alert color for display
 */
rgb_t get_alert_color(uint8_t alert_level);

#endif // EVENT_TRACKER_H
