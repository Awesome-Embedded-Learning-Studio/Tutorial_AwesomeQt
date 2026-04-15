---
id: "045"
title: "其他模块：QtMqtt 物联网协议"
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

掌握 QtMqtt 模块的使用，包括 MQTT 客户端连接、主题订阅/发布、
QoS 等级、遗嘱消息等。学会构建基于 MQTT 的物联网通信应用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- MQTT 协议基础：
  - 发布/订阅模式
  - Broker / Client 架构
  - Topic 主题与通配符 (+, #)
  - QoS 等级：0 (至多一次), 1 (至少一次), 2 (恰好一次)
  - 遗嘱消息 (Will Message)
  - 保持连接 (Keep Alive)
- QMqttClient：
  - setHostname / setPort / setClientId
  - setUsername / setPassword
  - setKeepAlive / setWillTopic / setWillMessage
  - connectToHost / disconnectFromHost
  - stateChanged / connected / disconnected 信号
  - publish (QoS, retain)
  - subscribe / unsubscribe
  - messageReceived 信号
- QMqttSubscription：
  - stateChanged / messageReceived
  - qos() 确认等级
- QMqttMessage：消息对象
  - topic, payload, qos, retain
- SSL/TLS 安全连接
- 主题过滤与消息路由设计
- CMake 配置：find_package(Qt6 REQUIRED COMPONENTS Mqtt)

踩坑重点：
1. QoS 2 消息在弱网环境下性能差，需根据场景选择合适的 QoS
2. 订阅主题通配符使用不当导致收到大量无关消息
3. Keep Alive 设置过短导致频繁断连重连

练习项目：实现一个 MQTT 监控面板，连接公共 MQTT Broker，
订阅多个传感器主题，实时显示温湿度数据，支持发送控制命令，
使用图表展示历史数据趋势。

## 涉及文件

- document/tutorials/beginner/05-other-modules/08-qtmqtt-iot-beginner.md
- examples/beginner/05-other-modules/08-qtmqtt-iot-beginner/

## 参考资料

- [Qt MQTT Module](https://doc.qt.io/qt-6/qtmqtt-index.html)
- [QMqttClient Class Reference](https://doc.qt.io/qt-6/qmqttclient.html)
- [MQTT v5.0 Specification](https://docs.oasis-open.org/mqtt/mqtt/v5.0/mqtt-v5.0.html)
