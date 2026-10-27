---
title: 连接表——信号到槽的映射结构
description: doActivate 从 ConnectionData 取 SignalVector 按 signal_index 取 ConnectionList；Connection 节点同时挂信号链(nextConnectionList)与 receiver 的 senders 链；connectImpl new Connection 经 addConnection 双向插链并分配递增 id。
---

# 连接表——信号到槽的映射结构

> 本索引收录 Qt 6.9.1 源码中连接表数据结构与建立过程相关的已验证证据。对应专家篇《02 信号槽底层——activate 调用链》正文连接表、连接的建立两节。结构定义在更内层的私有头 `qobject_p_p.h`（双 `_p`）。断开路径、句柄有效性与连接自带参数元类型表的引文出自正文风险与故障预防一节，按机制归入本组。

## 连接表取快照

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| doActivate 从 sp->connections loadAcquire 取 ConnectionData 快照，signalVector 以 signal_index 当下标取 ConnectionList | :4061-4068 | `ConnectionDataPointer connections(sp->connections.loadAcquire()); ... if (signal_index < signalVector->count()) list = &signalVector->at(signal_index); else list = &signalVector->at(-1);` | loadAcquire 保证后续读看到此前连接写入；越界信号落到 -1 桶。 |

## ConnectionList / Connection / ConnectionData 结构

源码文件：`qtbase/src/corelib/kernel/qobject_p_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ConnectionList 是带 first/last 的单链表容器 | :28-33 | `// ConnectionList is a singly-linked list / struct ConnectionList { QAtomicPointer<Connection> first; QAtomicPointer<Connection> last; };` | 每个信号对应一个，串着所有连到该信号的槽连接。 |
| Connection 节点同时挂两链（信号链 nextConnectionList/prevConnectionList + receiver 链 prev/next），connectionType 2bit 存四种类型 | :69-95 | `Connection **prev; QAtomicPointer<Connection> nextConnectionList; ... ushort connectionType : 2;` | 双链设计让「断某信号所有连接」和「断某对象接收的所有连接」都高效；type 0=Auto/1=Direct/2=Queued/3=Blocking。 |
| ConnectionData 持 currentConnectionId/ref/signalVector/senders，currentConnectionId 析构置 0 当停止发射哨兵 | :136-145 | `QAtomicInteger<uint> currentConnectionId; QAtomicInt ref; QAtomicPointer<SignalVector> signalVector; Connection *senders = nullptr;` | currentConnectionId 递增 id 用于重入保护，析构置 0 让发射循环停下。 |

## connectImpl + addConnection 建立连接

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| connectImpl new Connection 填字段后调 addConnection 插表，isSlotObject=true 标记函数对象式连接 | :5318-5334 | `std::unique_ptr<Connection> c{new Connection}; c->sender = s; ... c->isSlotObject = true; c->slotObj = ...; QObjectPrivate::get(s)->addConnection(...)` | lambda/std::bind 包装成 slotObj；填完字段交 addConnection 插链。 |
| addConnection 尾插信号链表 + c->id=++currentConnectionId 分配递增 id + 双向挂进 receiver 的 senders 链 | :282-300 | `connectionList.last...->nextConnectionList.storeRelaxed(c); ... c->id = ++cd->currentConnectionId; ... c->prev = &(rd->connections...->senders); *c->prev = c;` | 双向插链（信号链尾插 + senders 链头插）；递增 id 是重入保护第一道防线。 |

## 断开与句柄有效性（正文风险与故障预防一节）

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qobjectdefs.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| removeConnection 第一个动作清空 receiver，所有断开路径（句柄版 disconnect、按 signal/receiver/method 各重载、任一端析构）都经过它 | qobject.cpp:303-307 | `void QObjectPrivate::ConnectionData::removeConnection(QObjectPrivate::Connection *c) { Q_ASSERT(c->receiver.loadRelaxed()); ... c->receiver.storeRelaxed(nullptr);` | 摘链第一步就清空 receiver，句柄布尔值从此可靠为 false。 |
| isConnected_helper 返回 c->receiver，句柄布尔值由它决定 | qobject.cpp:5653-5659 | `bool QMetaObject::Connection::isConnected_helper() const { ... return c->receiver.loadRelaxed(); }` | 断开后 receiver 已被清空，句柄布尔值确定返回 false。 |
| operator RestrictedBool 在 d_ptr 非空且 isConnected_helper 为真时才返回非空指针 | qobjectdefs.h:672 | `operator RestrictedBool() const { return d_ptr && isConnected_helper() ? &Connection::d_ptr : nullptr; }` | RestrictedBool 技巧实现安全 bool 检查，返回值不可当整数误用。 |

## 连接自带的参数元类型表（正文风险与故障预防一节）

源码文件：`qtbase/src/corelib/kernel/qobject.h` / `qobject_impl.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| PMF 版 connect 在 Queued/BlockingQueued 时调 ConnectionTypes<...>::types() 取参数元类型表 | qobject.h:257-259 | `const int *types = nullptr; if (type == Qt::QueuedConnection \|\| type == Qt::BlockingQueuedConnection) types = QtPrivate::ConnectionTypes<...>::types();` | 取不到类型时静默留空，连接照常建立。 |
| ConnectionTypes 对未注册类型静默返回 nullptr 的主特化 | qobject_impl.h:34-35 | `template <typename ArgList, bool Declared = ...> struct ConnectionTypes { static const int *types() { return nullptr; } };` | PMF 版 connect 不报警的根源，报警推迟到第一次 emit 的补查。 |
