#include "visual.h"
#include "hardware.h"
#include "i2c.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "visual";

// =============================================================================
// LED STRIP CONFIGURATION
// =============================================================================

// Helix LED strip configuration
static led_strip_handle_t helix_strip = NULL;

// Base LED strip configuration (controlled via MUX)
static led_strip_handle_t base_strip = NULL;

// =============================================================================
// GLOBAL STATE
// =============================================================================

// Helix NeoPixel state
static LightState helix_light_state = {{0, 0, 0}, 255};
static bool helix_two_sided = true;
static bool helix_dirty = false;

// Base light states (one per base slot)
static LightState base_light_states[DnaSlotsCount] = {0};
static bool base_dirty[DnaSlotsCount] = {0};
static bool all_bases_dirty = false;

// =============================================================================
// LED STRIP INITIALIZATION HELPERS
// =============================================================================

// Configure LED strip
static led_strip_handle_t configure_led_strip(gpio_num_t gpio, uint16_t led_count) {
    led_strip_config_t strip_config = {
        .max_leds = led_count,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_CLK_HZ,
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };

    led_strip_handle_t strip = NULL;
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(ret));
        return NULL;
    }

    // Set GPIO
    ret = led_strip_set_gpio(strip, gpio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set LED strip GPIO: %s", esp_err_to_name(ret));
        led_strip_del(strip);
        return NULL;
    }

    return strip;
}

// Convert ColorRGB + brightness to led_strip pixel
static void color_to_led_pixel(ColorRGB color, uint8_t brightness, led_strip_pixel_t *pixel) {
    // Apply brightness scaling
    float brightness_factor = brightness / 255.0f;
    pixel->red = (uint8_t)(color.r * brightness_factor);
    pixel->green = (uint8_t)(color.g * brightness_factor);
    pixel->blue = (uint8_t)(color.b * brightness_factor);
    pixel->white = 0;
}

// =============================================================================
// INITIALIZATION
// =============================================================================

// Initialize visual system
void VisualInit(void) {
    ESP_LOGI(TAG, "Initializing visual system");

    // Initialize helix LED strip
    helix_strip = configure_led_strip(HelixNeoPixelSig, HelixNeoPixelCount);
    if (helix_strip == NULL) {
        ESP_LOGE(TAG, "Failed to initialize helix LED strip");
    } else {
        ESP_LOGI(TAG, "Helix LED strip initialized with %d LEDs", HelixNeoPixelCount);
    }

    // Initialize base LED strip (via MUX)
    base_strip = configure_led_strip(MuxNeoPixelSig, BaseNeoPixelCount);
    if (base_strip == NULL) {
        ESP_LOGE(TAG, "Failed to initialize base LED strip");
    } else {
        ESP_LOGI(TAG, "Base LED strip initialized");
    }

    // Set default helix color (off)
    VisualSetHelixColor((ColorRGB){0, 0, 0}, 0);

    // Initialize all base colors to off
    for (int i = 0; i < DnaSlotsCount; i++) {
        VisualSetBaseColor(i, (ColorRGB){0, 0, 0}, 0);
    }

    // Clear dirty flags
    VisualApplyHelix();
    VisualApplyAllBases();

    ESP_LOGI(TAG, "Visual system initialized successfully");
}

// Deinitialize visual system
void VisualDeinit(void) {
    ESP_LOGI(TAG, "Deinitializing visual system");

    if (helix_strip != NULL) {
        led_strip_del(helix_strip);
        helix_strip = NULL;
    }

    if (base_strip != NULL) {
        led_strip_del(base_strip);
        base_strip = NULL;
    }

    ESP_LOGI(TAG, "Visual system deinitialized");
}

// Update visual system (call periodically to update LEDs)
void VisualUpdate(void) {
    // Update helix if dirty
    if (helix_dirty) {
        VisualApplyHelix();
        helix_dirty = false;
    }

    // Update all bases if needed
    if (all_bases_dirty) {
        VisualApplyAllBases();
        all_bases_dirty = false;
    } else {
        // Update individual bases
        for (int i = 0; i < DnaSlotsCount; i++) {
            if (base_dirty[i]) {
                VisualApplyBase(i);
                base_dirty[i] = false;
            }
        }
    }
}

// =============================================================================
// HELIX NEOPIXEL CONTROL
// =============================================================================

