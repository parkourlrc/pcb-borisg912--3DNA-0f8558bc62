#pragma once

#include <stdint.h>
#include "mpu6050.h"

// Initialize gyroscope (MPU-6500)
void GyroInit(void);

// Deinitialize gyroscope
void GyroDeinit(void);

// Read accelerometer and gyroscope data
void GyroReadData(float *accel_x, float *accel_y, float *accel_z,
                  float *gyro_x, float *gyro_y, float *gyro_z);

// Get raw sensor data (integer values)
void GyroReadRawData(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z,
                     int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z);

// Get MPU6050 device handle (read-only)
mpu6050_handle_t GyroGetDeviceHandle(void);

// Check if gyroscope is initialized
bool GyroIsInitialized(void);
