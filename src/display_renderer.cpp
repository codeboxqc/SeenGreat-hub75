/*
 * display_renderer.cpp - Display Rendering Implementation
 */

#include "display_renderer.h"
#include <string.h>

// ==================== Display Functions ====================

void display_startup_screen(void) {
    hub75_clear();
    
    // Title
    hub75_draw_string(4, 2, "Disaster", RGB_CYAN, 1);
    hub75_draw_string(4, 11, "Alert", RGB_CYAN, 1);
    
    // Board name
    hub75_draw_string(4, 22, BOARD_NAME, RGB_DARK_GRAY, 1);
    
    hub75_swap_buffers();
}

void display_status(const char *line1, const char *line2) {
    hub75_clear();
    
    if (line1) {
        hub75_draw_string(2, 8, line1, RGB_CYAN, 1);
    }
    if (line2) {
        hub75_draw_string(2, 18, line2, RGB_YELLOW, 1);
    }
    
    hub75_swap_buffers();
}

void display_error(const char *line1, const char *line2) {
    hub75_clear();
    
    // Red border
    hub75_draw_rect(0, 0, TOTAL_WIDTH, TOTAL_HEIGHT, RGB_RED);
    
    if (line1) {
        hub75_draw_string(4, 8, line1, RGB_RED, 1);
    }
    if (line2) {
        hub75_draw_string(4, 18, line2, RGB_WHITE, 1);
    }
    
    hub75_swap_buffers();
}

void display_wifi_connecting(int progress) {
    hub75_clear();
    
    hub75_draw_string(2, 4, "Connecting", RGB_CYAN, 1);
    hub75_draw_string(2, 14, "WiFi", RGB_CYAN, 1);
    
    // Progress dots
    int dots = (progress / 10) % 6;
    for (int i = 0; i < dots; i++) {
        hub75_fill_circle(8 + i * 10, 26, 2, RGB_YELLOW);
    }
    
    hub75_swap_buffers();
}

void draw_event_icon(const char *type, int x, int y) {
    rgb_t color = RGB_YELLOW;
    
    if (strcmp(type, "EQ") == 0) {
        // Earthquake - zigzag seismograph
        color = RGB_ORANGE;
        hub75_draw_line(x, y+4, x+2, y+1, color);
        hub75_draw_line(x+2, y+1, x+4, y+6, color);
        hub75_draw_line(x+4, y+6, x+6, y+2, color);
        hub75_draw_line(x+6, y+2, x+8, y+4, color);
        
    } else if (strcmp(type, "TC") == 0) {
        // Tropical Cyclone - spiral
        color = RGB_CYAN;
        hub75_draw_circle(x+4, y+4, 3, color);
        hub75_set_pixel(x+4, y+4, color);
        hub75_set_pixel(x+3, y+2, color);
        hub75_set_pixel(x+5, y+6, color);
        
    } else if (strcmp(type, "VO") == 0) {
        // Volcano - triangle
        color = RGB_RED;
        hub75_draw_triangle(x+4, y, x, y+7, x+8, y+7, color);
        hub75_fill_rect(x+3, y+2, 3, 2, RGB_ORANGE);
        
    } else if (strcmp(type, "FL") == 0) {
        // Flood - waves
        color = RGB_BLUE;
        for (int i = 0; i < 3; i++) {
            int bx = x + i * 3;
            hub75_set_pixel(bx, y+4, color);
            hub75_set_pixel(bx+1, y+3, color);
            hub75_set_pixel(bx+2, y+4, color);
        }
        hub75_draw_hline(x, y+6, 9, color);
        
    } else if (strcmp(type, "WF") == 0) {
        // Wildfire - flame
        color = RGB_ORANGE;
        hub75_set_pixel(x+4, y, color);
        hub75_draw_vline(x+3, y+1, 3, color);
        hub75_draw_vline(x+4, y+1, 4, RGB_RED);
        hub75_draw_vline(x+5, y+2, 3, color);
        hub75_draw_hline(x+2, y+5, 5, RGB_YELLOW);
        
    } else if (strcmp(type, "DR") == 0) {
        // Drought - cracked earth
        color = RGB_YELLOW;
        hub75_draw_hline(x, y+6, 9, color);
        hub75_draw_line(x+2, y+6, x+4, y+2, color);
        hub75_draw_line(x+4, y+2, x+6, y+6, color);
        
    } else {
        // Generic - exclamation mark
        hub75_fill_rect(x+3, y, 2, 5, RGB_YELLOW);
        hub75_fill_rect(x+3, y+6, 2, 2, RGB_YELLOW);
    }
}

void display_next_alert(void) {
    disaster_event_t event;
    
    if (!get_next_event_from_queue(&event)) {
        return;
    }
    
    Serial.printf("[DISPLAY] Showing: %s - %s\n", event.type, event.title);
    
    hub75_clear();
    
    // Alert level bar at top
    rgb_t alert_color = get_alert_color(event.alert_level);
    hub75_fill_rect(0, 0, TOTAL_WIDTH, 3, alert_color);
    
    // Event type icon
    draw_event_icon(event.type, 2, 5);
    
    // Event type name
    hub75_draw_string(14, 5, get_event_type_name(event.type), alert_color, 1);
    
    // Magnitude if applicable
    if (event.magnitude > 0) {
        char mag_str[8];
        snprintf(mag_str, sizeof(mag_str), "%.1f", event.magnitude);
        hub75_draw_string(TOTAL_WIDTH - 18, 5, mag_str, RGB_YELLOW, 1);
    }
    
    // Country (truncated)
    char country_short[11];
    strncpy(country_short, event.country, 10);
    country_short[10] = '\0';
    hub75_draw_string(2, 14, country_short, RGB_CYAN, 1);
    
    // Title (truncated)
    char title_short[11];
    strncpy(title_short, event.title, 10);
    title_short[10] = '\0';
    hub75_draw_string(2, 23, title_short, RGB_WHITE, 1);
    
    // NEW indicator
    if (event.is_new) {
        hub75_draw_string(TOTAL_WIDTH - 20, 23, "NEW", RGB_RED, 1);
    }
    
    hub75_swap_buffers();
}

void display_idle_screen(void) {
    static unsigned long frame = 0;
    frame++;
    
    hub75_clear();
    
    // Border
    hub75_draw_rect(0, 0, TOTAL_WIDTH, TOTAL_HEIGHT, RGB_GREEN);
    
    // Status
    hub75_draw_string(4, 4, "Monitoring", RGB_GREEN, 1);
    hub75_draw_string(4, 14, "No New", RGB_CYAN, 1);
    hub75_draw_string(4, 22, "Alerts", RGB_CYAN, 1);
    
    // Heartbeat dot
    if ((frame / 50) % 2 == 0) {
        hub75_fill_circle(TOTAL_WIDTH - 8, TOTAL_HEIGHT - 6, 2, RGB_GREEN);
    }
    
    // Event count
    char count_str[8];
    snprintf(count_str, sizeof(count_str), "%d", get_seen_event_count());
    hub75_draw_string(TOTAL_WIDTH - 12, 4, count_str, RGB_DARK_GRAY, 1);
    
    hub75_swap_buffers();
}

void display_update(void) {
    hub75_refresh();
}
