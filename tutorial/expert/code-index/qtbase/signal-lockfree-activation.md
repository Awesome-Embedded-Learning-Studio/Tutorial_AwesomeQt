---
title: 无锁发射——条带锁、快照与孤儿链
description: 131 把地址哈希条带锁只归 connect 与 disconnect、doActivate 发射循环全程不持 sender 连接表锁（receiver 侧短暂加锁三处）、ConnectionDataPointer 快照加引用计数护表、removeConnection 摘链保留 nextConnectionList、orphaned 链延迟删除、cleanOrphanedConnections 四触发点、maybeSignalConnected 与 isSignalConnected 直查 SignalVector（Qt5 位图已删）、resizeSignalVector 八格对齐增长。
---

# 无锁发射——条带锁、快照与孤儿链

> 本索引收录 Qt 6.9.1 源码中发射循环线程安全机制相关的已验证证据。对应专家篇《18 信号槽深度拆解》正文发射循环的无锁真相一节。02 篇的连接表结构与清理证据见 [signal-connection-table](./signal-connection-table.md) 与 [signal-dispatch-cleanup](./signal-dispatch-cleanup.md)。

## 条带锁池

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 131 桶锁池 + signalSlotLock 按对象地址取锁 | :130-137 | `Q_CONSTINIT static QBasicMutex _q_ObjectMutexPool[131]; ... static inline QBasicMutex *signalSlotLock(const QObject *o)` | 注释明说 mutex to be locked when accessing the connection lists or the senders list；SignalMutex/ConnectionListLock 全库不存在。 |
| doActivate 发射循环全程不持 sender 连接表锁 | :4024-4191 | 主循环 4076-4182 无 sender 条带锁 | receiver 侧短暂加锁共三处（3979、4095、4120）。 |
| connect/disconnect 用 QOrderedMutexLocker 双锁 | :3613-3614 | `QOrderedMutexLocker locker(signalSlotLock(sender), signalSlotLock(receiver));` | 双方都持锁时按固定顺序防死锁。 |

## 快照与摘链

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qobject_p.h` / `qobject_p_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ConnectionDataPointer 是 QExplicitlySharedDataPointer 别名 | qobject_p.h:206-207 | `using ConnectionDataPointer = QExplicitlySharedDataPointer<ConnectionData>;` | 构造快照原子加 ConnectionData::ref（qobject_p_p.h:141），析构减；发射期间表被换掉也保证这次遍历活到循环结束。 |
| doActivate 开头取快照 | qobject.cpp:4061 | `ConnectionDataPointer connections(sp->connections.loadAcquire());` | 不拿裸指针的原因见上行。 |
| removeConnection 摘链但保留 nextConnectionList | qobject.cpp:337-343 | `// keep c->nextConnectionList intact, as it might still get accessed by activate` | 注释原话；发射循环 4178 行正顺着它走。 |
| 摘下的节点 CAS 进 orphaned 链延迟删除 | qobject.cpp:346-354 | `c->orphaned.storeRelaxed(...)` 一带 | 等没有发射在跑才真正 delete。 |
| Connection 自带 QAtomicInt ref_{2} 逐节点计数 | qobject_p_p.h:85-87 | `QAtomicInt ref_{ 2 }; // ref_ is 2 for the use in the internal lists, and for the use in QMetaObject::Connection` | 内部表与句柄各一份；ref/deref 在 103/111。发射循环期间不做逐节点 ref/deref，安全靠快照加保留指针加孤儿链。 |
| 循环条件：nextConnectionList 非空且 id 不超快照 | qobject.cpp:4178 | `c = c->nextConnectionList.loadRelaxed() != nullptr && c->id <= highestConnectionId` | 发射中新增连接的 id 必大于快照，本轮跳过。 |

## 孤儿清理与信号连接查询

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qobject_p_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| cleanOrphanedConnectionsImpl：ref 大于 1 说明还有 activate 在跑，直接返回 | qobject.cpp:369-395 | `if (ref.loadAcquire() > 1) return;`（377-379） | 等于 1 才摘 orphaned 链（383 行 exchange），解锁后 deleteOrphaned 逐个删（跑用户代码不持锁）。 |
| 孤儿链挂在持连接表的 sender 侧 ConnectionData | qobject_p_p.h:145 | `SignalVector *orphaned;` | 清理发生在持表的一方，不是 receiver 侧。 |
| 四个触发点：~QObject、disconnect 两路、doActivate 结束 | qobject.cpp:1117 / :3780 / :5566 / :4188 | 独立 grep 命中 | 对象死亡、断连、发射收尾三场景都会触发清点。 |
| deleteOrphaned 里 freeSlotObject 提前销毁槽对象 | qobject.cpp:409 | `c->freeSlotObject();` 一带 | ~QObject 路径（1100-1103 + 1126-1127）是手动等效操作。 |
| connectedSignals 位图在 6.9 不存在，maybeSignalConnected 直查 SignalVector | qobject.cpp:448-465 | `if (signalVector->at(-1).first.loadAcquire()) return true; if (signalIndex < uint(cd->signalVectorCount())) { ... return c != nullptr; }` | 先看 at(-1) 全体信号桶再查本信号桶；doActivate 4046 行用它做发射前短路。Qt5 的 quint64 位图与溢出故事无从谈起。 |
| isSignalConnected 完整版含 QML declarative 通道 | qobject.cpp:422-446 | `checkDeclarative` 参数分支 | 与 maybeSignalConnected 的区别就在这层。 |
| resizeSignalVector：容量够不动，向上取整 8 倍数，一次 malloc 头加 size+1 桶 | qobject_p_p.h:182-216 | `size = (size + 7) & ~7; void *ptr = malloc(sizeof(SignalVector) + (size + 1) * sizeof(ConnectionList));` | +1 就是 at(-1) 全体桶；memcpy 旧表、原子换指针、旧表头 CAS 挂 orphaned（注释明说无 ABA 问题）。 |
| SignalVector 本体与 at(i) 取桶 | qobject_p_p.h:122-132 | at(i) 用 reinterpret_cast 取 this+1 后第 i+1 个桶 | 头尾两块内存在同一次 malloc 里。 |
