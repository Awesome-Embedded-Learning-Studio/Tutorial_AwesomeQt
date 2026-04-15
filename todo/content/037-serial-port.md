---
id: "037"
title: "QtNetwork 入门：串口通信 (QSerialPort)"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: medium
---

## 目标

掌握 Qt 串口通信的实现方法，包括 QSerialPort 和 QSerialPortInfo 的使用。
学会配置串口参数、收发数据、处理串口事件，构建与硬件设备的通信应用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- 串口通信基础概念：波特率、数据位、校验位、停止位、流控
- QSerialPortInfo：枚举可用串口
  - availablePorts
  - portName, description, manufacturer, serialNumber
  - hasProductIdentifier, hasVendorIdentifier
- QSerialPort：串口操作
  - setPort / setPortName
  - setBaudRate, setDataBits, setParity, setStopBits, setFlowControl
  - open / close
  - readyRead 信号
  - write / read / readAll
  - clear / flush
  - errorOccurred 信号
- 数据帧解析与协议处理
  - 固定长度帧
  - 起止符帧 (如 STX/ETX)
  - CRC 校验
- 定时器配合：轮询 vs 事件驱动
- CMake 配置：find_package(Qt6 REQUIRED COMPONENTS SerialPort)

踩坑重点：
1. 波特率不匹配导致接收到乱码
2. 串口打开失败但未检查返回值导致后续 write/read 崩溃
3. 未设置读缓冲区大小导致大数据量时数据丢失

练习项目：实现一个串口调试助手，支持串口枚举与选择、参数配置、
HEX/ASCII 模式收发显示、自动发送、数据保存功能。

## 涉及文件

- document/tutorials/beginner/04-qtnetwork/06-serial-port-beginner.md
- examples/beginner/04-qtnetwork/06-serial-port-beginner/

## 参考资料

- [QSerialPort Class Reference](https://doc.qt.io/qt-6/qserialport.html)
- [QSerialPortInfo Class Reference](https://doc.qt.io/qt-6/qserialportinfo.html)
- [Qt Serial Port Module](https://doc.qt.io/qt-6/qtserialport-index.html)
