---
id: "036"
title: "QtNetwork 入门：SSL/TLS 加密通信"
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

掌握 Qt 中 SSL/TLS 加密通信的实现方法，包括 QSslSocket、QSslCertificate、
QSslKey 的使用。理解证书验证、加密套件选择、双向认证等安全通信概念。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- SSL/TLS 基础概念：对称/非对称加密、证书链、CA
- QSslSocket：
  - connectToHostEncrypted / startClientEncryption
  - encrypted / sslErrors / peerVerifyError 信号
  - setPeerVerifyMode：AutoVerifyPeer, VerifyPeer, QueryPeer, VerifyNone
  - ignoreSslErrors (谨慎使用)
- QSslCertificate：证书加载与验证
  - fromPath / fromData
  - issuerInfo / subjectInfo
  -有效期验证
- QSslKey：私钥管理
- QSslConfiguration：加密配置
  - setProtocol：TLSv1_2, TLSv1_3
  - setCiphers
  - setCaCertificates
- 双向认证 (mTLS) 实现
- 自签名证书的生成与使用 (OpenSSL 命令)
- Qt SSL 后端要求

踩坑重点：
1. 忽略所有 SSL 错误 (ignoreSslErrors) 在生产环境中导致中间人攻击风险
2. 自签名证书未添加到 CA 列表导致连接失败
3. Qt 未链接 OpenSSL 后端导致 SSL 功能不可用

练习项目：实现一个基于 SSL 加密的文件传输工具，包含服务端和客户端，
使用自签名证书进行加密通信，支持证书指纹校验。

## 涉及文件

- document/tutorials/beginner/04-qtnetwork/05-ssl-tls-beginner.md
- examples/beginner/04-qtnetwork/05-ssl-tls-beginner/

## 参考资料

- [QSslSocket Class Reference](https://doc.qt.io/qt-6/qsslsocket.html)
- [QSslCertificate Class Reference](https://doc.qt.io/qt-6/qsslcertificate.html)
- [QSslConfiguration Class Reference](https://doc.qt.io/qt-6/qsslconfiguration.html)
- [Secure Sockets Layer (SSL) Classes](https://doc.qt.io/qt-6/ssl.html)
