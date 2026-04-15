---
id: "041"
title: "其他模块：QtBluetooth 蓝牙通信"
category: content
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: medium
---

## 目标

掌握 QtBluetooth 模块的使用，包括蓝牙设备扫描、经典蓝牙通信 (RFCOMM)
和低功耗蓝牙 (BLE) 操作。理解蓝牙协议栈基础与 Qt 蓝牙 API 的使用方式。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- 蓝牙基础：经典蓝牙 vs BLE (Bluetooth Low Energy)
- QBluetoothLocalDevice：本地蓝牙设备
- QBluetoothDeviceDiscoveryAgent：设备扫描
  - start / stop
  - deviceDiscovered / finished / error 信号
  - supportedDiscoveryMethods
- 经典蓝牙 (RFCOMM)：
  - QBluetoothServer / QBluetoothSocket
  - 监听与连接
  - 数据收发
- 低功耗蓝牙 (BLE)：
  - QLowEnergyController
  - QLowEnergyService
  - QLowEnergyCharacteristic / QLowEnergyDescriptor
  - 服务发现、特征读写、通知订阅
- QBluetoothUuid：UUID 管理
- 蓝牙权限配置 (Android/iOS)

踩坑重点：
1. 不同平台蓝牙 API 支持差异大，Linux/Windows/macOS/Android 行为不一致
2. BLE 读写操作需要等待前一个操作完成再发起下一个
3. 蓝牙扫描未获取位置权限 (Android) 导致扫描失败

练习项目：实现一个 BLE 设备扫描与控制工具，支持扫描附近 BLE 设备、
列举服务与特征值、读写特征值、订阅通知。

## 涉及文件

- document/tutorials/beginner/05-other-modules/04-qtbluetooth-beginner.md
- examples/beginner/05-other-modules/04-qtbluetooth-beginner/

## 参考资料

- [Qt Bluetooth Module](https://doc.qt.io/qt-6/qtbluetooth-index.html)
- [QLowEnergyController Class Reference](https://doc.qt.io/qt-6/qlowenergycontroller.html)
- [Bluetooth Examples](https://doc.qt.io/qt-6/qtbluetooth-examples.html)
