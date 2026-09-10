#include "i2c.h"
#include "hardware.h"
#include "driver/i2c_master.h"

i2c_master_bus_handle_t i2c_bus_handle;

esp_err_t I2cInit(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2cSda,
        .scl_io_num = I2cScl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    // Set bus frequency
    i2c_master_bus_add_device(i2c_bus_handle, "bq25895", 0x6B);  // BQ25895 I2C address
    i2c_master_bus_add_device(i2c_bus_handle, "mpu6500", 0x68);   // MPU-6500 I2C address

    return ESP_OK;
}

void I2cDeinit(void) {
    if (i2c_bus_handle != NULL) {
        i2c_del_master_bus(i2c_bus_handle);
        i2c_bus_handle = NULL;
    }
}
