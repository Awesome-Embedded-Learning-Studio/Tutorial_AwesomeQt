---
title: "04 · QtNetwork 网络编程（专家）"
description: "深入网络模块源码：QAbstractSocket 十一态状态机、QUdpSocket 数据报平台封装、QNetworkAccessManager 可插拔后端与请求队列、QWebSocket RFC 6455 帧解析、SSL OpenSSL/Schannel 抽象层、QSerialPort POSIX/Win32 封装。共 6 篇。"
---

# 04 · QtNetwork 网络编程（专家）

> 规划中（6 篇），敬请期待...

## 章节规划

- **01 QAbstractSocket 源码** — 十一态状态机转换、非阻塞 connect 跨平台实现、`QSocketNotifier` 异步驱动
- **02 QUdpSocket 源码** — `recvfrom`/`sendto` 封装、多播 `IP_ADD_MEMBERSHIP` 平台差异、`QNetworkDatagram` TTL 传递
- **03 QNetworkAccessManager 源码** — `QNetworkAccessBackend` 可插拔后端、HTTP/2 多路复用、请求优先级队列、`QNetworkDiskCache` LRU 淘汰
- **04 QWebSocket 源码** — RFC 6455 帧结构解析、客户端掩码 XOR、分片帧重组缓冲、控制帧优先处理
- **05 SSL 源码** — `QSslSocketBackendPrivate` 后端选择、TLS 握手状态管理、证书链验证、ALPN 协议协商
- **06 QSerialPort 源码** — `termios`/`DCB` 配置、`QWinOverlappedIoNotifier` vs `QSocketNotifier`、硬件流控跨平台实现
