#pragma once

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/i2c_master.h"

// General
#define DnaSlotsCount 13     /* 12 helix slots + 1 foundation slot */

// I2C bus
#define I2cSda  22
#define I2cScl  23
#define I2cFreq 100000
#define I2C_PORT I2C_NUM_0

extern i2c_master_bus_handle_t i2c_bus_handle;

// BQ25895
#define BmsInt    6
#define BmsOtg    21
#define BmsCe     16
#define BuckPwrOK 3

// Base Multiplexers (address bits shared by both MUXes)
#define MuxS0          11
#define MuxS1          10
#define MuxS2          9
#define MuxS3          8
#define MuxResEn       18
#define MuxResSig      0
#define MuxResAdcChan  ADC_CHANNEL_0   /* ADC1_CH0 -> GPIO0 */
#define MuxNeoPixelEn  15
#define MuxNeoPixelSig 19

// Sensors
#define LightSensor  1
#define LightAdcChan ADC_CHANNEL_1   /* ADC1_CH1 -> GPIO1 */
#define GyroInt      17

// Helix NeoPixels
#define HelixNeoPixelSig   20
#define HelixNeoPixelCount 40
#define HelixNeoPixelStandCount 20
#define HelixNeoPixelStandSides 2

// Base NeoPixels (per base via MUX)
#define BaseNeoPixelCount 1  // One NeoPixel per base

// Inputs/Outputs
#define TouchButton     2
#define BackboardButton 7
#define AudioPlayerTx   5

// Other
#define FanControl 4

// Hardware initialization
void HardwareInit(void);
void HardwareDeinit(void);
