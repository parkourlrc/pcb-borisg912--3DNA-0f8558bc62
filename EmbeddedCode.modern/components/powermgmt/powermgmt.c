#include "powermgmt.h"
#include "hardware.h"
#include "driver/gpio.h"
#include "i2c.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "powermgmt";

// BQ25895 I2C address
#define BQ25895_ADDR  0x6B

// BQ25895 Registers
#define BQ_REG_STATUS          0x00
#define BQ_REG_CHG_CURRENT     0x02
#define BQ_REG_PRE_CHG         0x03
#define BQ_REG_VOLTAGE         0x05
#define BQ_REG_INPUT_SRC       0x08
#define BQ_REG_POWER_ON        0x09
#define BQ_REG_CHARGE_PUMP     0x0A
#define BQ_REG_SYS_STATUS      0x0E
#define BQ_REG_FAULT           0x0F

// Current limits (mA) - base values for BQ25895
#define CHG_CURRENT_BASE   64  // 64mA per LSB
#define TERM_CURRENT_BASE  8   // 8mA per LSB

// Default settings
#define DEFAULT_CHG_CURRENT_MHZ   512     // 512mA = 8 * 64mA
#define DEFAULT_TERM_CURRENT_MHZ  128     // 128mA = 16 * 8mA
#define DEFAULT_VOLTAGE_MV        4208    // 4.208V

// Bit masks for registers
#define INPUT_SRC_USB_ENABLE    (1 << 4)   // OTG_EN
#define INPUT_SRC_USB_HIZ       (1 << 3)   // USB_HIZ
#define POWER_ON_WATCHDOG_RST   (1 << 7)   // WATCHDOG timer reset

i2c_master_device_handle_t bq25895_device;

// Public 5V system flags (accessible by any function)
volatile bool Power5vPresent = false;
volatile bool Power5vPowerOk = false;

// Internal OTG state tracking
static bool is_otg_enabled = false;

static esp_err_t bq25895_read_reg(uint8_t reg_addr, uint8_t *data, uint8_t len) {
    return i2c_master_read_from_device(I2C_PORT, BQ25895_ADDR, reg_addr, data, len, pdMS_TO_TICKS(100));
}

static esp_err_t bq25895_write_reg(uint8_t reg_addr, uint8_t *data, uint8_t len) {
    return i2c_master_write_to_device(I2C_PORT, BQ25895_ADDR, reg_addr, data, len, pdMS_TO_TICKS(100));
}

static esp_err_t bq25895_read_byte(uint8_t reg_addr, uint8_t *value) {
    return bq25895_read_reg(reg_addr, value, 1);
}

static esp_err_t bq25895_write_byte(uint8_t reg_addr, uint8_t value) {
    return bq25895_write_reg(reg_addr, &value, 1);
}

void PowerMgmtInit(void) {
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << BmsCe) | (1ULL << BmsOtg),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&config);
    gpio_set_direction(BmsInt, GPIO_MODE_INPUT);
    gpio_set_direction(BuckPwrOK, GPIO_MODE_INPUT);

    gpio_set_level(BmsCe, 0);    /* charging not allowed */
    gpio_set_level(BmsOtg, 0);   /* OTG boost off */

    // Initialize I2C device for BQ25895
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BQ25895_ADDR,
        .scl_speed_hz = I2cFreq,
    };
    esp_err_t ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &bq25895_device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add BQ25895 I2C device: %s", esp_err_to_name(ret));
        return;
    }

    // Configure default charging parameters
    PowerMgmtSetChargeCurrent(DEFAULT_CHG_CURRENT_MHZ);
    PowerMgmtSetTerminationCurrent(DEFAULT_TERM_CURRENT_MHZ);
    PowerMgmtSetChargeVoltage(DEFAULT_VOLTAGE_MV);
    PowerMgmtSetUsbHighPower(false);
    PowerMgmtSetWatchdogTimer(true);

    // Enable charging
    PowerMgmtSetChargeEnable(true);
}

void PowerMgmtSetChargeEnable(bool enabled) {
    gpio_set_level(BmsCe, enabled ? 1 : 0);
}

