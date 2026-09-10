#include "input.h"
#include "hardware.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "input";

// =============================================================================
// CONFIGURATION
// =============================================================================

// Button press detection constants
#define BUTTON_DEBOUNCE_MS        20      // Debounce time
#define BUTTON_HOLD_5S_MS         5000    // Hold for 5 seconds
#define BUTTON_HOLD_10S_MS        10000   // Hold for 10 seconds (emergency reset)
#define BUTTON_DOUBLE_PRESS_MS    500     // Max time between presses for multi-press
#define BUTTON_MAX_PRESSES        3       // Max consecutive presses to detect

// =============================================================================
// BUTTON DEFINITIONS
// =============================================================================

// Button configuration
typedef struct {
    gpio_num_t gpio;
    bool active_low;  // true = active low (pull-up), false = active high
} ButtonConfig;

static const ButtonConfig button_configs[InputButtonCount] = {
    [InputButtonBackboard] = {BackboardButton, true},  // Active low with pull-up
    [InputButtonTouch]     = {TouchButton,     true},  // Active low (capacitive touch)
};

// =============================================================================
// STATE
// =============================================================================

static InputButtonState button_states[InputButtonCount] = {0};
static InputEventCallback event_callback = NULL;
static uint32_t hold_checkpoint_time[InputButtonCount] = {0};

// Pending events (one per button)
static InputPressType pending_events[InputButtonCount] = {0};

// =============================================================================
// TIME HELPERS
// =============================================================================

// Get current time in milliseconds
static uint32_t get_current_time_ms(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

// =============================================================================
// BUTTON READING
// =============================================================================

// Read the raw state of a button
static bool read_button_raw(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return false;
    }
    return gpio_get_level(button_configs[button_id].gpio) == 0;
}

// Check if a button is currently pressed (considering active low)
static bool is_button_pressed(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return false;
    }
    bool raw = read_button_raw(button_id);
    return button_configs[button_id].active_low ? !raw : raw;
}

// =============================================================================
// PRESS DETECTION LOGIC (SHARED FOR ALL BUTTONS)
// =============================================================================

