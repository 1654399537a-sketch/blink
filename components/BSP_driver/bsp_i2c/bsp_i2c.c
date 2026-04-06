#include "bsp_i2c.h"
 //本文件实现ESP32平台的I2C底层驱动



static const char *TAG = "BSP_I2C";

/*=======================================
 * 内部辅助函数
 *=======================================
 */

static int esp_err_to_bsp_err(esp_err_t esp_err)
{
    switch (esp_err) {
        case ESP_OK:
            return BSP_I2C_OK;
        case ESP_ERR_TIMEOUT:
            return BSP_I2C_ERR_TIMEOUT;
        case ESP_ERR_NOT_FOUND:
            return BSP_I2C_ERR_NACK;
        case ESP_ERR_INVALID_ARG:
            return BSP_I2C_ERR_PARAM;
        default:
            return BSP_I2C_ERR_BUS;
    }
}

/*=======================================
 * BSP I2C API 接口实现
 *=======================================
 */

bsp_i2c_host_handle_t bsp_i2c_host_init(uint8_t port, uint8_t sda_pin, uint8_t scl_pin, uint32_t freq_hz)
{
    if (port >= I2C_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid I2C port number: %d", port);
        return NULL;
    }

    i2c_master_bus_handle_t bus_handle = NULL;

    i2c_master_bus_config_t bus_config = {
        .i2c_port = port,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    esp_err_t ret = i2c_master_bus_add_device(&bus_config, &bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add I2C bus: %s", esp_err_to_name(ret));
        return NULL;
    }

    ret = i2c_master_bus_set_clock(bus_handle, freq_hz);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set I2C clock: %s", esp_err_to_name(ret));
        return NULL;
    }

    ESP_LOGI(TAG, "I2C bus initialized: port=%d, SDA=%d, SCL=%d, freq=%lu Hz",
             port, sda_pin, scl_pin, freq_hz);

    return (bsp_i2c_host_handle_t)bus_handle;
}

int bsp_i2c_host_deinit(bsp_i2c_host_handle_t handle)
{
    if (handle == NULL) {
        return BSP_I2C_ERR_PARAM;
    }

    i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)handle;
    esp_err_t ret = i2c_master_bus_rm_device(bus_handle);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove I2C bus: %s", esp_err_to_name(ret));
    }

    return esp_err_to_bsp_err(ret);
}

int bsp_i2c_write(bsp_i2c_host_handle_t handle, uint8_t dev_addr, const uint8_t *data, uint16_t size, uint16_t timeout_ms)
{
    if (handle == NULL || data == NULL || size == 0) {
        return BSP_I2C_ERR_PARAM;
    }

    i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)handle;

    i2c_device_config_t dev_config = {
        .dev_addr = dev_addr,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .timeout = timeout_ms / portTICK_PERIOD_MS,
        .flags = {
            .disable_ack_check = false,
        },
    };

    i2c_device_handle_t dev_handle = NULL;
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device for write: %s", esp_err_to_name(ret));
        return esp_err_to_bsp_err(ret);
    }

    ret = i2c_master_transmit(dev_handle, data, size, timeout_ms / portTICK_PERIOD_MS);

    i2c_master_bus_rm_device(dev_handle);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C write failed: addr=0x%02X, size=%d, err=%s",
                 dev_addr, size, esp_err_to_name(ret));
    }

    return esp_err_to_bsp_err(ret);
}

int bsp_i2c_read(bsp_i2c_host_handle_t handle, uint8_t dev_addr, uint8_t *data, uint16_t size, uint16_t timeout_ms)
{
    if (handle == NULL || data == NULL || size == 0) {
        return BSP_I2C_ERR_PARAM;
    }

    i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)handle;

    i2c_device_config_t dev_config = {
        .dev_addr = dev_addr,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .timeout = timeout_ms / portTICK_PERIOD_MS,
        .flags = {
            .disable_ack_check = false,
        },
    };

    i2c_device_handle_t dev_handle = NULL;
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device for read: %s", esp_err_to_name(ret));
        return esp_err_to_bsp_err(ret);
    }

    ret = i2c_master_receive(dev_handle, data, size, timeout_ms / portTICK_PERIOD_MS);

    i2c_master_bus_rm_device(dev_handle);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C read failed: addr=0x%02X, size=%d, err=%s",
                 dev_addr, size, esp_err_to_name(ret));
    }

    return esp_err_to_bsp_err(ret);
}

int bsp_i2c_write_then_read(bsp_i2c_host_handle_t handle, uint8_t dev_addr,
                            const uint8_t *write_data, uint16_t write_size,
                            uint8_t *read_data, uint16_t read_size,
                            uint16_t timeout_ms)
{
    if (handle == NULL || write_data == NULL || read_data == NULL) {
        return BSP_I2C_ERR_PARAM;
    }

    if (write_size == 0 || read_size == 0) {
        return BSP_I2C_ERR_PARAM;
    }

    i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)handle;

    i2c_device_config_t dev_config = {
        .dev_addr = dev_addr,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .timeout = timeout_ms / portTICK_PERIOD_MS,
        .flags = {
            .disable_ack_check = false,
        },
    };

    i2c_device_handle_t dev_handle = NULL;
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device for write_then_read: %s", esp_err_to_name(ret));
        return esp_err_to_bsp_err(ret);
    }

    ret = i2c_master_transmit_receive(dev_handle, write_data, write_size, read_data, read_size,
                                      timeout_ms / portTICK_PERIOD_MS);

    i2c_master_bus_rm_device(dev_handle);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C write_then_read failed: addr=0x%02X, wsize=%d, rsize=%d, err=%s",
                 dev_addr, write_size, read_size, esp_err_to_name(ret));
    }

    return esp_err_to_bsp_err(ret);
}

bool bsp_i2c_probe(bsp_i2c_host_handle_t handle, uint8_t dev_addr, uint16_t timeout_ms)
{
    if (handle == NULL) {
        ESP_LOGE(TAG, "Probe failed: invalid handle");
        return false;
    }

    i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)handle;

    i2c_device_config_t dev_config = {
        .dev_addr = dev_addr,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .timeout = timeout_ms / portTICK_PERIOD_MS,
        .flags = {
            .disable_ack_check = false,
        },
    };

    i2c_device_handle_t dev_handle = NULL;
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Probe: failed to add device 0x%02X: %s", dev_addr, esp_err_to_name(ret));
        return false;
    }

    ret = i2c_master_probe(bus_handle, dev_addr, timeout_ms / portTICK_PERIOD_MS);

    i2c_master_bus_rm_device(dev_handle);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Probe: device found at addr=0x%02X", dev_addr);
        return true;
    } else {
        ESP_LOGI(TAG, "Probe: no device found at addr=0x%02X", dev_addr);
        return false;
    }
}

uint8_t bsp_i2c_scan(bsp_i2c_host_handle_t handle, uint8_t *found_addr, uint8_t max_cnt)
{
    if (handle == NULL || found_addr == NULL || max_cnt == 0) {
        ESP_LOGE(TAG, "Scan failed: invalid parameters");
        return 0;
    }

    uint8_t found_count = 0;
    uint8_t addr;

    ESP_LOGI(TAG, "Starting I2C scan...");

    for (addr = 0x08; addr <= 0x77; addr++) {
        if (bsp_i2c_probe(handle, addr, BSP_I2C_TIMEOUT_MS_DEFAULT)) {
            if (found_count < max_cnt) {
                found_addr[found_count] = addr;
                found_count++;
                ESP_LOGI(TAG, "  Found device at 0x%02X", addr);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "I2C scan complete: found %d device(s)", found_count);

    return found_count;
}
