---
title: "专家层"
description: "深入 Qt 源码，解析核心机制的设计原理和实现细节。共规划 102 篇，涵盖 MOC 编译器、信号槽底层、COW 机制、事件循环、对象树所有权、渲染管线等。"
---

# 专家层

深入 Qt 源码，解析核心机制的设计原理和实现细节。每篇教程以源码拆解为核心，结论落「文件:行号 + 逐字原文 + 一句话解读」，读者能拿行号自己开源码核对。

## 规划内容（102 篇）

- **00 · 环境搭建（2 篇）** — 从源码编译 Qt6、CMake 专家级配置（生成器表达式 · INTERFACE/PUBLIC/PRIVATE 传播 · 交叉工具链）
- **01 · QtBase（21 篇）** — QObject 元对象系统源码、信号槽 activate 调用链、QString SSO/COW、容器隐式共享、事件循环源码、QIODevice 抽象层、QThread 平台封装、MOC 编译器原理、信号槽深度拆解、COW 全解、对象树所有权（`setParent_helper` / `deleteChildren` / `deleteLater` / 线程亲和）
- **02 · QtGui（6 篇）** — QPainter 渲染后端、QTransform 矩阵运算、QImage 像素格式、字体引擎、OpenGL 上下文管理、拖放平台协议
- **03 · QtWidgets（43 篇）** — 主题能力 11 篇（布局算法、事件分发 `QApplication::notify`、Model/View 解耦、QSS 解析器、控件渲染脏矩形 / BackingStore、对话框模态事件循环、主窗口布局引擎、图形视图 BSP 树、动画时间轴、MDI 子窗口、QStyle 绘制委托）+ 控件速查 32 篇（从 QWidget / QAbstractButton 等枢纽控件到 QDialog / QFileDialog 等对话框，逐个拆源码；8 个无自研算法的 trivial 控件标「无专家层」）
- **04 · QtNetwork（6 篇）** — QAbstractSocket 十一态状态机、QUdpSocket 平台封装、QNetworkAccessManager 后端、QWebSocket 帧解析、SSL OpenSSL 抽象层、QSerialPort POSIX/Win32 封装
- **05 · 其他模块（17 篇）** — 数据库驱动插件、Charts 渲染流水线、Multimedia 平台后端、Camera、SVG 解析器、PrintSupport、Modbus PDU 协议栈、MQTT 帧编解码、Bluetooth、StateMachine、3D ECS、Quick3D 渲染、Pdf、HttpServer、WebSockets Server、Qt5Compat 等（NFC / SCXML / WebEngine 等 8 个小众模块暂缓，稳定后再评估）
- **06 · QML（7 篇）** — 绑定引擎源码、Controls 样式实现、C++ 类型系统桥接、动画引擎时间轴、组件编译器、模型视图虚拟化、WorkerScript 线程模型（V4 引擎 JIT/GC、QQmlEngine 元对象桥接 2 篇远期）

> 连载中 · 已完成 **19 篇**（QtBase 卷主体，全部审结；卷内仅剩 18 信号槽深拆、20 事件循环源码两篇）——
> [01 QObject 元对象系统](01-qtbase/01-qobject-meta-system-expert.md)、[02 信号槽底层 activate 调用链](01-qtbase/02-signal-slot-internals-expert.md)、[03 QString 内存模型](01-qtbase/03-qstring-memory-expert.md)、[04 COW 容器实战](01-qtbase/04-cow-container-practice-expert.md)、[05 QVariant 类型擦除](01-qtbase/05-qvariant-expert.md)、[06 智能指针家族](01-qtbase/06-memory-model-expert.md)、[07 事件循环源码](01-qtbase/07-event-loop-internals-expert.md)、[08 QIODevice 与 QFile](01-qtbase/08-file-io-iodevice-expert.md)、[09 QThread 源码](01-qtbase/09-qthread-internals-expert.md)、[10 QProcess 源码](01-qtbase/10-qprocess-expert.md)、[11 QTimer 源码](01-qtbase/11-qtimer-expert.md)、[12 QPluginLoader 插件系统](01-qtbase/12-plugin-loader-expert.md)、[13 QTranslator 与 QM 格式](01-qtbase/13-i18n-expert.md)、[14 qDebug 与 QLoggingCategory](01-qtbase/14-logging-expert.md)、[15 QRegularExpression 封装 PCRE2](01-qtbase/15-regex-pcre-expert.md)、[16 JSON 解析与 CBOR 内核](01-qtbase/16-json-parser-expert.md)、[17 MOC 编译器原理](01-qtbase/17-moc-compiler-expert.md)、[19 COW 隐式共享](01-qtbase/19-cow-implicit-sharing-expert.md)、[21 对象树所有权](01-qtbase/21-object-tree-ownership-expert.md)。
