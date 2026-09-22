idf_component_register(SRCS "gyro.c"
                    INCLUDE_DIRS "include")

# Add dependency on Espressif's MPU6050 driver
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components/mpu6050)
