#include "gyro.h"
#include "hardware.h"
#include "i2c.h"
#include "esp_log.h"

static const char *TAG = "gyro";

// MPU-6500 device handle
static mpu6050_handle_t mpu6050_dev = NULL;
static bool gyro_initialized = false;

// Initialize gyroscope (MPU-6500)
void GyroInit(void) {
    if (gyro_initialized) {
        ESP_LOGI(TAG, "Gyro already initialized");
        return;
    }

    ESP_LOGI(TAG, "Initializing MPU-6500 gyroscope");

    // Configure MPU-6500
    mpu6050_config_t mpu_config = {
        .i2c_port = I2C_PORT,
        .i2c_addr = MPU6050_I2C_ADDRESS_HIGH,  // AD0 = 1 (0x69)
        .sda_io_num = I2cSda,
        .scl_io_num = I2cScl,
        .scl_speed_hz = I2cFreq,
    };

    esp_err_t ret = mpu6050_new_device(&mpu_config, &mpu6050_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create MPU-6500 device: %s", esp_err_to_name(ret));
        return;
    }

    // Initialize the sensor
    ret = mpu6050_init(mpu6050_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MPU-6500: %s", esp_err_to_name(ret));
        mpu6050_del_device(mpu6050_dev);
        mpu6050_dev = NULL;
        return;
    }

    // Configure gyro range (optional)
    ret = mpu6050_set_gyro_full_scale(mpu6050_dev, MPU6050_GYRO_FS_250);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set gyro range: %s", esp_err_to_name(ret));
    }

    // Configure accelerometer range (optional)
    ret = mpu6050_set_accel_full_scale(mpu6050_dev, MPU6050_ACCEL_FS_2);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set accel range: %s", esp_err_to_name(ret));
    }

    // Configure digital low-pass filter (optional)
    ret = mpu6050_set_dlpf(mpu6050_dev, MPU6050_DLPF_6);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set DLPF: %s", esp_err_to_name(ret));
    }

    gyro_initialized = true;
    ESP_LOGI(TAG, "MPU-6500 gyroscope initialized successfully");
}

// Deinitialize gyroscope
void GyroDeinit(void) {
    if (!gyro_initialized || mpu6050_dev == NULL) {
        return;
    }

    ESP_LOGI(TAG, "Deinitializing MPU-6500 gyroscope");
    mpu6050_del_device(mpu6050_dev);
    mpu6050_dev = NULL;
    gyro_initialized = false;
    ESP_LOGI(TAG, "MPU-6500 gyroscope deinitialized");
}

// Read accelerometer and gyroscope data
void GyroReadData(float *accel_x, float *accel_y, float *accel_z,
                  float *gyro_x, float *gyro_y, float *gyro_z) {
    if (!gyro_initialized || mpu6050_dev == NULL) {
        if (accel_x) *accel_x = 0;
        if (accel_y) *accel_y = 0;
        if (accel_z) *accel_z = 0;
        if (gyro_x) *gyro_x = 0;
        if (gyro_y) *gyro_y = 0;
        if (gyro_z) *gyro_z = 0;
        return;
    }

    int16_t raw_accel[3] = {0};
    int16_t raw_gyro[3] = {0};

    esp_err_t ret = mpu6050_get_raw_data(mpu6050_dev, raw_accel, raw_gyro);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read MPU-6500 data: %s", esp_err_to_name(ret));
        return;
    }

    // Convert raw data to floating point values
    // Accelerometer: default sensitivity is 16384 LSB/g
    // Gyroscope: default sensitivity is 131 LSB/deg/s
    if (accel_x) *accel_x = raw_accel[0] / 16384.0f;
    if (accel_y) *accel_y = raw_accel[1] / 16384.0f;
    if (accel_z) *accel_z = raw_accel[2] / 16384.0f;
    if (gyro_x) *gyro_x = raw_gyro[0] / 131.0f;
    if (gyro_y) *gyro_y = raw_gyro[1] / 131.0f;
    if (gyro_z) *gyro_z = raw_gyro[2] / 131.0f;
}

// Get raw sensor data (integer values)
void GyroReadRawData(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z,
                     int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z) {
    if (!gyro_initialized || mpu6050_dev == NULL) {
        if (accel_x) *accel_x = 0;
        if (accel_y) *accel_y = 0;
        if (accel_z) *accel_z = 0;
        if (gyro_x) *gyro_x = 0;
        if (gyro_y) *gyro_y = 0;
        if (gyro_z) *gyro_z = 0;
        return;
    }

    int16_t raw_accel[3] = {0};
    int16_t raw_gyro[3] = {0};

    esp_err_t ret = mpu6050_get_raw_data(mpu6050_dev, raw_accel, raw_gyro);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read MPU-6500 raw data: %s", esp_err_to_name(ret));
        return;
    }

    if (accel_x) *accel_x = raw_accel[0];
    if (accel_y) *accel_y = raw_accel[1];
    if (accel_z) *accel_z = raw_accel[2];
    if (gyro_x) *gyro_x = raw_gyro[0];
    if (gyro_y) *gyro_y = raw_gyro[1];
    if (gyro_z) *gyro_z = raw_gyro[2];
}

// Get MPU6050 device handle (read-only)
mpu6050_handle_t GyroGetDeviceHandle(void) {
    return mpu6050_dev;
}

// Check if gyroscope is initialized
bool GyroIsInitialized(void) {
    return gyro_initialized;
}
