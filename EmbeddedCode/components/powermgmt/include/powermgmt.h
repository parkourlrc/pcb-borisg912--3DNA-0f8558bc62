#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ChargeIdle = 0,
    ChargeActive,
    ChargeDone,
    ChargeFault
} ChargeState;

// Public 5V system flags (accessible by any function)
extern volatile bool Power5vPresent;
extern volatile bool Power5vPowerOk;

// Initialization
void PowerMgmtInit(void);

// GPIO-controlled functions
void PowerMgmtSetChargeEnable(bool enabled);
void PowerMgmtSetOtgEnable(bool enabled);

// USB power management
void PowerMgmtSetUsbHighPower(bool enable);
void PowerMgmtSetUsbCharging(bool enable);

// Current and voltage limits
void PowerMgmtSetChargeCurrent(uint16_t current_ma);
void PowerMgmtSetTerminationCurrent(uint16_t current_ma);
void PowerMgmtSetChargeVoltage(uint16_t voltage_mv);
void PowerMgmtSetInputVoltageLimit(uint16_t voltage_mv);

// Watchdog and timers
void PowerMgmtSetWatchdogTimer(bool enable);

// Status reading
ChargeState PowerMgmtReadState(void);
uint8_t PowerMgmtReadFaultStatus(void);
uint8_t PowerMgmtReadSystemStatus(void);
void PowerMgmtClearFaults(void);

// Getters
uint16_t PowerMgmtGetChargeCurrent(void);
uint16_t PowerMgmtGetChargeVoltage(void);
uint16_t PowerMgmtGetTerminationCurrent(void);

// 5V system management
void PowerMgmtUpdate5vSystem(void);
void PowerMgmtForce5v(bool enable);
const char* PowerMgmtGet5vStatusString(void);

// Ship mode (power off/on)
void PowerMgmtEnterShipMode(void);
void PowerMgmtExitShipMode(void);

// Emergency reset
void PowerMgmtEmergencyForceReset(void);
