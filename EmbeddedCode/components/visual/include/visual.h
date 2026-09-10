#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "led_strip.h"
#include "personalization.h"

// =============================================================================
// TYPE DEFINITIONS
// =============================================================================

// Light state for a position (color + brightness)
typedef struct {
    ColorRGB color;
    uint8_t brightness;  // 0-255
} LightState;

// Helix light configuration
typedef struct {
    LightState color;
    bool two_sided;  // true = light both sides, false = light one side
} HelixLightConfig;

// =============================================================================
// INITIALIZATION
// =============================================================================

// Initialize visual system
void VisualInit(void);

// Deinitialize visual system
void VisualDeinit(void);

// Update visual system (call periodically to update LEDs)
void VisualUpdate(void);

// =============================================================================
// HELIX NEOPIXEL CONTROL
// =============================================================================

// Set color for the entire helix strip
void VisualSetHelixColor(ColorRGB color, uint8_t brightness);

// Set color for a specific position in the helix
void VisualSetHelixPosition(uint16_t position, ColorRGB color, uint8_t brightness);

// Set two-sided mode for helix (one LED or two LEDs per position)
void VisualSetHelixTwoSided(bool two_sided);

// Get current helix color and brightness
void VisualGetHelixColor(ColorRGB *color, uint8_t *brightness);

// Get current helix two-sided setting
bool VisualIsHelixTwoSided(void);

// Apply color to helix LEDs
void VisualApplyHelix(void);

// =============================================================================
// BASE NEOPIXEL CONTROL
// =============================================================================

// Set color for a specific base (0 to DnaSlotsCount-1)
void VisualSetBaseColor(uint8_t base_index, ColorRGB color, uint8_t brightness);

// Get current color and brightness for a base
void VisualGetBaseColor(uint8_t base_index, ColorRGB *color, uint8_t *brightness);

// Apply color to a specific base LED
void VisualApplyBase(uint8_t base_index);

// Apply colors to all base LEDs
void VisualApplyAllBases(void);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Convert hue, saturation, value to RGB
ColorRGB VisualHsvToRgb(uint16_t hue, uint8_t saturation, uint8_t value);

// Get a predefined color by name
typedef enum {
    ColorRed,
    ColorGreen,
    ColorBlue,
    ColorYellow,
    ColorCyan,
    ColorMagenta,
    ColorWhite,
    ColorBlack,
    ColorOrange,
    ColorPurple,
    ColorPink,
    ColorAqua,
    // DNA base colors
    ColorBaseA,
    ColorBaseT,
    ColorBaseG,
    ColorBaseC,
    ColorBaseUnknown,
    // Preset colors
    ColorLightBlue,
    ColorDarkBlue,
    ColorLightRed,
    ColorDarkRed,
    ColorLightGreen,
    ColorDarkGreen
} PredefinedColor;

ColorRGB VisualGetPredefinedColor(PredefinedColor color);

// =============================================================================
// DIRECT LED STRIP ACCESS (Optional for advanced use)
// =============================================================================

// Get the helix LED strip handle (read-only)
led_strip_handle_t VisualGetHelixStripHandle(void);

// Get the base LED strip handle (read-only)
led_strip_handle_t VisualGetBaseStripHandle(void);
