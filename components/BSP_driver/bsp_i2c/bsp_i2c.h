/**
 * @file bsp_i2c.h
 * @brief BSP I2C 底层驱动抽象层 - 解耦ESP32原生库函数
 *
 * 本文件提供I2C底层接口的抽象层，通过宏定义解耦ESP-IDF的原生I2C驱动，
 * 方便后续移植到其他平台或单片机。
 *
 * @date 2026-04-01
 */

#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#include <stdint.h>
#include <stdbool.h>

/*=======================================
 * 平台相关头文件 - ESP32
 *=======================================
 * 说明: 根据不同平台包含对应的头文件
 *       移植时只需修改此处的#include
 */

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*=======================================
 * I2C 端口和速度配置宏
 *=======================================
 */

#define BSP_I2C_PORT_DEFAULT          (0)
#define BSP_I2C_FREQ_HZ_DEFAULT       (400000)
#define BSP_I2C_TIMEOUT_MS_DEFAULT    (1000)
#define BSP_I2C_SDA_PIN_DEFAULT       (21)
#define BSP_I2C_SCL_PIN_DEFAULT       (22)

/*=======================================
 * I2C 错误码定义
 *=======================================
 */

#define BSP_I2C_OK                    (0)
#define BSP_I2C_ERR_TIMEOUT           (-1)
#define BSP_I2C_ERR_NACK              (-2)
#define BSP_I2C_ERR_BUS               (-3)
#define BSP_I2C_ERR_PARAM             (-4)
#define BSP_I2C_ERR_NO_INIT           (-5)
#define BSP_I2C_ERR_BUSY              (-6)

/*=======================================
 * I2C 设备配置结构体
 *=======================================
 */

typedef struct {
    uint8_t dev_addr;
    uint32_t freq_hz;
    uint16_t timeout_ms;
} bsp_i2c_dev_config_t;

typedef void* bsp_i2c_host_handle_t;

/*=======================================
 * 函数声明
 *=======================================
 */

bsp_i2c_host_handle_t bsp_i2c_host_init(uint8_t port, uint8_t sda_pin, uint8_t scl_pin, uint32_t freq_hz);
int bsp_i2c_host_deinit(bsp_i2c_host_handle_t handle);
int bsp_i2c_write(bsp_i2c_host_handle_t handle, uint8_t dev_addr, const uint8_t *data, uint16_t size, uint16_t timeout_ms);
int bsp_i2c_read(bsp_i2c_host_handle_t handle, uint8_t dev_addr, uint8_t *data, uint16_t size, uint16_t timeout_ms);
int bsp_i2c_write_then_read(bsp_i2c_host_handle_t handle, uint8_t dev_addr,
                            const uint8_t *write_data, uint16_t write_size,
                            uint8_t *read_data, uint16_t read_size,
                            uint16_t timeout_ms);
bool bsp_i2c_probe(bsp_i2c_host_handle_t handle, uint8_t dev_addr, uint16_t timeout_ms);
uint8_t bsp_i2c_scan(bsp_i2c_host_handle_t handle, uint8_t *found_addr, uint8_t max_cnt);

#endif /* __BSP_I2C_H__ */
