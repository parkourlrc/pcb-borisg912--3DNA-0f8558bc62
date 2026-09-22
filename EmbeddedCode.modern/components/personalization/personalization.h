#pragma once

#include <stdint.h>

// =============================================================================
// COLOR DEFINITIONS
// =============================================================================

// RGB Color structure (0-255 per channel)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ColorRGB;

// DNA Base Colors (A-T-G-C)
#define COLOR_BASE_A        (ColorRGB){255,   0,   0}    // Red
#define COLOR_BASE_T        (ColorRGB){255, 255,   0}    // Yellow
#define COLOR_BASE_G        (ColorRGB){  0,   0, 255}    // Blue
#define COLOR_BASE_C        (ColorRGB){  0, 255,   0}    // Green
#define COLOR_BASE_UNKNOWN (ColorRGB){128,   0, 128}    // Purple

// Color Presets
#define COLOR_LIGHT_BLUE   (ColorRGB){100, 150, 255}
#define COLOR_DARK_BLUE    (ColorRGB){  0,   0, 128}
#define COLOR_LIGHT_RED    (ColorRGB){255, 100, 100}
#define COLOR_DARK_RED     (ColorRGB){128,   0,   0}
#define COLOR_ORANGE       (ColorRGB){255, 165,   0}
#define COLOR_LIGHT_GREEN  (ColorRGB){100, 255, 100}
#define COLOR_DARK_GREEN   (ColorRGB){  0, 128,   0}
#define COLOR_PINK         (ColorRGB){255, 192, 203}
#define COLOR_PURPLE       (ColorRGB){128,   0, 128}
#define COLOR_WHITE        (ColorRGB){255, 255, 255}

// =============================================================================
// SOUND EFFECT DEFINITIONS
// =============================================================================

// Sound effect track numbers (for DFPlayer Mini)
typedef enum {
    SOUND_SUCCESS = 1,      // Successful operation
    SOUND_FAIL = 2,         // Failed operation
    SOUND_BLE_PAIR = 3,     // Bluetooth pairing
    SOUND_BLE_SUCCESS = 4,  // Bluetooth connection success
    SOUND_BASE_CORRECT = 5, // Correct base pair
    SOUND_BASE_WRONG = 6,   // Wrong base pair
    SOUND_POWER_ON = 7,     // Power on sound
    SOUND_POWER_OFF = 8,    // Power off sound
    SOUND_ALERT = 9,        // Alert/notification
    SOUND_MAX            // Total number of sound effects
} SoundEffect;

// Get track number for a sound effect
uint16_t PersonalizationGetSoundTrack(SoundEffect effect);

// =============================================================================
// DISPLAY BRIGHTNESS PRESETS
// =============================================================================

#define BRIGHTNESS_OFF      0
#define BRIGHTNESS_MIN      10
#define BRIGHTNESS_LOW      50
#define BRIGHTNESS_MEDIUM   128
#define BRIGHTNESS_HIGH     200
#define BRIGHTNESS_MAX      255

// =============================================================================
// NEOPIXEL CONFIGURATION
// =============================================================================

// Helix NeoPixel configuration
#define NEOPIXEL_HELIX_BRIGHTNESS  BRIGHTNESS_MEDIUM
#define NEOPIXEL_BASE_BRIGHTNESS   BRIGHTNESS_MEDIUM

// Default color for helix when idle
#define NEOPIXEL_HELIX_DEFAULT     COLOR_WHITE
