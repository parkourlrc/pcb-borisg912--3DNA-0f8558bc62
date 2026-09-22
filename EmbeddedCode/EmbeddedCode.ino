// General
#define DnaSlotsCount 13     /* 12 helix slots + 1 foundation slot */

// I2C bus
#define I2cSda  22
#define I2cScl  23
#define I2cFreq 100000

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
#define FanControl 4

void setup() {
  pinMode(MuxS0, OUTPUT); pinMode(MuxS1, OUTPUT);
  pinMode(MuxS2, OUTPUT); pinMode(MuxS3, OUTPUT);
  pinMode(MuxResEn, OUTPUT); pinMode(MuxNeoPixelEn, OUTPUT);
  pinMode(MuxNeoPixelSig, OUTPUT); pinMode(HelixNeoPixelSig, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

}
