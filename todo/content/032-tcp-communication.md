---
id: "032"
title: "QtNetwork 入门：TCP 通信 (QTcpServer/QTcpSocket)"
category: content
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: ["033", "035", "036", "044", "045"]
estimated_effort: medium
---

## 目标

掌握 Qt 中 TCP 网络通信的实现方法，包括 QTcpServer 服务端和 QTcpSocket 客户端的使用。
理解 TCP 连接建立、数据收发、连接关闭的完整流程，学会构建可靠的网络应用。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- TCP 基础概念回顾：三次握手、可靠传输、流式传输
- QTcpServer 服务端：
  - listen / close
  - newConnection 信号
  - nextPendingConnection 获取客户端 socket
- QTcpSocket 客户端：
  - connectToHost / disconnectFromHost
  - connected / disconnected / stateChanged 信号
  - readyRead 信号与数据读取
  - write / read / readAll / readLine
  - waitForConnected / waitForReadyRead / waitForDisconnected
- 数据分包与粘包处理
  - 消息头 + 消息体模式
  - QDataStream 的序列化与反序列化
- 错误处理：errorOccurred 信号
- 多客户端管理
- CMake 配置：find_package(Qt6 REQUIRED COMPONENTS Network)

踩坑重点：
1. TCP 粘包问题：一次 readyRead 可能包含多条消息或不完整消息
2. 忘记检查 bytesAvailable() 就直接 read 导致数据丢失
3. QTcpServer::listen 未检查返回值导致静默失败

练习项目：实现一个简易聊天室，包含服务端 (QTcpServer) 和多客户端 (QTcpSocket)，
支持用户登录、消息广播、私聊功能，使用消息头+消息体协议解决粘包问题。

## 涉及文件

- document/tutorials/beginner/04-qtnetwork/01-tcp-communication-beginner.md
- examples/beginner/04-qtnetwork/01-tcp-communication-beginner/

## 参考资料

- [QTcpServer Class Reference](https://doc.qt.io/qt-6/qtcpserver.html)
- [QTcpSocket Class Reference](https://doc.qt.io/qt-6/qtcpsocket.html)
- [Qt Network Programming](https://doc.qt.io/qt-6/qtnetwork-programming.html)
