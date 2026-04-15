---
id: "033"
title: "QtNetwork 入门：UDP 通信 (QUdpSocket/组播)"
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

掌握 Qt 中 UDP 网络通信的实现方法，包括 QUdpSocket 的使用、
单播、广播和组播 (Multicast) 的实现。理解 UDP 与 TCP 的区别与适用场景。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- UDP vs TCP：无连接、不可靠、数据报、低延迟
- QUdpSocket 基本使用：
  - bind / close
  - writeDatagram / readDatagram
  - readyRead 信号
  - hasPendingDatagrams / pendingDatagramSize
- 单播 (Unicast)：点对点发送
- 广播 (Broadcast)：setBroadcast(true), QHostAddress::Broadcast
- 组播 (Multicast)：
  - joinMulticastGroup / leaveMulticastGroup
  - setMulticastInterface
  - QHostAddress 组播地址范围 (224.0.0.0 - 239.255.255.255)
  - setSocketOption(QAbstractSocket::MulticastTtlOption)
- IPv4 vs IPv6 注意事项

踩坑重点：
1. UDP 数据报大小超过 MTU (约 1500 字节) 导致分片或丢失
2. bind 端口被占用导致失败但未检查返回值
3. 组播在非本地网络需要配置路由，否则数据包无法到达

练习项目：实现一个局域网发现工具，使用 UDP 广播发现局域网内的服务节点，
使用组播实现简单的群组消息通信。

## 涉及文件

- document/tutorials/beginner/04-qtnetwork/02-udp-communication-beginner.md
- examples/beginner/04-qtnetwork/02-udp-communication-beginner/

## 参考资料

- [QUdpSocket Class Reference](https://doc.qt.io/qt-6/qudpsocket.html)
- [Broadcast Sender Example](https://doc.qt.io/qt-6/qtnetwork-broadcastsender-example.html)
- [Multicast Sender Example](https://doc.qt.io/qt-6/qtnetwork-multicastsender-example.html)
