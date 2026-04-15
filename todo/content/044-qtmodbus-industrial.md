---
id: "044"
title: "其他模块：QtModbus 工业协议"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["032"]
blocks: []
estimated_effort: medium
---

## 目标

掌握 QtModbus 模块的使用，包括 Modbus TCP 和 Modbus RTU 通信。
理解 Modbus 协议基础，学会构建与工业设备通信的 Qt 应用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- Modbus 协议基础：
  - Modbus TCP vs Modbus RTU
  - 数据模型：线圈 (Coils)、离散输入 (Discrete Inputs)、
    保持寄存器 (Holding Registers)、输入寄存器 (Input Registers)
  - 功能码：01-06, 15-16
- QModbusClient (QModbusTcpClient / QModbusRtuSerialClient)：
  - connectDevice / disconnectDevice
  - sendReadRequest / sendWriteRequest / sendReadWriteRequest
  - stateChanged / errorOccurred 信号
- QModbusServer (QModbusTcpServer / QModbusRtuSerialServer)：
  - setServerAddress / listen
  - setData / data
  - 请求处理与响应
- QModbusDataUnit：数据单元
  - 数据类型 (RegisterType)
  - 起始地址与数量
  - values()
- QModbusReply：请求响应
  - finished 信号
  - result() / error()
- 寄存器地址映射与数据解析 (16位整数、32位浮点等)

踩坑重点：
1. Modbus RTU 串口参数 (波特率、校验) 与设备不匹配导致通信失败
2. 寄存器地址偏移 (0-based vs 1-based) 不同厂商实现不一致
3. 32位数据在两个 16位寄存器中的字节序 (大端/小端) 问题

练习项目：实现一个 Modbus TCP 客户端工具，
支持连接管理、寄存器读写、线圈操作、数据监控 (定时轮询)，
支持 Modbus RTU 串口模式。

## 涉及文件

- document/tutorials/beginner/05-other-modules/07-qtmodbus-industrial-beginner.md
- examples/beginner/05-other-modules/07-qtmodbus-industrial-beginner/

## 参考资料

- [Qt Modbus Module](https://doc.qt.io/qt-6/qtmodbus-index.html)
- [QModbusClient Class Reference](https://doc.qt.io/qt-6/qmodbusclient.html)
- [Modbus Example](https://doc.qt.io/qt-6/qtmodbus-modbus-example.html)