void PowerMgmtSetOtgEnable(bool enabled) {
    gpio_set_level(BmsOtg, enabled ? 1 : 0);
}

// Enable/disable USB high power charging (5V/9V)
void PowerMgmtSetUsbHighPower(bool enable) {
    uint8_t reg_value;
    if (bq25895_read_byte(INPUT_SRC_USB_HIZ, &reg_value) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read INPUT_SOURCE register");
        return;
    }
    
    if (enable) {
        reg_value &= ~INPUT_SRC_USB_HIZ;  // Clear HIZ bit to enable
    } else {
        reg_value |= INPUT_SRC_USB_HIZ;   // Set HIZ bit to disable
    }
    
    bq25895_write_byte(INPUT_SRC_USB_HIZ, reg_value);
}

// Enable/disable USB charging
void PowerMgmtSetUsbCharging(bool enable) {
    uint8_t reg_value;
    if (bq25895_read_byte(INPUT_SRC_USB_HIZ, &reg_value) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read INPUT_SOURCE register");
        return;
    }
    
    if (enable) {
        reg_value &= ~INPUT_SRC_USB_HIZ;  // Clear HIZ bit to enable
    } else {
        reg_value |= INPUT_SRC_USB_HIZ;   // Set HIZ bit to disable
    }
    
    bq25895_write_byte(INPUT_SRC_USB_HIZ, reg_value);
}

// Set charge current limit in mA (64mA per step, max 3008mA)
void PowerMgmtSetChargeCurrent(uint16_t current_ma) {
    uint8_t reg_value;
    
    // Clamp to valid range (0-3008mA in 64mA steps)
    if (current_ma > 3008) current_ma = 3008;
    if (current_ma < 64) current_ma = 64;
    
    reg_value = (current_ma / CHG_CURRENT_BASE) - 1;
    bq25895_write_byte(BQ_REG_CHG_CURRENT, reg_value);
}

// Set termination current in mA (8mA per step, max 2048mA)
void PowerMgmtSetTerminationCurrent(uint16_t current_ma) {
    uint8_t reg_value;
    
    // Clamp to valid range
    if (current_ma > 2048) current_ma = 2048;
    if (current_ma < 8) current_ma = 8;
    
    reg_value = (current_ma / TERM_CURRENT_BASE) - 1;
    bq25895_write_byte(BQ_REG_PRE_CHG, reg_value & 0x0F);  // Term current in bits 0-3
}

// Set charge voltage limit in mV (16mV per step, 4.0V-4.656V)
void PowerMgmtSetChargeVoltage(uint16_t voltage_mv) {
    uint8_t reg_value;
    
    // Clamp to valid range (4000mV - 4656mV)
    if (voltage_mv > 4656) voltage_mv = 4656;
    if (voltage_mv < 4000) voltage_mv = 4000;
    
    reg_value = (voltage_mv - 4000) / 16;
    bq25895_write_byte(BQ_REG_VOLTAGE, reg_value);
}

// Set input voltage limit for USB (5V or 9V)
void PowerMgmtSetInputVoltageLimit(uint16_t voltage_mv) {
    uint8_t reg_value;
    
    if (bq25895_read_byte(BQ_REG_INPUT_SRC, &reg_value) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read INPUT_SOURCE register");
        return;
    }
    
    // Bit 6: VINDPM - Input voltage limit
    // 0 = 4.6V, 1 = 4.8V (for USB 5V input)
    // For 9V input, different settings apply
    if (voltage_mv >= 9000) {
        reg_value |= (1 << 6);  // Higher limit for 9V
    } else {
        reg_value &= ~(1 << 6); // Standard limit for 5V
    }
    
    bq25895_write_byte(BQ_REG_INPUT_SRC, reg_value);
}

// Reset watchdog timer
void PowerMgmtSetWatchdogTimer(bool enable) {
    uint8_t reg_value = enable ? POWER_ON_WATCHDOG_RST : 0;
    bq25895_write_byte(BQ_REG_POWER_ON, reg_value);
}

