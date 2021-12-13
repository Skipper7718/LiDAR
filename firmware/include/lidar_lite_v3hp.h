#ifndef __LIDAR_CMD_H__
#define __LIDAR_CMD_H__

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// general Bus information, i2c_default is forr pi pico only
#define LIDAR_DEFAULT_ADDR          0x62
#define I2C_PORT                    i2c_default

// I2C commands from datasheet
#define LIDAR_ACQ_COMMAND			0x00
#define LIDAR_STATUS				0x01
#define LIDAR_SIG_COUNT_VAL			0x02
#define LIDAR_ACQ_CONFIG_REG		0x04
#define LIDAR_LEGACY_RESET_EN		0x06
#define LIDAR_SIGNAL_STRENGTH		0x0e
#define LIDAR_FULL_DELAY_HIGH		0x0f
#define LIDAR_FULL_DELAY_LOW		0x10
#define LIDAR_REF_COUNT_VAL			0x12
#define LIDAR_UNIT_ID_HIGH			0x16
#define LIDAR_UNIT_ID_LOW			0x17
#define LIDAR_I2C_ID_HIGH			0x18
#define LIDAR_I2C_ID_LOW			0x19
#define LIDAR_I2C_SEC_ADDR			0x1a
#define LIDAR_THRESHHOLD_BYPASS		0x1c
#define LIDAR_I2C_CONFIG			0x1e
#define LIDAR_PEAK_STACK_HIGH_BYTE	0x26
#define LIDAR_PEAK_STACK_LOW_BYTE	0x27
#define LIDAR_COMMAND				0x40
#define LIDAR_HEALTH_STATUS			0x48
#define LIDAR_CORR_DATA				0x52
#define LIDAR_CORR_DATA_SIGN		0x53
#define LIDAR_POWER_CONTROL			0x65

/**
 * @brief initialize i2c bus of the pi pico
 * It is not necessary is bus is already set up
 */
void init_lidar(int sda, int scl);

/**
 * @brief read lidar measurement in cm. Blocking.
 */
int lidar_get_measurement();

/**
 * @brief write read command and read to buffer
 */
int i2c_read_address(i2c_inst_t *i2c, uint8_t address, uint8_t mem_addr, uint8_t *buf, size_t len);


#ifdef __cplusplus
}
#endif

#endif
