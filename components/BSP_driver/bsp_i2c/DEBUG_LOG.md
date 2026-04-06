# BSP I2C 底层驱动解耦日志

## 文档信息
- **创建日期**: 2026-04-01
- **项目路径**: `c:\Users\S105\Desktop\zjl\esp32-S3\blink\components\BSP_driver\bsp_i2c\`
- **目的**: 解耦ESP32 I2C原生库函数，便于后续移植到其他平台

---

## 修改记录

### 2026-04-01 - 初始创建 BSP I2C 抽象层

#### 1. 创建 `bsp_i2c.h` 头文件

**文件路径**: `components/BSP_driver/bsp_i2c/bsp_i2c.h`

**主要内容**:

1. **平台抽象层宏定义** - 解耦ESP-IDF原生库函数:
   - `BSP_I2C_HANDLE_TYPE` - I2C句柄类型抽象
   - `BSP_I2C_HOST_INIT` - I2C主机初始化函数抽象
   - `BSP_I2C_HOST_DEINIT` - I2C主机反初始化函数抽象
   - `BSP_I2C_HOST_WRITE` - I2C写入函数抽象
   - `BSP_I2C_HOST_READ` - I2C读取函数抽象
   - `BSP_I2C_HOST_WRITE_THEN_READ` - I2C复合操作函数抽象
   - `BSP_I2C_HOST_PROBE` - I2C设备探测函数抽象

2. **默认配置宏定义**:
   - `BSP_I2C_PORT_DEFAULT` - 默认端口号 (0)
   - `BSP_I2C_FREQ_HZ_DEFAULT` - 默认频率 (400kHz Fast Mode)
   - `BSP_I2C_TIMEOUT_MS_DEFAULT` - 默认超时 (1000ms)
   - `BSP_I2C_SDA_PIN_DEFAULT` - 默认SDA引脚 (GPIO21)
   - `BSP_I2C_SCL_PIN_DEFAULT` - 默认SCL引脚 (GPIO22)

3. **错误码定义**:
   - `BSP_I2C_OK` - 成功
   - `BSP_I2C_ERR_TIMEOUT` - 超时错误
   - `BSP_I2C_ERR_NACK` - 从机未应答
   - `BSP_I2C_ERR_BUS` - 总线错误
   - `BSP_I2C_ERR_PARAM` - 参数错误
   - `BSP_I2C_ERR_NO_INIT` - 未初始化
   - `BSP_I2C_ERR_BUSY` - 总线忙

4. **数据结构定义**:
   - `bsp_i2c_dev_config_t` - I2C设备配置结构体
   - `bsp_i2c_host_handle_t` - I2C主机句柄类型

5. **API函数声明**:
   - `bsp_i2c_host_init()` - 初始化I2C主机总线
   - `bsp_i2c_host_deinit()` - 反初始化I2C主机总线
   - `bsp_i2c_write()` - 向从机写入数据
   - `bsp_i2c_read()` - 从从机读取数据
   - `bsp_i2c_write_then_read()` - 复合操作(先写后读)
   - `bsp_i2c_probe()` - 探测从机设备是否存在
   - `bsp_i2c_scan()` - 扫描总线上的所有设备

---

#### 2. 创建 `bsp_i2c.c` 实现文件

**文件路径**: `components/BSP_driver/bsp_i2c/bsp_i2c.c`

**主要内容**:

1. **内部辅助函数**:
   - `i2c_port_get_from_handle()` - 从句柄获取I2C端口号
   - `esp_err_to_bsp_err()` - ESP错误码转换为BSP统一错误码

2. **API接口实现**:
   - `bsp_i2c_host_init()` - 使用 `i2c_master_bus_add_device()` 初始化总线
   - `bsp_i2c_host_deinit()` - 使用 `i2c_master_bus_rm_device()` 移除设备
   - `bsp_i2c_write()` - 使用 `i2c_master_transmit()` 发送数据
   - `bsp_i2c_read()` - 使用 `i2c_master_receive()` 接收数据
   - `bsp_i2c_write_then_read()` - 使用 `i2c_master_transmit_receive()` 复合操作
   - `bsp_i2c_probe()` - 使用 `i2c_master_probe()` 探测设备
   - `bsp_i2c_scan()` - 遍历地址0x08-0x77扫描所有设备

---

## 移植指南

### 如何移植到其他平台

当需要将本驱动移植到其他芯片平台时，只需修改 `bsp_i2c.h` 中的宏定义即可：

1. **替换句柄类型**:
   ```c
   // 将 ESP32 的 i2c_device_handle_t 替换为目标平台的句柄类型
   #define BSP_I2C_HANDLE_TYPE           your_platform_handle_type
   ```

2. **替换函数调用**:
   ```c
   // 将 ESP-IDF 函数替换为目标平台的等效函数
   #define BSP_I2C_HOST_INIT             your_platform_i2c_init
   #define BSP_I2C_HOST_WRITE            your_platform_i2c_write
   #define BSP_I2C_HOST_READ             your_platform_i2c_read
   // ... 以此类推
   ```

3. **调整默认引脚和参数**:
   ```c
   // 根据目标硬件调整默认引脚配置
   #define BSP_I2C_SDA_PIN_DEFAULT       (your_sda_pin)
   #define BSP_I2C_SCL_PIN_DEFAULT       (your_scl_pin)
   ```

4. **修改 `bsp_i2c.c` 实现**:
   - 实现与目标平台I2C外设交互的具体代码
   - 保持API接口不变，确保上层代码兼容

---

## 设计说明

### 为什么使用宏定义解耦？

1. **平台无关性**: 上层应用代码调用 `bsp_i2c_write()` 时，不需要关心底层是 ESP32 还是 STM32
2. **易于维护**: 当 ESP-IDF API 变更时，只需修改宏定义，无需改动业务代码
3. **便于测试**: 可以通过宏替换实现Mock函数，方便单元测试
4. **代码复用**: 同一套上层代码可用于多个平台

### 错误处理策略

- 所有函数都包含参数合法性检查
- 使用统一的错误码便于调用者判断
- 错误时返回具体错误码，成功返回 BSP_I2C_OK (0)
- 使用 ESP_LOG 系列函数输出调试信息

---

## 后续计划

- [ ] 添加更多错误码详细信息
- [ ] 支持10位地址模式
- [ ] 添加DMA支持选项
- [ ] 添加中断模式支持
- [ ] 编写使用示例代码