// Update button state for a specific button
static void update_button_state(InputButtonId button_id) {
    InputButtonState *state = &button_states[button_id];
    bool current_pressed = is_button_pressed(button_id);
    uint32_t current_time = get_current_time_ms();

    // Calculate overall press time
    if (current_pressed && state->is_pressed) {
        state->overall_press_time_ms = current_time - state->press_start_time;
    } else {
        state->overall_press_time_ms = 0;
    }

    // Calculate hold time from checkpoint
    if (current_pressed && hold_checkpoint_time[button_id] > 0) {
        state->hold_time_from_checkpoint_ms = current_time - hold_checkpoint_time[button_id];
    } else {
        state->hold_time_from_checkpoint_ms = 0;
    }

    // Handle button press start
    if (current_pressed && !state->is_pressed) {
        state->press_start_time = current_time;
        state->is_pressed = true;
        state->consecutive_presses = 1;
        state->press_handled = false;
        state->detected_type = InputPressNone;
        ESP_LOGD(TAG, "Button %d pressed", button_id);
    }

    // Handle button release
    if (!current_pressed && state->is_pressed) {
        uint32_t press_duration = current_time - state->press_start_time;

        // Check for debounce
        if (press_duration >= BUTTON_DEBOUNCE_MS) {
            // Valid press - count consecutive presses
            if (current_time - state->release_time <= BUTTON_DOUBLE_PRESS_MS) {
                state->consecutive_presses++;
            } else {
                state->consecutive_presses = 1;
            }
            state->release_time = current_time;
        }

        state->is_pressed = false;
        ESP_LOGD(TAG, "Button %d released, duration: %lu ms, presses: %d",
                 button_id, press_duration, state->consecutive_presses);
    }

    // Check for hold detection
    if (state->is_pressed && !state->press_handled) {
        uint32_t hold_duration = current_time - state->press_start_time;

        if (hold_duration >= BUTTON_HOLD_10S_MS) {
            // 10+ second hold - emergency reset
            pending_events[button_id] = InputPressHold10s;
            state->detected_type = InputPressHold10s;
            state->press_handled = true;
            ESP_LOGW(TAG, "Button %d held for 10+ seconds - EMERGENCY", button_id);
        } else if (hold_duration >= BUTTON_HOLD_5S_MS) {
            // 5+ second hold
            pending_events[button_id] = InputPressHold5s;
            state->detected_type = InputPressHold5s;
            state->press_handled = true;
            ESP_LOGI(TAG, "Button %d held for 5+ seconds", button_id);
        }
    }

    // Check for multi-press completion (button released after consecutive presses)
    if (!current_pressed && state->consecutive_presses > 0 &&
        state->consecutive_presses <= BUTTON_MAX_PRESSES &&
        !state->press_handled) {

        // Wait for double press timeout to see if more presses are coming
        if (current_time - state->release_time >= BUTTON_DOUBLE_PRESS_MS) {
            // Multi-press sequence complete
            switch (state->consecutive_presses) {
                case 1:
                    pending_events[button_id] = InputPressSingle;
                    state->detected_type = InputPressSingle;
                    break;
                case 2:
                    pending_events[button_id] = InputPressDouble;
                    state->detected_type = InputPressDouble;
                    break;
                case 3:
                    pending_events[button_id] = InputPressTriple;
                    state->detected_type = InputPressTriple;
                    break;
                default:
                    pending_events[button_id] = InputPressSingle;
                    state->detected_type = InputPressSingle;
                    break;
            }
            state->press_handled = true;
            ESP_LOGI(TAG, "Button %d: %d presses detected", button_id, state->consecutive_presses);
        }
    }

    // Reset state if no activity for a while
    if (!current_pressed && !state->is_pressed &&
        current_time - state->release_time > BUTTON_DOUBLE_PRESS_MS * 2) {
        state->consecutive_presses = 0;
        state->press_handled = false;
        state->detected_type = InputPressNone;
    }

    // Update state's detected type
    if (pending_events[button_id] != InputPressNone && !state->press_handled) {
        state->detected_type = pending_events[button_id];
    }
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

// Initialize input system
void InputInit(void) {
    ESP_LOGI(TAG, "Initializing input system");

    for (int i = 0; i < InputButtonCount; i++) {
        // Configure GPIO as input with pull-up
        gpio_config_t config = {
            .pin_bit_mask = (1ULL << button_configs[i].gpio),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&config);

        // Initialize state
        button_states[i].is_pressed = false;
        button_states[i].was_pressed = false;
        button_states[i].press_start_time = 0;
        button_states[i].release_time = 0;
        button_states[i].consecutive_presses = 0;
        button_states[i].press_handled = false;
        button_states[i].detected_type = InputPressNone;
        button_states[i].overall_press_time_ms = 0;
        button_states[i].hold_time_from_checkpoint_ms = 0;
        hold_checkpoint_time[i] = 0;
        pending_events[i] = InputPressNone;
    }

    ESP_LOGI(TAG, "Input system initialized");
}

// Update input system - call periodically (e.g., every 10ms)
void InputUpdate(void) {
    for (int i = 0; i < InputButtonCount; i++) {
        update_button_state((InputButtonId)i);

        // If we have a pending event, notify callback
        if (pending_events[i] != InputPressNone && event_callback != NULL) {
            event_callback((InputButtonId)i, pending_events[i]);
            // Don't clear here - let caller clear via InputGetPressType
        }
    }
}

// Set callback for button events
void InputSetEventCallback(InputEventCallback callback) {
    event_callback = callback;
}

// Check if a specific button press type has occurred (clears after read)
InputPressType InputGetPressType(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return InputPressNone;
    }
    InputPressType result = pending_events[button_id];
    pending_events[button_id] = InputPressNone;
    return result;
}

// Clear any pending button events
void InputClearEvents(void) {
    for (int i = 0; i < InputButtonCount; i++) {
        pending_events[i] = InputPressNone;
        button_states[i].press_handled = false;
        button_states[i].consecutive_presses = 0;
        button_states[i].detected_type = InputPressNone;
    }
}

// Clear events for a specific button
void InputClearButtonEvents(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return;
    }
    pending_events[button_id] = InputPressNone;
    button_states[button_id].press_handled = false;
    button_states[button_id].consecutive_presses = 0;
    button_states[button_id].detected_type = InputPressNone;
}

// =============================================================================
// STATE ACCESSORS
// =============================================================================

// Get the current state of a button
InputButtonState InputGetButtonState(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return (InputButtonState){0};
    }
    return button_states[button_id];
}

// Get overall press time for a button (in ms)
uint32_t InputGetOverallPressTime(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return 0;
    }
    return button_states[button_id].overall_press_time_ms;
}

// Get hold time from last checkpoint for a button (in ms)
uint32_t InputGetHoldTimeFromCheckpoint(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return 0;
    }
    return button_states[button_id].hold_time_from_checkpoint_ms;
}

// Get currently detected press type for a button
InputPressType InputGetDetectedPressType(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return InputPressNone;
    }
    return button_states[button_id].detected_type;
}

// Set a checkpoint for hold time measurement
void InputSetHoldCheckpoint(InputButtonId button_id) {
    if (button_id >= InputButtonCount) {
        return;
    }
    hold_checkpoint_time[button_id] = get_current_time_ms();
    button_states[button_id].hold_time_from_checkpoint_ms = 0;
    ESP_LOGD(TAG, "Hold checkpoint set for button %d", button_id);
}
