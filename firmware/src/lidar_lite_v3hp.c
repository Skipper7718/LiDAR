#include "pico/stdlib.h"
#include "lidar_lite_v3hp.h"
#include "hardware/i2c.h"

void init_lidar(int sda, int scl) {
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
}

int lidar_get_measurement() {
    uint8_t cmd[2] = {LIDAR_ACQ_COMMAND, 0x04};

    //wait for next measurement to occur
    while(true) {
        i2c_write_blocking(I2C_PORT, LIDAR_DEFAULT_ADDR, cmd, 2, false);
        uint8_t status;
        i2c_read_address(I2C_PORT, LIDAR_DEFAULT_ADDR, LIDAR_STATUS, &status, 1);
        if( status & 0x01 == 1 )
            break;
    }

    uint8_t high_byte, low_byte;
    i2c_read_address(I2C_PORT, LIDAR_DEFAULT_ADDR, LIDAR_FULL_DELAY_HIGH, &high_byte, 1);
    i2c_read_address(I2C_PORT, LIDAR_DEFAULT_ADDR, LIDAR_FULL_DELAY_LOW, &low_byte, 1);
    int measurement = (((int) high_byte) << 8) | ((int) low_byte);
    return measurement;
}

int i2c_read_address(i2c_inst_t *i2c, uint8_t address, uint8_t mem_addr, uint8_t *buf, size_t len) {
    i2c_write_blocking(i2c, address, &mem_addr, 1, false);
    i2c_read_blocking(i2c, address, buf, len, false);
}
