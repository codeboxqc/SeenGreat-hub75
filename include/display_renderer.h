/*
 * display_renderer.h - Display Rendering Functions
 * 
 * Renders alerts and status screens on the HUB75 display
 */

#ifndef DISPLAY_RENDERER_H
#define DISPLAY_RENDERER_H

#include <Arduino.h>
#include "config.h"
#include "hub75_driver.h"
#include "event_tracker.h"

/**
 * Display startup splash screen
 */
void display_startup_screen(void);

/**
 * Display a status message (2 lines)
 */
void display_status(const char *line1, const char *line2);

/**
 * Display an error message (2 lines, red border)
 */
void display_error(const char *line1, const char *line2);

/**
 * Display WiFi connecting animation
 */
void display_wifi_connecting(int progress);

/**
 * Display the next alert from queue
 */
void display_next_alert(void);

/**
 * Display idle/monitoring screen
 */
void display_idle_screen(void);

/**
 * Draw event type icon
 */
void draw_event_icon(const char *type, int x, int y);

/**
 * Update display (call in main loop)
 */
void display_update(void);

#endif // DISPLAY_RENDERER_H
