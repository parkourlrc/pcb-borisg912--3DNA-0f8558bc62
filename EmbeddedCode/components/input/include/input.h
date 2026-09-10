#pragma once

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// BUTTON PRESS TYPES
// =============================================================================

typedef enum {
    InputPressNone = 0,
    InputPressSingle,
    InputPressDouble,
    InputPressTriple,
    InputPressHold5s,
    InputPressHold10s
} InputPressType;

// =============================================================================
// BUTTON IDS
// =============================================================================

typedef enum {
    InputButtonBackboard = 0,
    InputButtonTouch,
    InputButtonCount
} InputButtonId;

// =============================================================================
// PRESS STATE INFORMATION
// =============================================================================

// Press state for a button
typedef struct {
    InputPressType detected_type;      // Currently detected press type
    uint32_t overall_press_time_ms;    // Total time from press start to now
    uint32_t hold_time_from_checkpoint_ms; // Time since last checkpoint
    bool is_pressed;                   // Current button state
    bool was_pressed;                  // Previous button state
    uint32_t press_start_time;        // When the press started
    uint32_t release_time;             // When the press ended
    uint8_t consecutive_presses;       // Number of consecutive presses
    bool press_handled;               // Whether press has been handled
} InputButtonState;

// =============================================================================
// EVENT CALLBACK
// =============================================================================

// Button event callback type
typedef void (*InputEventCallback)(InputButtonId button_id, InputPressType press_type);

// =============================================================================
// INITIALIZATION AND UPDATE
// =============================================================================

// Initialize input system
void InputInit(void);

// Update input system - call periodically (e.g., every 10ms)
void InputUpdate(void);

// =============================================================================
// EVENT HANDLING
// =============================================================================

// Set callback for button events
void InputSetEventCallback(InputEventCallback callback);

// Check if a specific button press type has occurred (clears after read)
InputPressType InputGetPressType(InputButtonId button_id);

// Clear any pending button events
void InputClearEvents(void);

// Clear events for a specific button
void InputClearButtonEvents(InputButtonId button_id);

// =============================================================================
// STATE ACCESSORS
// =============================================================================

// Get the current state of a button
InputButtonState InputGetButtonState(InputButtonId button_id);

// Get overall press time for a button (in ms)
uint32_t InputGetOverallPressTime(InputButtonId button_id);

// Get hold time from last checkpoint for a button (in ms)
uint32_t InputGetHoldTimeFromCheckpoint(InputButtonId button_id);

// Get currently detected press type for a button
InputPressType InputGetDetectedPressType(InputButtonId button_id);

// Set a checkpoint for hold time measurement
void InputSetHoldCheckpoint(InputButtonId button_id);