// Set color for the entire helix strip
void VisualSetHelixColor(ColorRGB color, uint8_t brightness) {
    helix_light_state.color = color;
    helix_light_state.brightness = brightness;
    helix_dirty = true;
    ESP_LOGD(TAG, "Helix color set to R:%d G:%d B:%d, brightness:%d",
             color.r, color.g, color.b, brightness);
}

// Set color for a specific position in the helix
void VisualSetHelixPosition(uint16_t position, ColorRGB color, uint8_t brightness) {
    if (position >= HelixNeoPixelCount) {
        ESP_LOGW(TAG, "Helix position %d out of range", position);
        return;
    }

    // For now, set the global helix color
    // In a full implementation, this would set individual LED colors
    VisualSetHelixColor(color, brightness);
}

// Set two-sided mode for helix (one LED or two LEDs per position)
void VisualSetHelixTwoSided(bool two_sided) {
    helix_two_sided = two_sided;
    helix_dirty = true;
    ESP_LOGD(TAG, "Helix two-sided mode: %s", two_sided ? "true" : "false");
}

// Get current helix color and brightness
void VisualGetHelixColor(ColorRGB *color, uint8_t *brightness) {
    if (color != NULL) {
        *color = helix_light_state.color;
    }
    if (brightness != NULL) {
        *brightness = helix_light_state.brightness;
    }
}

// Get current helix two-sided setting
bool VisualIsHelixTwoSided(void) {
    return helix_two_sided;
}

// Apply color to helix LEDs
void VisualApplyHelix(void) {
    if (helix_strip == NULL) {
        ESP_LOGW(TAG, "Helix strip not initialized");
        return;
    }

    led_strip_pixel_t pixel;
    color_to_led_pixel(helix_light_state.color, helix_light_state.brightness, &pixel);

    // Apply to all LEDs in the helix
    for (int i = 0; i < HelixNeoPixelCount; i++) {
        esp_err_t ret = led_strip_set_pixel(helix_strip, i, &pixel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set helix pixel %d: %s", i, esp_err_to_name(ret));
        }
    }

    // Refresh the strip
    esp_err_t ret = led_strip_refresh(helix_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh helix strip: %s", esp_err_to_name(ret));
    }

    ESP_LOGD(TAG, "Applied helix color: R:%d G:%d B:%d",
             pixel.red, pixel.green, pixel.blue);
}

// =============================================================================
// BASE NEOPIXEL CONTROL
// =============================================================================

// Set color for a specific base (0 to DnaSlotsCount-1)
void VisualSetBaseColor(uint8_t base_index, ColorRGB color, uint8_t brightness) {
    if (base_index >= DnaSlotsCount) {
        ESP_LOGW(TAG, "Base index %d out of range", base_index);
        return;
    }

    base_light_states[base_index].color = color;
    base_light_states[base_index].brightness = brightness;
    base_dirty[base_index] = true;
    all_bases_dirty = true;

    ESP_LOGD(TAG, "Base %d color set to R:%d G:%d B:%d, brightness:%d",
             base_index, color.r, color.g, color.b, brightness);
}

// Get current color and brightness for a base
void VisualGetBaseColor(uint8_t base_index, ColorRGB *color, uint8_t *brightness) {
    if (base_index >= DnaSlotsCount) {
        ESP_LOGW(TAG, "Base index %d out of range", base_index);
        return;
    }

    if (color != NULL) {
        *color = base_light_states[base_index].color;
    }
    if (brightness != NULL) {
        *brightness = base_light_states[base_index].brightness;
    }
}

// Apply color to a specific base LED (via MUX)
void VisualApplyBase(uint8_t base_index) {
    if (base_index >= DnaSlotsCount) {
        ESP_LOGW(TAG, "Base index %d out of range", base_index);
        return;
    }

    if (base_strip == NULL) {
        ESP_LOGW(TAG, "Base strip not initialized");
        return;
    }

    // TODO: Select base via MUX (address lines S0-S3)
    // For now, assume base 0 is selected
    // In actual implementation:
    // 1. Set MUX address to select the base
    // 2. Apply color via base_strip
    // 3. Deselect MUX

    led_strip_pixel_t pixel;
    color_to_led_pixel(base_light_states[base_index].color, 
                       base_light_states[base_index].brightness, &pixel);

    esp_err_t ret = led_strip_set_pixel(base_strip, 0, &pixel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set base %d pixel: %s", base_index, esp_err_to_name(ret));
    }

    ret = led_strip_refresh(base_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh base strip: %s", esp_err_to_name(ret));
    }

    ESP_LOGD(TAG, "Applied base %d color: R:%d G:%d B:%d",
             base_index, pixel.red, pixel.green, pixel.blue);

    base_dirty[base_index] = false;
}

