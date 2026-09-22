#include "audio.h"
#include "hardware.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "audio";

// DFPlayer Mini UART configuration
#define DFPLAYER_UART_PORT    UART_NUM_1
#define DFPLAYER_UART_BAUDRATE 9600
#define DFPLAYER_TXD_PIN      AudioPlayerTx

// DFPlayer Mini command codes
#define DF_PLAYER_CMD_PLAY_TRACK       0x03
#define DF_PLAYER_CMD_SET_VOLUME       0x06
#define DF_PLAYER_CMD_PAUSE            0x0E
#define DF_PLAYER_CMD_POWER_SAVER_ON   0x0C
#define DF_PLAYER_CMD_POWER_SAVER_OFF  0x0D

static uart_port_t uart_port = -1;
static bool player_initialized = false;

// Initialize UART for DFPlayer Mini
static esp_err_t init_uart(void) {
    uart_config_t uart_config = {
        .baud_rate = DFPLAYER_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t ret = uart_param_config(DFPLAYER_UART_PORT, &uart_config);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = uart_set_pin(DFPLAYER_UART_PORT, UART_PIN_NO_CHANGE, 
                       DFPLAYER_TXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = uart_driver_install(DFPLAYER_UART_PORT, 256, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    uart_port = DFPLAYER_UART_PORT;
    return ESP_OK;
}

// Send command to DFPlayer Mini
static void send_command(uint8_t cmd, uint16_t param) {
    if (uart_port == -1) {
        ESP_LOGE(TAG, "UART not initialized");
        return;
    }

    // DFPlayer Mini command format:
    // Start byte (0x7E), Version (0xFF), Length (0x06), Command, Feedback (0x00),
    // Parameter High, Parameter Low, Checksum, End byte (0xEF)
    uint8_t command[10] = {
        0x7E,          // Start byte
        0xFF,          // Version
        0x06,          // Length
        cmd,           // Command
        0x00,          // Feedback (0 = no feedback)
        (uint8_t)(param >> 8),  // Parameter high byte
        (uint8_t)(param & 0xFF), // Parameter low byte
        0x00,          // Checksum placeholder
        0xEF           // End byte
    };

    // Calculate checksum (sum of bytes 1-6)
    uint8_t checksum = 0;
    for (int i = 1; i <= 6; i++) {
        checksum += command[i];
    }
    command[7] = -checksum;

    // Send command
    uart_write_bytes(uart_port, command, 10);
    ESP_LOGD(TAG, "Sent command: 0x%02X, param: 0x%04X", cmd, param);
}

// Initialize audio player (DFPlayer Mini)
void PlayerInit(void) {
    if (player_initialized) {
        ESP_LOGI(TAG, "Audio player already initialized");
        return;
    }

    ESP_LOGI(TAG, "Initializing DFPlayer Mini");

    if (init_uart() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UART for DFPlayer");
        return;
    }

    // Wait for DFPlayer to initialize
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Set default volume
    PlayerSetVolume(20);

    player_initialized = true;
    ESP_LOGI(TAG, "DFPlayer Mini initialized");
}

// Send raw command to DFPlayer Mini
void PlayerSendCmd(uint8_t command, uint16_t parameter) {
    if (!player_initialized) {
        ESP_LOGW(TAG, "Audio player not initialized");
        return;
    }
    send_command(command, parameter);
}

// Play a specific track by number
void PlayerPlayTrack(uint16_t trackNumber) {
    if (!player_initialized) {
        ESP_LOGW(TAG, "Audio player not initialized");
        return;
    }
    send_command(DF_PLAYER_CMD_PLAY_TRACK, trackNumber);
    ESP_LOGD(TAG, "Playing track %d", trackNumber);
}

// Play a predefined sound effect
void PlayerPlaySound(SoundEffect effect) {
    if (!player_initialized) {
        ESP_LOGW(TAG, "Audio player not initialized");
        return;
    }
    uint16_t track = PersonalizationGetSoundTrack(effect);
    PlayerPlayTrack(track);
}

// Pause playback
void PlayerPause(void) {
    if (!player_initialized) {
        ESP_LOGW(TAG, "Audio player not initialized");
        return;
    }
    send_command(DF_PLAYER_CMD_PAUSE, 0);
    ESP_LOGD(TAG, "Playback paused");
}

// Set volume (0-30)
void PlayerSetVolume(uint8_t volume) {
    if (!player_initialized) {
        ESP_LOGW(TAG, "Audio player not initialized");
        return;
    }
    if (volume > 30) volume = 30;
    send_command(DF_PLAYER_CMD_SET_VOLUME, volume);
    ESP_LOGD(TAG, "Volume set to %d", volume);
}

// Enter power save mode
void PlayerEnterPowerSaver(void) {
    if (!player_initialized) {
        ESP_LOGW(TAG, "Audio player not initialized");
        return;
    }
    send_command(DF_PLAYER_CMD_POWER_SAVER_ON, 0);
    ESP_LOGD(TAG, "Entered power save mode");
}

// Exit power save mode
void PlayerExitPowerSaver(void) {
    if (!player_initialized) {
        ESP_LOGW(TAG, "Audio player not initialized");
        return;
    }
    send_command(DF_PLAYER_CMD_POWER_SAVER_OFF, 0);
    ESP_LOGD(TAG, "Exited power save mode");
}

// Check if audio player is initialized
bool PlayerIsInitialized(void) {
    return player_initialized && uart_port != -1;
}
