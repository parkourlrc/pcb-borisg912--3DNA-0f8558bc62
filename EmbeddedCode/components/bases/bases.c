#include "bases.h"
#include "hardware.h"
#include "esp_log.h"

static const char *TAG = "bases";

// Array of DNA slots (13 total: 12 helix + 1 foundation)
static DnaSlot dna_slots[DnaSlotsCount] = {0};

// Standard colors for each base type (A, T, C, G)
static const ColorRGB base_type_colors[] = {
    [BaseTypeA] = {255,   0,   0},  // Red for Adenine
    [BaseTypeT] = {  0, 255,   0},  // Green for Thymine
    [BaseTypeC] = {  0,   0, 255},  // Blue for Cytosine
    [BaseTypeG] = {255, 255,   0},  // Yellow for Guanine
    [BaseTypeUnknown] = {128, 128, 128}  // Gray for unknown
};

// Initialize bases system
void BasesInit(void) {
    // Initialize all slots with default values (off)
    for (int i = 0; i < DnaSlotsCount; i++) {
        dna_slots[i].CurrLight.color = (ColorRGB){0, 0, 0};
        dna_slots[i].CurrLight.brightness = 255;
    }
    
    ESP_LOGI(TAG, "Bases system initialized with %d slots", DnaSlotsCount);
}

// Get pointer to the DnaSlot array (for direct access)
DnaSlot* BasesGetSlots(void) {
    return dna_slots;
}

// Get the number of DNA slots
uint8_t BasesGetSlotCount(void) {
    return DnaSlotsCount;
}

// === Color Management ===

// Set color for a specific slot
void BasesSetSlotColor(uint8_t slot_index, ColorRGB color, uint8_t brightness) {
    if (slot_index >= DnaSlotsCount) {
        ESP_LOGW(TAG, "Slot index %d out of range", slot_index);
        return;
    }
    
    dna_slots[slot_index].CurrLight.color = color;
    dna_slots[slot_index].CurrLight.brightness = brightness;
    
    // Apply to visual system
    VisualSetBaseColor(slot_index, color, brightness);
    
    ESP_LOGD(TAG, "Slot %d color set to R:%d G:%d B:%d, brightness:%d", 
             slot_index, color.r, color.g, color.b, brightness);
}

// Get color for a specific slot
void BasesGetSlotColor(uint8_t slot_index, ColorRGB *color, uint8_t *brightness) {
    if (slot_index >= DnaSlotsCount) {
        ESP_LOGW(TAG, "Slot index %d out of range", slot_index);
        if (color) *color = (ColorRGB){0, 0, 0};
        if (brightness) *brightness = 255;
        return;
    }
    
    if (color != NULL) {
        *color = dna_slots[slot_index].CurrLight.color;
    }
    if (brightness != NULL) {
        *brightness = dna_slots[slot_index].CurrLight.brightness;
    }
}

// Set all slots to a specific color
void BasesSetAllSlotsColor(ColorRGB color, uint8_t brightness) {
    for (int i = 0; i < DnaSlotsCount; i++) {
        BasesSetSlotColor(i, color, brightness);
    }
    ESP_LOGD(TAG, "All %d slots set to R:%d G:%d B:%d, brightness:%d", 
             DnaSlotsCount, color.r, color.g, color.b, brightness);
}

// Clear all slot colors (set to black/off)
void BasesClearAllSlots(void) {
    BasesSetAllSlotsColor((ColorRGB){0, 0, 0}, 255);
    ESP_LOGD(TAG, "All slots cleared (off)");
}

// === Base Identification Colors ===

// Get the standard color for a base type
ColorRGB BasesGetBaseTypeColor(BaseType base_type) {
    if (base_type >= sizeof(base_type_colors) / sizeof(base_type_colors[0])) {
        ESP_LOGW(TAG, "Unknown base type %d", base_type);
        return base_type_colors[BaseTypeUnknown];
    }
    return base_type_colors[base_type];
}

// Set a slot color based on its base type
void BasesSetSlotByType(uint8_t slot_index, BaseType base_type, uint8_t brightness) {
    if (slot_index >= DnaSlotsCount) {
        ESP_LOGW(TAG, "Slot index %d out of range", slot_index);
        return;
    }
    
    ColorRGB color = BasesGetBaseTypeColor(base_type);
    BasesSetSlotColor(slot_index, color, brightness);
    
    ESP_LOGD(TAG, "Slot %d set to base type %d color (R:%d G:%d B:%d)", 
             slot_index, base_type, color.r, color.g, color.b);
}
