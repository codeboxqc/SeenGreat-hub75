/*
 * config.h - Configuration for Pico 2 Disaster Alert Display
 * 
 * ╔═══════════════════════════════════════════════════════════════════╗
 * ║  EASY PIN CONFIGURATION - Change board type in platformio.ini     ║
 * ║  or uncomment the appropriate BOARD_* define below                ║
 * ╚═══════════════════════════════════════════════════════════════════╝
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

 

 
// ==================== Display Configuration ====================

#define PANEL_WIDTH     64      // Panel width in pixels
#define PANEL_HEIGHT    64      // Panel height (32 or 64)
#define PANEL_CHAIN     1       // Number of chained panels
#define COLOR_DEPTH     4       // Bits per color channel (1-8)
#define DEFAULT_BRIGHTNESS 128  // Default brightness (0-255)

// Calculate total dimensions
#define TOTAL_WIDTH     (PANEL_WIDTH * PANEL_CHAIN)
#define TOTAL_HEIGHT    PANEL_HEIGHT

// ==================== PIN MAPPING ====================
/*
 * ╔═══════════════════════════════════════════════════════════════════╗
 * ║  SEENGREAT RGB MATRIX ADAPTER - PICO PINOUT                       ║
 * ╠═══════════════════════════════════════════════════════════════════╣
 * ║  Signal  │  Pico Pin  │  Description                              ║
 * ╠══════════╪════════════╪═══════════════════════════════════════════╣
 * ║  R1      │  GP2       │  Red upper half                           ║
 * ║  G1      │  GP3       │  Green upper half                         ║
 * ║  B1      │  GP4       │  Blue upper half                          ║
 * ║  R2      │  GP5       │  Red lower half                           ║
 * ║  G2      │  GP8       │  Green lower half                         ║
 * ║  B2      │  GP9       │  Blue lower half                          ║
 * ║  A       │  GP10      │  Row address bit A                        ║
 * ║  B       │  GP16      │  Row address bit B                        ║
 * ║  C       │  GP18      │  Row address bit C                        ║
 * ║  D       │  GP20      │  Row address bit D                        ║
 * ║  E       │  GP22      │  Row address bit E (64x64 panels)         ║
 * ║  CLK     │  GP11      │  Pixel clock                              ║
 * ║  LAT     │  GP12      │  Latch/strobe                             ║
 * ║  OE      │  GP13      │  Output enable (active low)               ║
 * ╚══════════╧════════════╧═══════════════════════════════════════════╝
 */

// ---------- Seengreat Adapter Board (Default) ----------
#if defined(BOARD_SEENGREAT) || (!defined(BOARD_INTERSTATE75) && !defined(BOARD_ADAFRUIT_PORTAL) && !defined(BOARD_CUSTOM))
    #define PIN_R1      2
    #define PIN_G1      3
    #define PIN_B1      4
    #define PIN_R2      5
    #define PIN_G2      8
    #define PIN_B2      9
    #define PIN_A       10
    #define PIN_B       16
    #define PIN_C       18
    #define PIN_D       20
    #define PIN_E       22
    #define PIN_CLK     11
    #define PIN_LAT     12
    #define PIN_OE      13
    //#define BOARD_NAME  "Seengreat"
#endif

// ---------- Pimoroni Interstate 75 ----------
#ifdef BOARD_INTERSTATE75
    #define PIN_R1      0
    #define PIN_G1      1
    #define PIN_B1      2
    #define PIN_R2      3
    #define PIN_G2      4
    #define PIN_B2      5
    #define PIN_A       6
    #define PIN_B       7
    #define PIN_C       8
    #define PIN_D       9
    #define PIN_E       10
    #define PIN_CLK     11
    #define PIN_LAT     12
    #define PIN_OE      13
    #define BOARD_NAME  "Interstate75"
#endif

// ---------- Custom Board - EDIT THESE! ----------
#ifdef BOARD_CUSTOM
    #define PIN_R1      2
    #define PIN_G1      3
    #define PIN_B1      4
    #define PIN_R2      5
    #define PIN_G2      6
    #define PIN_B2      7
    #define PIN_A       10
    #define PIN_B       11
    #define PIN_C       12
    #define PIN_D       13
    #define PIN_E       14
    #define PIN_CLK     15
    #define PIN_LAT     16
    #define PIN_OE      17
    #define BOARD_NAME  "Custom"
#endif

#ifndef BOARD_NAME
    #define BOARD_NAME "Seengreat"
#endif

// ==================== Row Address Bits ====================

#if PANEL_HEIGHT == 64
    #define ADDR_BITS   5
#else
    #define ADDR_BITS   4
#endif

#define ROWS_PER_SCAN   (PANEL_HEIGHT / 2)

 
 

// ==================== EEPROM Storage ====================

#define EEPROM_SIZE         2048
#define EEPROM_MAGIC        0xDA
#define EEPROM_VERSION      1

// ==================== Debug Options ====================

#define DEBUG_SERIAL        1

#if DEBUG_SERIAL
    #define DBG_PRINT(...)      Serial.printf(__VA_ARGS__)
    #define DBG_PRINTLN(msg)    Serial.println(msg)
#else
    #define DBG_PRINT(...)      ((void)0)
    #define DBG_PRINTLN(msg)    ((void)0)
#endif

#endif // CONFIG_H
