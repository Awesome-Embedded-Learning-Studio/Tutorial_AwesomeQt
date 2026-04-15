---
id: "062"
title: "iMX6ULL: 触摸屏校准与输入处理"
category: embedded
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["061"]
blocks: []
estimated_effort: medium
---

## 目标

编写 iMX6ULL 触摸屏校准与输入处理教程，涵盖 tslib 集成、多点触控和 evdev 配置。

## 验收标准

- [ ] tslib 交叉编译与集成
- [ ] 触摸屏校准流程（ts_calibrate）
- [ ] Qt tslib 插件配置
- [ ] evdev 输入设备配置
- [ ] 多点触控支持说明

## 实施说明

1. 交叉编译 tslib 库，集成到 rootfs
2. 运行 `ts_calibrate` 和 `ts_test` 进行校准和测试
3. 配置 Qt 的 tslib 插件：`QT_QPA_GENERIC_PLUGINS=tslib`
4. 设置 `TSLIB_TSDEVICE`、`TSLIB_CALIBFILE` 等环境变量
5. evdev 方式：配置 `QT_QPA_EVDEV_MOUSE_PARAMETERS` 直接读取 `/dev/input/eventX`
6. 多点触控：说明如何启用 `mtdev` 和 Qt 的多点触控协议支持
7. 包含常见的输入问题排查（坐标翻转、灵敏度等）

## 涉及文件

- `document/tutorials/embedded/imx6ull-touch-input.md`（新建）
- `examples/embedded/imx6ull/touch-demo/`（示例代码）

## 参考资料

- tslib: https://github.com/libts/tslib
- Qt 输入处理: https://doc.qt.io/qt-6/embedded-linux.html#touchscreen-integration
