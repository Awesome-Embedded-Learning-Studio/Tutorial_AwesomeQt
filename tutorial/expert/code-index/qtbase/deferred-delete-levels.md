---
title: loopLevel 与 scopeLevel——deleteLater 的时机数据流
description: loopLevel 由 exec 的 LoopReference 增减、scopeLevel 由 QScopedScopeLevelCounter 增减（notifyInternal2 与 native event filter 消费）、deleteLater 在投递时刻把两个计数器快照铸进 QDeferredDeleteEvent（跨线程时快照 0/0、scopeLevel 为 0 且 loopLevel 非 0 时猜 1 兜底）、sendPostedEvents 与当前值比较决定放行（与 07 篇 DeferredDelete 判定衔接）。
---

# loopLevel 与 scopeLevel——deleteLater 的时机数据流

> 本索引收录 Qt 6.9.1 源码中循环层级计数器相关的已验证证据。对应专家篇《20 事件循环源码》正文 loopLevel 与 scopeLevel 一节。07 篇的 DeferredDelete 全链与 allowDeferredDelete 判定式见 [deferred-delete-and-filters](./deferred-delete-and-filters.md)。

## 两个计数器

源码文件：`qtbase/src/corelib/thread/qthread_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| loopLevel/scopeLevel 定义在 QThreadData | :365-368 | `int loopLevel = 0; int scopeLevel = 0;` | loopLevel 由 LoopReference 构造/析构增减（qeventloop.cpp:156/175），QCoreApplication::exec 不直接碰。 |
| QScopedScopeLevelCounter：构造 ++ 析构 -- | :378-387 | `{ ++threadData->scopeLevel; }` / `{ --threadData->scopeLevel; }` | 消费点 notifyInternal2（qcoreapplication.cpp:1098）与 native event filter（qabstracteventdispatcher.cpp:557-560，注释明说抬高 scopeLevel 让 filter 里的 deleteLater 留给外层主循环处理）。 |

## deleteLater 的快照

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qcoreevent_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| deleteLater 在投递时刻把当时的 loopLevel/scopeLevel 快照进事件 | qobject.cpp:2471-2499 | `loopLevel = objectThreadData->loopLevel; scopeLevel = objectThreadData->scopeLevel; if (scopeLevel == 0 && loopLevel != 0) scopeLevel = 1; ... new QDeferredDeleteEvent(loopLevel, scopeLevel);` | 快照不是无条件的：跨线程调用时对象线程计数器为 0/0；猜 1 兜底防非规范路径。 |
| QDeferredDeleteEvent 载体 | qcoreevent_p.h:24-35 | 事件类定义 | sendPostedEvents 读回并与线程当前值比较得 allowDeferredDelete（07 篇已覆盖该比较段）。 |
