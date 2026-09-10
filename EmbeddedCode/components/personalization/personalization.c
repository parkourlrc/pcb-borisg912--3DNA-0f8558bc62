#include "personalization.h"

// Get track number for a sound effect
uint16_t PersonalizationGetSoundTrack(SoundEffect effect) {
    switch (effect) {
        case SOUND_SUCCESS:      return 1;
        case SOUND_FAIL:         return 2;
        case SOUND_BLE_PAIR:     return 3;
        case SOUND_BLE_SUCCESS:  return 4;
        case SOUND_BASE_CORRECT: return 5;
        case SOUND_BASE_WRONG:   return 6;
        case SOUND_POWER_ON:     return 7;
        case SOUND_POWER_OFF:    return 8;
        case SOUND_ALERT:        return 9;
        case SOUND_MAX:
        default:                return 0;
    }
}
