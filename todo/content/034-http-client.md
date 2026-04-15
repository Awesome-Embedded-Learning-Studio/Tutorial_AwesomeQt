---
id: "034"
title: "QtNetwork 入门：HTTP 客户端 (QNetworkAccessManager)"
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

掌握 Qt 中 HTTP 客户端的实现方法，包括 QNetworkAccessManager、QNetworkRequest、
QNetworkReply 的使用。学会发起 GET/POST/PUT/DELETE 请求，处理响应数据，
管理 Cookie、认证、重定向等常见 HTTP 场景。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QNetworkAccessManager：网络访问管理器
  - 单例模式 (通常一个应用一个实例)
  - get / post / put / deleteResource / sendCustomRequest
- QNetworkRequest：请求构造
  - setHeader / setRawHeader
  - 常用请求头：Content-Type, User-Agent, Authorization
- QNetworkReply：响应处理
  - readyRead / finished / downloadProgress / uploadProgress 信号
  - readAll / read 读取响应体
  - attribute(QNetworkRequest::HttpStatusCodeAttribute)
  - error() / errorString() 错误处理
  - readBufferSize 与流式处理大响应
- POST 请求：
  - 表单数据 (application/x-www-form-urlencoded)
  - JSON 数据 (application/json)
  - multipart/form-data 文件上传 (QHttpMultiPart)
- QNetworkCookieJar：Cookie 管理
- 重定向处理
- 超时处理
- QJsonDocument 解析 JSON 响应

踩坑重点：
1. QNetworkReply 需要手动 deleteLater() 否则内存泄漏
2. 在 finished 信号中未检查 error() 就直接解析数据导致崩溃
3. 同步等待 (QEventLoop) 导致界面冻结

练习项目：实现一个 REST API 客户端，支持 GET/POST/PUT/DELETE 请求，
包含请求参数编辑器、响应 JSON 格式化显示、请求历史记录功能。

## 涉及文件

- document/tutorials/beginner/04-qtnetwork/03-http-client-beginner.md
- examples/beginner/04-qtnetwork/03-http-client-beginner/

## 参考资料

- [QNetworkAccessManager Class Reference](https://doc.qt.io/qt-6/qnetworkaccessmanager.html)
- [QNetworkRequest Class Reference](https://doc.qt.io/qt-6/qnetworkrequest.html)
- [QNetworkReply Class Reference](https://doc.qt.io/qt-6/qnetworkreply.html)
- [HTTP Example](https://doc.qt.io/qt-6/qtnetwork-http-example.html)
