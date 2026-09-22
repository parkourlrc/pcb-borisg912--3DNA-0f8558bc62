#include "hardware.h"
#include "i2c.h"
#include "esp_log.h"

static const char *TAG = "hardware";

// I2C bus handle (defined in hardware.h)
i2c_master_bus_handle_t i2c_bus_handle = NULL;

// Initialize hardware
void HardwareInit(void) {
    ESP_LOGI(TAG, "Initializing hardware");

    // Initialize I2C bus
    esp_err_t ret = I2cInit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Hardware initialized");
}

// Deinitialize hardware
void HardwareDeinit(void) {
    ESP_LOGI(TAG, "Deinitializing hardware");
    I2cDeinit();
    ESP_LOGI(TAG, "Hardware deinitialized");
}
