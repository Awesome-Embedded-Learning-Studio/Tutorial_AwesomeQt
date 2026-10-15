---
title: "01 · QtBase 核心模块（专家）"
description: "深入 QtBase 源码：QObject 元对象系统拆解、信号槽 activate 调用链、QString 无 SSO 的源码证明与 COW 内存模型、容器隐式共享、QVariant 类型擦除、事件循环平台抽象、QIODevice 缓冲机制、QThread 平台封装，以及 MOC 编译器、信号槽深度拆解、COW 全解、事件循环深度源码等 4 个专家专属章节，另有对象树所有权源码拆解。共 21 篇。"
---

# 01 · QtBase 核心模块（专家）

> 咱们已经写成 19 / 21 篇，其余的也在陆续更新。

## 章节规划

- **01 QObject 元对象系统源码拆解** — `QObjectPrivate` d 指针、`QMetaObject` 结构体、`qt_metacall` 分发
- **02 信号槽底层** — `QMetaObject::activate` 调用链、`ConnectionData` 连接列表、跨线程 `QMetaCallEvent`
- **03 QString 内存模型** — `QArrayDataPointer` 内部结构、无 SSO 的源码证明、`detach()` 写时复制
- **04 COW 在 Qt 容器中的实战** — QString/QByteArray/QList 的 detach 触发点、const 与非 const 访问的性能差异、fromRawData 与 QVarLengthArray 的取舍
- **05 QVariant 类型擦除** — `QVariant::Private` 联合体、`QMetaType` 函数指针表、自定义类型注册 vtable
- **06 Qt 智能指针家族源码拆解** — `ExternalRefCountData` 强弱双计数、weak 升 strong 的 CAS 循环、`QPointer` 的 `-1` 哨兵、`QScopedPointer` 栈上 RAII 与 `QSharedData` 内部 ref 对比（`QObject::~QObject` 子对象析构归 21 篇）
- **07 事件循环源码** — `QAbstractEventDispatcher` 平台抽象、`processEvents()` 一次迭代、定时器集成
- **08 QIODevice 抽象层** — 读写缓冲区管理、`QFile` 平台 IO 后端、`QBuffer` 内存 IO
- **09 QThread 源码** — `QThreadPrivate` 平台实现、`QThreadData` 身份包、`quit`/`exit` 与 `terminate` 的取舍、`QThread::create`（`QThreadStorage<T>`、`moveToThread()` 连接类型升级延后，暂无落点篇章）
- **10 QProcess 源码** — `vforkfd`/`CreateProcessW` 双平台启动链、`forkfd` 进程死亡事件化、ChannelMode 五态与管道重定向、`terminate`/`kill` 跨平台不对称
- **11 QTimer 分发机制** — `timerEvent()` 分发路径、精度保证机制、定时器合并优化
- **12 QPluginLoader 源码** — `QFactoryLoader` 静态/动态插件查找、元数据 CBOR 编码、免加载读取
- **13 QTranslator 源码** — `.qm` 文件二进制布局、哈希查找、复数规则运行时求值
- **14 日志系统源码** — `qt_message_output` 分发链、`QLoggingCategory` 规则解析
- **15 QRegularExpression 源码** — PCRE2 集成、JIT 编译触发、UTF-16 偏移转换
- **16 JSON 解析与 CBOR 内核源码拆解** — `QJsonDocument` 薄壳、`QCborContainerPrivate` 内存布局、递归下降 `Parser`、`Writer` 与 JSON↔CBOR/QVariant 互转桥
- **17【专属】MOC 编译器原理** — 词法/语法分析、`qt_static_metacall` 生成、`QMetaObject` 内存布局
- **18【专属】信号槽深度拆解** — `Connection`/`ConnectionList` 数据结构、信号重入处理、性能基准
- **19【专属】COW 隐式共享全解** — 原子引用计数、`QArrayData`/`QSharedData` 两条路线对比、detach 机制与线程安全边界（自定义共享类模板骨架待补）
- **20【专属】事件循环深度源码** — `QSocketNotifier` 集成、`processEvents` flags 语义、嵌套事件循环重入安全
- **21 对象树所有权源码** — `setParent_helper` 三段式增删子对象、`deleteChildren` 手写循环防 double-free、`deleteLater` 投递 `DeferredDelete`、对象树同线程约束
