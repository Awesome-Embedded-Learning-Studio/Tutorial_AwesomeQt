---
title: "01 · QtBase 核心模块（专家）"
description: "深入 QtBase 源码：QObject 元对象系统拆解、信号槽 activate 调用链、QString SSO/COW 内存模型、容器隐式共享、QVariant 类型擦除、事件循环平台抽象、QIODevice 缓冲机制、QThread 平台封装，以及 MOC 编译器、信号槽深度拆解、COW 全解、事件循环深度源码等 4 个专家专属章节。共 20 篇。"
---

# 01 · QtBase 核心模块（专家）

> 规划中（20 篇），敬请期待...

## 章节规划

- **01 QObject 元对象系统源码拆解** — `QObjectPrivate` d 指针、`QMetaObject` 结构体、`qt_metacall` 分发
- **02 信号槽底层** — `QMetaObject::activate` 调用链、`ConnectionData` 连接列表、跨线程 `QMetaCallEvent`
- **03 QString 内存模型** — `QArrayDataPointer` 内部结构、SSO 触发条件、`detach()` 写时复制
- **04 容器隐式共享（COW）源码** — `QSharedData` 引用计数、`QListData` 增长策略、`QHash` 开放地址法
- **05 QVariant 类型擦除** — `QVariant::Private` 联合体、`QMetaType` 函数指针表、自定义类型注册 vtable
- **06 引用计数与内存模型** — `QAtomicInt` 无锁引用计数、`ExternalRefCountData`、`QObject::~QObject` 子对象析构
- **07 事件循环源码** — `QAbstractEventDispatcher` 平台抽象、`processEvents()` 一次迭代、定时器集成
- **08 QIODevice 抽象层** — 读写缓冲区管理、`QFile` 平台 IO 后端、`QBuffer` 内存 IO
- **09 QThread 源码** — `QThreadPrivate` 平台实现、`QThreadStorage<T>`、`moveToThread()` 连接类型升级
- **10 QProcess 源码** — `fork()`+`execve()` vs `CreateProcess()`、进程组管理
- **11 QTimer 分发机制** — `timerEvent()` 分发路径、精度保证机制、定时器合并优化
- **12 QPluginLoader 源码** — `QFactoryLoader` 静态/动态插件查找、元数据 JSON 嵌入解析
- **13 QTranslator 源码** — `.qm` 文件二进制布局、哈希查找、复数规则运行时求值
- **14 日志系统源码** — `qt_message_output` 分发链、`QLoggingCategory` 规则解析
- **15 QRegularExpression 源码** — PCRE2 集成、JIT 编译触发、UTF-16 偏移转换
- **16 JSON 解析状态机** — `QJsonParser` 递归下降、`QJsonPrivate::Data` 二进制表示
- **17【专属】MOC 编译器原理** — 词法/语法分析、`qt_static_metacall` 生成、`QMetaObject` 内存布局
- **18【专属】信号槽深度拆解** — `Connection`/`ConnectionList` 数据结构、信号重入处理、性能基准
- **19【专属】COW 隐式共享全解** — `QExplicitlySharedDataPointer` vs `QSharedDataPointer`、自定义共享类模板
- **20【专属】事件循环深度源码** — `QSocketNotifier` 集成、`processEvents` flags 语义、嵌套事件循环重入安全