// Apply colors to all base LEDs
void VisualApplyAllBases(void) {
    for (int i = 0; i < DnaSlotsCount; i++) {
        if (base_dirty[i]) {
            VisualApplyBase(i);
        }
    }
    all_bases_dirty = false;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Convert hue, saturation, value to RGB
ColorRGB VisualHsvToRgb(uint16_t hue, uint8_t saturation, uint8_t value) {
    hue %= 360;
    uint8_t rgb_max = value;
    uint8_t rgb_min = rgb_max * (255 - saturation) / 255;

    uint8_t r, g, b;

    if (hue < 60) {
        r = rgb_max;
        g = rgb_min + (rgb_max - rgb_min) * hue / 60;
        b = rgb_min;
    } else if (hue < 120) {
        r = rgb_max - (rgb_max - rgb_min) * (hue - 60) / 60;
        g = rgb_max;
        b = rgb_min;
    } else if (hue < 180) {
        r = rgb_min;
        g = rgb_max;
        b = rgb_min + (rgb_max - rgb_min) * (hue - 120) / 60;
    } else if (hue < 240) {
        r = rgb_min;
        g = rgb_max - (rgb_max - rgb_min) * (hue - 180) / 60;
        b = rgb_max;
    } else if (hue < 300) {
        r = rgb_min + (rgb_max - rgb_min) * (hue - 240) / 60;
        g = rgb_min;
        b = rgb_max;
    } else {
        r = rgb_max;
        g = rgb_min;
        b = rgb_max - (rgb_max - rgb_min) * (hue - 300) / 60;
    }

    return (ColorRGB){r, g, b};
}

// Get a predefined color by name
ColorRGB VisualGetPredefinedColor(PredefinedColor color) {
    switch (color) {
        // Basic colors
        case ColorRed:      return (ColorRGB){255,   0,   0};
        case ColorGreen:    return (ColorRGB){  0, 255,   0};
        case ColorBlue:     return (ColorRGB){  0,   0, 255};
        case ColorYellow:   return (ColorRGB){255, 255,   0};
        case ColorCyan:     return (ColorRGB){  0, 255, 255};
        case ColorMagenta:  return (ColorRGB){255,   0, 255};
        case ColorWhite:    return (ColorRGB){255, 255, 255};
        case ColorBlack:    return (ColorRGB){  0,   0,   0};
        case ColorOrange:   return (ColorRGB){255, 165,   0};
        case ColorPurple:   return (ColorRGB){128,   0, 128};
        case ColorPink:     return (ColorRGB){255, 192, 203};
        case ColorAqua:     return (ColorRGB){  0, 255, 255};

        // DNA base colors
        case ColorBaseA:        return COLOR_BASE_A;
        case ColorBaseT:        return COLOR_BASE_T;
        case ColorBaseG:        return COLOR_BASE_G;
        case ColorBaseC:        return COLOR_BASE_C;
        case ColorBaseUnknown: return COLOR_BASE_UNKNOWN;

        // Preset colors
        case ColorLightBlue:   return COLOR_LIGHT_BLUE;
        case ColorDarkBlue:    return COLOR_DARK_BLUE;
        case ColorLightRed:    return COLOR_LIGHT_RED;
        case ColorDarkRed:     return COLOR_DARK_RED;
        case ColorLightGreen:  return COLOR_LIGHT_GREEN;
        case ColorDarkGreen:   return COLOR_DARK_GREEN;

        default: return (ColorRGB){255, 255, 255};
    }
}

// =============================================================================
// DIRECT LED STRIP ACCESS
// =============================================================================

// Get the helix LED strip handle (read-only)
led_strip_handle_t VisualGetHelixStripHandle(void) {
    return helix_strip;
}

// Get the base LED strip handle (read-only)
led_strip_handle_t VisualGetBaseStripHandle(void) {
    return base_strip;
}
