#pragma once

#include <stdint.h>
#include "personalization.h"

// Initialize audio player (DFPlayer Mini)
void PlayerInit(void);

// Send raw command to DFPlayer Mini
void PlayerSendCmd(uint8_t command, uint16_t parameter);

// Play a specific track by number
void PlayerPlayTrack(uint16_t trackNumber);

// Play a predefined sound effect
void PlayerPlaySound(SoundEffect effect);

// Pause playback
void PlayerPause(void);

// Set volume (0-30)
void PlayerSetVolume(uint8_t volume);

// Enter power save mode
void PlayerEnterPowerSaver(void);

// Exit power save mode
void PlayerExitPowerSaver(void);

// Check if audio player is initialized
bool PlayerIsInitialized(void);
