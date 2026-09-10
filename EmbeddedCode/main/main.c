#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "i2c.h"
#include "powermgmt.h"
#include "audio.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing I2C...");
    esp_err_t ret = I2cInit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "I2C initialized successfully");

    ESP_LOGI(TAG, "Initializing Power Management...");
    PowerMgmtInit();
    ESP_LOGI(TAG, "Power Management initialized");

    ESP_LOGI(TAG, "Initializing Audio Player...");
    PlayerInit();
    ESP_LOGI(TAG, "Audio Player initialized");

    ChargeState last_state = ChargeIdle;
    ChargeState current_state;

    while (1) {
        current_state = PowerMgmtReadState();
        
        if (current_state != last_state) {
            ESP_LOGI(TAG, "BMS state changed from %d to %d", last_state, current_state);
            PlayerPlayTrack(2);
            last_state = current_state;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}