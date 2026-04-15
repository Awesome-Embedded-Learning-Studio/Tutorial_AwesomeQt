---
id: "035"
title: "QtNetwork 入门：WebSocket 实时通信"
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

掌握 Qt WebSocket 模块的使用，包括 QWebSocket 客户端和 QWebSocketServer 服务端。
学会构建基于 WebSocket 的实时双向通信应用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- WebSocket 协议基础：ws/wss、握手、帧、ping/pong
- QWebSocket 客户端：
  - open / close / abort
  - sendTextMessage / sendBinaryMessage
  - textMessageReceived / binaryMessageReceived
  - connected / disconnected / errorOccurred
  - ping / pong 信号
- QWebSocketServer 服务端：
  - listen / close
  - newConnection 信号
  - QWebSocket *nextPendingConnection
  - 多客户端管理
- 消息协议设计：JSON 文本帧 vs 二进制帧
- QUrl 构造 WebSocket 地址
- SSL/WSS 安全连接
- CMake 配置：find_package(Qt6 REQUIRED COMPONENTS WebSockets)

踩坑重点：
1. QWebSocketServer 连接的 socket 需手动管理生命周期，忘记 deleteLater 导致泄漏
2. 大消息未分帧导致发送失败或延迟
3. wss 连接未处理 SSL 证书验证导致握手失败

练习项目：实现一个实时协作白板，服务端使用 QWebSocketServer 管理连接，
客户端使用 QWebSocket 实时同步绘图操作，支持多用户同时绘制。

## 涉及文件

- document/tutorials/beginner/04-qtnetwork/04-websocket-realtime-beginner.md
- examples/beginner/04-qtnetwork/04-websocket-realtime-beginner/

## 参考资料

- [QWebSocket Class Reference](https://doc.qt.io/qt-6/qwebsocket.html)
- [QWebSocketServer Class Reference](https://doc.qt.io/qt-6/qwebsocketserver.html)
- [Qt WebSockets Module](https://doc.qt.io/qt-6/qtwebsockets-index.html)
