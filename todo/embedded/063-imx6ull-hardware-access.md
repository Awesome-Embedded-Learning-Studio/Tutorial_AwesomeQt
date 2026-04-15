---
id: "063"
title: "iMX6ULL: GPIO/I2C/SPI 硬件访问"
category: embedded
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["061"]
blocks: []
estimated_effort: medium
---

## 目标

编写在 iMX6ULL 上通过 sysfs、spidev、QFile 方式访问硬件寄存器的教程（GPIO/I2C/SPI）。

## 验收标准

- [ ] GPIO sysfs 方式读写控制
- [ ] I2C 设备访问（SMBus/libi2c）
- [ ] SPI 设备访问（spidev）
- [ ] QFile 封装 sysfs 操作的 Qt 化方法
- [ ] 完整硬件交互示例（如控制 LED、读取传感器）

## 实施说明

1. **GPIO**：通过 `/sys/class/gpio/` 导出引脚，使用 `QFile` 读写 direction 和 value
2. **I2C**：使用 `/dev/i2c-N` 设备节点，通过 ioctl 或 libi2c 进行通信
3. **SPI**：配置内核 spidev 驱动，通过 `/dev/spidevN.M` 设备节点通信
4. 封装为 Qt 友好的类（继承 QObject，支持信号槽）
5. 示例项目：控制板载 LED + 读取 I2C 温度传感器 + SPI 显示屏驱动
6. 包含设备树（Device Tree）修改说明

## 涉及文件

- `document/tutorials/embedded/imx6ull-hardware-access.md`（新建）
- `examples/embedded/imx6ull/gpio-control/`（示例代码）
- `examples/embedded/imx6ull/i2c-sensor/`（示例代码）
- `examples/embedded/imx6ull/spi-display/`（示例代码）

## 参考资料

- Linux GPIO sysfs: https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
- Linux I2C dev-interface: https://www.kernel.org/doc/Documentation/i2c/dev-interface
- Linux SPI spidev: https://www.kernel.org/doc/Documentation/spi/spidev
