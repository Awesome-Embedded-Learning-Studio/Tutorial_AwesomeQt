---
title: 连接表——信号到槽的映射结构
description: doActivate 从 ConnectionData 取 SignalVector 按 signal_index 取 ConnectionList；Connection 节点同时挂信号链(nextConnectionList)与 receiver 的 senders 链；connectImpl new Connection 经 addConnection 双向插链并分配递增 id。
---

# 连接表——信号到槽的映射结构

> 本索引收录 Qt 6.9.1 源码中连接表数据结构与建立过程相关的已验证证据。对应专家篇《02 信号槽底层——activate 调用链》§3.3-§3.4。结构定义在更内层的私有头 `qobject_p_p.h`（双 `_p`）。

## 连接表取快照

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| doActivate 从 sp->connections loadAcquire 取 ConnectionData 快照，signalVector 以 signal_index 当下标取 ConnectionList | :4061-4068 | `ConnectionDataPointer connections(sp->connections.loadAcquire()); ... if (signal_index < signalVector->count()) list = &signalVector->at(signal_index); else list = &signalVector->at(-1);` | loadAcquire 保证后续读看到此前连接写入；越界信号落到 -1 桶。 |

## ConnectionList / Connection / ConnectionData 结构

源码文件：`qtbase/src/corelib/kernel/qobject_p_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ConnectionList 是带 first/last 的单链表容器 | :29-33 | `struct ConnectionList { QAtomicPointer<Connection> first; QAtomicPointer<Connection> last; };` | 每个信号对应一个，串着所有连到该信号的槽连接。 |
| Connection 节点同时挂两链（信号链 nextConnectionList/prevConnectionList + receiver 链 prev/next），connectionType 2bit 存四种类型 | :69-95 | `Connection **prev; QAtomicPointer<Connection> nextConnectionList; ... ushort connectionType : 2;` | 双链设计让「断某信号所有连接」和「断某对象接收的所有连接」都高效；type 0=Auto/1=Direct/2=Queued/3=Blocking。 |
| ConnectionData 持 currentConnectionId/ref/signalVector/senders，currentConnectionId 析构置 0 当停止发射哨兵 | :136-145 | `QAtomicInteger<uint> currentConnectionId; QAtomicInt ref; QAtomicPointer<SignalVector> signalVector; Connection *senders = nullptr;` | currentConnectionId 递增 id 用于重入保护，析构置 0 让发射循环停下。 |

## connectImpl + addConnection 建立连接

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| connectImpl new Connection 填字段后调 addConnection 插表，isSlotObject=true 标记函数对象式连接 | :5318-5334 | `std::unique_ptr<Connection> c{new Connection}; c->sender = s; ... c->isSlotObject = true; c->slotObj = ...; QObjectPrivate::get(s)->addConnection(...)` | lambda/std::bind 包装成 slotObj；填完字段交 addConnection 插链。 |
| addConnection 尾插信号链表 + c->id=++currentConnectionId 分配递增 id + 双向挂进 receiver 的 senders 链 | :282-300 | `connectionList.last...->nextConnectionList.storeRelaxed(c); ... c->id = ++cd->currentConnectionId; ... c->prev = &(rd->connections...->senders); *c->prev = c;` | 双向插链（信号链尾插 + senders 链头插）；递增 id 是重入保护第一道防线。 |
