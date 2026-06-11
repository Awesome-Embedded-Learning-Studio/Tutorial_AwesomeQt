---
title: "专家层"
description: "深入 Qt 源码，解析核心机制的设计原理和实现细节。共规划 142 篇，涵盖 MOC 编译器、信号槽底层、COW 机制、事件循环、渲染管线等。"
---

# 专家层

深入 Qt 源码，解析核心机制的设计原理和实现细节。每篇教程以源码拆解为核心，帮助你读懂 Qt 内部实现并举一反三。

## 规划内容（142 篇）

- **00 · 环境搭建（2 篇）** — 从源码编译 Qt6、CMake 专家级配置
- **01 · QtBase（20 篇）** — QObject 元对象系统源码、信号槽 activate 调用链、QString SSO/COW、容器隐式共享、事件循环源码、QIODevice 抽象层、QThread 平台封装、MOC 编译器原理、信号槽深度拆解、COW 全解、事件循环深度源码
- **02 · QtGui（6 篇）** — QPainter 渲染后端、QTransform 矩阵运算、QImage 像素格式、字体引擎、OpenGL 上下文管理、拖放平台协议
- **03 · QtWidgets（74 篇）** — 布局算法源码、事件分发全流程、Model/View 解耦机制、QSS 解析器、控件渲染脏矩形、对话框模态事件循环、主窗口布局引擎、BSP 树碰撞检测、动画时间轴、MDI 子窗口管理 + 64 个控件源码拆解
- **04 · QtNetwork（6 篇）** — QAbstractSocket 状态机、QUdpSocket 平台封装、QNetworkAccessManager 后端、QWebSocket 帧解析、SSL OpenSSL 抽象层、QSerialPort POSIX/Win32 封装
- **05 · 其他模块（25 篇）** — 数据库驱动插件、Charts 渲染流水线、Multimedia 平台后端、SVG 解析器、Modbus PDU 协议栈、MQTT 帧编解码、BLE 栈封装、StateMachine 执行引擎、3D ECS 架构等
- **06 · QML（9 篇）** — 绑定引擎源码、Controls 样式实现、类型系统注册桥接、动画引擎时间轴、组件编译器、模型视图虚拟化、WorkerScript 线程模型、V4 JavaScript 引擎 JIT/GC、QQmlEngine 元对象桥接

> 敬请期待...