// Read charge status
ChargeState PowerMgmtReadState(void) {
    uint8_t status_reg;
    uint8_t fault_reg;
    
    if (bq25895_read_byte(BQ_REG_STATUS, &status_reg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read STATUS register");
        return ChargeFault;
    }
    
    if (bq25895_read_byte(BQ_REG_FAULT, &fault_reg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read FAULT register");
        return ChargeFault;
    }
    
    // Check for faults
    if (fault_reg != 0) {
        return ChargeFault;
    }
    
    // Check charge status
    // Bit 0: CHRG_STAT - 0 = not charging, 1 = charging
    // Bit 1: VBUS_STAT - 0 = no VBUS, 1 = VBUS present
    // Bit 2: PG_STAT - Power good
    bool is_charging = (status_reg & (1 << 0)) != 0;
    bool is_vbus_present = (status_reg & (1 << 1)) != 0;
    bool is_power_good = (status_reg & (1 << 2)) != 0;
    
    if (is_charging) {
        return ChargeActive;
    } else if (is_power_good && is_vbus_present) {
        return ChargeDone;
    } else {
        return ChargeIdle;
    }
}

// Read fault status
uint8_t PowerMgmtReadFaultStatus(void) {
    uint8_t fault_reg = 0;
    bq25895_read_byte(BQ_REG_FAULT, &fault_reg);
    return fault_reg;
}

// Read system status
uint8_t PowerMgmtReadSystemStatus(void) {
    uint8_t sys_status = 0;
    bq25895_read_byte(BQ_REG_SYS_STATUS, &sys_status);
    return sys_status;
}

// Clear fault status
void PowerMgmtClearFaults(void) {
    uint8_t fault_reg = 0;
    bq25895_read_byte(BQ_REG_FAULT, &fault_reg);
    // Writing to fault register clears it
    bq25895_write_byte(BQ_REG_FAULT, fault_reg);
}

// Get charge current limit in mA
uint16_t PowerMgmtGetChargeCurrent(void) {
    uint8_t reg_value;
    if (bq25895_read_byte(BQ_REG_CHG_CURRENT, &reg_value) != ESP_OK) {
        return 0;
    }
    return (reg_value + 1) * CHG_CURRENT_BASE;
}

// Get charge voltage limit in mV
uint16_t PowerMgmtGetChargeVoltage(void) {
    uint8_t reg_value;
    if (bq25895_read_byte(BQ_REG_VOLTAGE, &reg_value) != ESP_OK) {
        return 0;
    }
    return 4000 + (reg_value * 16);
}

// Get termination current in mA
uint16_t PowerMgmtGetTerminationCurrent(void) {
    uint8_t reg_value;
    if (bq25895_read_byte(BQ_REG_PRE_CHG, &reg_value) != ESP_OK) {
        return 0;
    }
    return (reg_value & 0x0F + 1) * TERM_CURRENT_BASE;
}

// Update 5V power OK status by reading BuckPwrOK pin
static void update_5v_power_ok(void) {
    bool previous_power_ok = Power5vPowerOk;
    Power5vPowerOk = gpio_get_level(BuckPwrOK) != 0;
    
    if (Power5vPowerOk != previous_power_ok) {
        ESP_LOGI(TAG, "5V power OK: %s", Power5vPowerOk ? "true" : "false");
    }
}

// Update 5V presence by reading VBUS from BQ25895 status
static void update_5v_presence(void) {
    uint8_t status_reg = 0;
    if (bq25895_read_byte(BQ_REG_STATUS, &status_reg) == ESP_OK) {
        bool was_present = Power5vPresent;
        // Bit 1: VBUS_STAT - 1 = VBUS present
        Power5vPresent = (status_reg & (1 << 1)) != 0;
        
        if (Power5vPresent != was_present) {
            ESP_LOGI(TAG, "VBUS %s", Power5vPresent ? "present" : "not present");
        }
    }
}

// Handle OTG enable/disable logic gracefully
static void handle_otg_state(void) {
    bool want_otg_enabled = false;
    
    // If we have 5V from charger (VBUS present) AND power is good
    if (Power5vPresent && Power5vPowerOk) {
        // Charger is connected and delivering power
        // PMIC can output 5-9V even with OTG disabled, so enable OTG
        want_otg_enabled = true;
    } else {
        // No charger or power not OK
        want_otg_enabled = false;
    }
    
    // If we need to change OTG state
    if (want_otg_enabled != is_otg_enabled) {
        if (want_otg_enabled) {
            // Enable OTG gracefully
            ESP_LOGI(TAG, "Enabling OTG (5V present and power OK)");
            PowerMgmtSetOtgEnable(true);
            is_otg_enabled = true;
        } else {
            // Disable OTG gracefully
            ESP_LOGI(TAG, "Disabling OTG (5V not present or power not OK)");
            PowerMgmtSetOtgEnable(false);
            is_otg_enabled = false;
        }
    }
    
    // If 5V was previously off and now it's on, handle initialization
    if (Power5vPresent && Power5vPowerOk && !is_otg_enabled) {
        // This shouldn't happen with the above logic, but handle it anyway
        PowerMgmtSetOtgEnable(true);
        is_otg_enabled = true;
    }
}

// Force 5V output state (enable/disable OTG)
void PowerMgmtForce5v(bool enable) {
    if (enable) {
        ESP_LOGI(TAG, "Force enabling 5V output");
        PowerMgmtSetOtgEnable(true);
        is_otg_enabled = true;
        // Mark as present so state doesn't get overridden
        Power5vPresent = true;
    } else {
        ESP_LOGI(TAG, "Force disabling 5V output");
        PowerMgmtSetOtgEnable(false);
        is_otg_enabled = false;
        Power5vPresent = false;
    }
}

// Main 5V system update - call periodically
void PowerMgmtUpdate5vSystem(void) {
    update_5v_presence();
    update_5v_power_ok();
    
    // Handle OTG state based on 5V presence and power OK
    handle_otg_state();
}

// Get current 5V system status
const char* PowerMgmtGet5vStatusString(void) {
    if (is_otg_enabled) {
        if (Power5vPowerOk) {
            return "5V OTG enabled, power OK";
        } else {
            return "5V OTG enabled, no power";
        }
    } else {
        if (Power5vPresent) {
            return "5V present, OTG disabled";
        } else {
            return "5V not present";
        }
    }
}

// Enter ship mode (actual power off)
void PowerMgmtEnterShipMode(void) {
    uint8_t reg_value;
    
    // Read current POWER_ON register
    if (bq25895_read_byte(BQ_REG_POWER_ON, &reg_value) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read POWER_ON register");
        return;
    }
    
    // Enable charging and disable OTG/5V
    PowerMgmtSetChargeEnable(true);
    PowerMgmtSetOtgEnable(false);
    is_otg_enabled = false;
    
    ESP_LOGI(TAG, "Shutting down...");

    // Set EN_1PIN bit to enter ship mode
    reg_value |= (1 << 1);  // EN_1PIN bit
    
    if (bq25895_write_byte(BQ_REG_POWER_ON, reg_value) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to shut down");
        return;
    }
}

// Emergency force reset - power off, stop all modules
void PowerMgmtEmergencyForceReset(void) {
    ESP_LOGE(TAG, "EMERGENCY FORCE RESET TRIGGERED");
    
    // Disable all power outputs
    PowerMgmtSetChargeEnable(false);
    PowerMgmtSetOtgEnable(false);
    PowerMgmtForce5v(false);
    is_otg_enabled = false;
    
    // Disable charging via BQ25895
    uint8_t reg_value = 0;
    bq25895_write_byte(BQ_REG_INPUT_SRC, 0x18);  // Disable USB charging + set HIZ
    
    // TODO: Add code to stop Wi-Fi, Bluetooth, and other peripherals
    // This would typically involve calling appropriate shutdown functions
    // for each subsystem
    
    // Infinite loop to prevent any further operation
    // In a real implementation, this would trigger a system reset or shutdown
    ESP_LOGE(TAG, "System halted - emergency reset");
    
    // For now, just disable everything and wait
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
