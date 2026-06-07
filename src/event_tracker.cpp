/*
 * event_tracker.cpp - Event Tracking Implementation
 */

#include "event_tracker.h"
#include <EEPROM.h>
#include <string.h>

// ==================== EEPROM Structure ====================

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint16_t count;
    uint16_t next_index;
    char event_ids[MAX_TRACKED_EVENTS][MAX_EVENT_ID_LEN];
} eeprom_data_t;

// ==================== Static Variables ====================

static char seen_event_ids[MAX_TRACKED_EVENTS][MAX_EVENT_ID_LEN];
static int seen_event_count = 0;
static int seen_event_index = 0;

// Display queue
static disaster_event_t display_queue[MAX_DISPLAY_QUEUE];
static int queue_head = 0;
static int queue_tail = 0;
static int queue_count = 0;

// ==================== EEPROM Functions ====================

static void load_from_eeprom(void) {
    EEPROM.begin(EEPROM_SIZE);
    
    uint8_t magic = EEPROM.read(0);
    uint8_t version = EEPROM.read(1);
    
    if (magic != EEPROM_MAGIC || version != EEPROM_VERSION) {
        Serial.println("[TRACKER] No valid EEPROM data, starting fresh");
        seen_event_count = 0;
        seen_event_index = 0;
        return;
    }
    
    seen_event_count = EEPROM.read(2) | (EEPROM.read(3) << 8);
    seen_event_index = EEPROM.read(4) | (EEPROM.read(5) << 8);
    
    if (seen_event_count > MAX_TRACKED_EVENTS) {
        seen_event_count = MAX_TRACKED_EVENTS;
    }
    if (seen_event_index >= MAX_TRACKED_EVENTS) {
        seen_event_index = 0;
    }
    
    // Load event IDs
    int addr = 6;
    for (int i = 0; i < seen_event_count && i < MAX_TRACKED_EVENTS; i++) {
        for (int j = 0; j < MAX_EVENT_ID_LEN; j++) {
            seen_event_ids[i][j] = EEPROM.read(addr++);
        }
    }
    
    Serial.printf("[TRACKER] Loaded %d seen events from EEPROM\n", seen_event_count);
}

void save_seen_events(void) {
    EEPROM.begin(EEPROM_SIZE);
    
    EEPROM.write(0, EEPROM_MAGIC);
    EEPROM.write(1, EEPROM_VERSION);
    EEPROM.write(2, seen_event_count & 0xFF);
    EEPROM.write(3, (seen_event_count >> 8) & 0xFF);
    EEPROM.write(4, seen_event_index & 0xFF);
    EEPROM.write(5, (seen_event_index >> 8) & 0xFF);
    
    int addr = 6;
    for (int i = 0; i < seen_event_count && i < MAX_TRACKED_EVENTS; i++) {
        for (int j = 0; j < MAX_EVENT_ID_LEN; j++) {
            EEPROM.write(addr++, seen_event_ids[i][j]);
        }
    }
    
    EEPROM.commit();
    Serial.printf("[TRACKER] Saved %d events to EEPROM\n", seen_event_count);
}

// ==================== Public Functions ====================

void event_tracker_init(void) {
    memset(seen_event_ids, 0, sizeof(seen_event_ids));
    memset(display_queue, 0, sizeof(display_queue));
    
    seen_event_count = 0;
    seen_event_index = 0;
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
    
    load_from_eeprom();
}

bool is_event_seen(const char *event_id) {
    if (!event_id || event_id[0] == '\0') return true;
    
    for (int i = 0; i < seen_event_count; i++) {
        if (strcmp(seen_event_ids[i], event_id) == 0) {
            return true;
        }
    }
    return false;
}

void mark_event_seen(const char *event_id) {
    if (!event_id || event_id[0] == '\0') return;
    if (is_event_seen(event_id)) return;
    
    // Add to circular buffer
    strncpy(seen_event_ids[seen_event_index], event_id, MAX_EVENT_ID_LEN - 1);
    seen_event_ids[seen_event_index][MAX_EVENT_ID_LEN - 1] = '\0';
    
    seen_event_index = (seen_event_index + 1) % MAX_TRACKED_EVENTS;
    
    if (seen_event_count < MAX_TRACKED_EVENTS) {
        seen_event_count++;
    }
    
    Serial.printf("[TRACKER] Marked seen: %s (total: %d)\n", event_id, seen_event_count);
    
    // Save every 5 events
    static int save_counter = 0;
    if (++save_counter >= 5) {
        save_seen_events();
        save_counter = 0;
    }
}

bool add_event_to_queue(disaster_event_t *event) {
    if (!event || event->id[0] == '\0') return false;
    
    if (is_event_seen(event->id)) {
        return false;
    }
    
    if (queue_count >= MAX_DISPLAY_QUEUE) {
        Serial.println("[TRACKER] Queue full, dropping oldest");
        queue_head = (queue_head + 1) % MAX_DISPLAY_QUEUE;
        queue_count--;
    }
    
    event->is_new = true;
    memcpy(&display_queue[queue_tail], event, sizeof(disaster_event_t));
    queue_tail = (queue_tail + 1) % MAX_DISPLAY_QUEUE;
    queue_count++;
    
    Serial.printf("[TRACKER] Queued: %s - %s (queue: %d)\n",
                  event->type, event->title, queue_count);
    
    return true;
}

bool get_next_event_from_queue(disaster_event_t *event) {
    if (queue_count == 0 || !event) return false;
    
    memcpy(event, &display_queue[queue_head], sizeof(disaster_event_t));
    queue_head = (queue_head + 1) % MAX_DISPLAY_QUEUE;
    queue_count--;
    
    mark_event_seen(event->id);
    
    return true;
}

int get_display_queue_count(void) {
    return queue_count;
}

void clear_display_queue(void) {
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
}

int get_seen_event_count(void) {
    return seen_event_count;
}

void clear_all_seen_events(void) {
    memset(seen_event_ids, 0, sizeof(seen_event_ids));
    seen_event_count = 0;
    seen_event_index = 0;
    save_seen_events();
    Serial.println("[TRACKER] Cleared all seen events");
}

const char* get_event_type_name(const char *code) {
    if (!code) return "ALERT";
    
    if (strcmp(code, "EQ") == 0) return "QUAKE";
    if (strcmp(code, "TC") == 0) return "CYCLONE";
    if (strcmp(code, "VO") == 0) return "VOLCANO";
    if (strcmp(code, "FL") == 0) return "FLOOD";
    if (strcmp(code, "DR") == 0) return "DROUGHT";
    if (strcmp(code, "WF") == 0) return "FIRE";
    if (strcmp(code, "LS") == 0) return "SLIDE";
    if (strcmp(code, "CW") == 0) return "C.WAVE";
    if (strcmp(code, "EP") == 0) return "EPIDEMIC";
    
    return "ALERT";
}

rgb_t get_alert_color(uint8_t alert_level) {
    switch (alert_level) {
        case 2:  return RGB_RED;
        case 1:  return RGB_ORANGE;
        default: return RGB_GREEN;
    }
}
