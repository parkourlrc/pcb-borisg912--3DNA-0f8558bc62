#pragma once

#include <stdint.h>
#include "visual.h"

// DNA base slot structure
// Represents one position in the DNA model (helix or foundation)
typedef struct {
    // Current light state for this slot
    LightState CurrLight;
    
    // TODO: Future entries
    // - Base type (A, T, C, G)
    // - Detected resistor value
    // - Identification status
    // - Pairing information
} DnaSlot;

// Initialize bases system
void BasesInit(void);

// Get pointer to the DnaSlot array (for direct access)
DnaSlot* BasesGetSlots(void);

// Get the number of DNA slots
uint8_t BasesGetSlotCount(void);

// === Color Management ===

// Set color for a specific slot
void BasesSetSlotColor(uint8_t slot_index, ColorRGB color, uint8_t brightness);

// Get color for a specific slot
void BasesGetSlotColor(uint8_t slot_index, ColorRGB *color, uint8_t *brightness);

// Set all slots to a specific color
void BasesSetAllSlotsColor(ColorRGB color, uint8_t brightness);

// Clear all slot colors (set to black/off)
void BasesClearAllSlots(void);

// === Base Identification Colors ===

// Set color based on base type (A, T, C, G)
typedef enum {
    BaseTypeA = 0,
    BaseTypeT,
    BaseTypeC,
    BaseTypeG,
    BaseTypeUnknown
} BaseType;

// Get the standard color for a base type
ColorRGB BasesGetBaseTypeColor(BaseType base_type);

// Set a slot color based on its base type
void BasesSetSlotByType(uint8_t slot_index, BaseType base_type, uint8_t brightness);
