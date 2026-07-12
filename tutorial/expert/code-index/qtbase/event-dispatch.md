---
title: 事件分发——dispatcher 抽象与 send/post 双路
description: processEvents 是转发 wrapper；QAbstractEventDispatcher 把 processEvents/registerTimer/wakeUp/interrupt 全声明纯虚（平台后端实现）；notify 是分发总入口；sendEvent 同步直发不删事件；notifyInternal2 强制同线程规则；postEvent 异步入队+wakeUp；sendPostedEvents 取出后走 sendEvent 派发并 unique_ptr 删事件；priority 降序+等优先级 FIFO。
---

# 事件分发——dispatcher 抽象与 send/post 双路

> 本索引收录 Qt 6.9.1 源码中事件分发链路相关的已验证证据。对应专家篇《07 事件循环源码拆解》§3.3-§3.5。

## processEvents 与 dispatcher 抽象层

源码文件：`qtbase/src/corelib/kernel/qeventloop.cpp` / `qabstracteventdispatcher.h` / `qabstracteventdispatcher.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QEventLoop::processEvents 自承是 wrapper，dispatcher 不存在返回 false，否则转发 flags 给 eventDispatcher->processEvents | qeventloop.cpp:94-104 | `if (!threadData->hasEventDispatcher()) return false; return threadData->eventDispatcher.loadRelaxed()->processEvents(flags);` | 真正收事件的是平台 dispatcher。 |
| QAbstractEventDispatcher 把平台相关项全声明纯虚（processEvents/registerTimer/unregisterTimer/wakeUp/interrupt） | qabstracteventdispatcher.h:44-74 | `virtual bool processEvents(...) = 0; virtual void registerTimer(...) = 0; ... virtual void wakeUp() = 0; virtual void interrupt() = 0;` | 无默认实现，UNIX/Win32/macOS 子类各自实现。 |
| dispatcher 文档定位：从窗口系统及事件源收事件，发给 QCoreApplication 处理 | qabstracteventdispatcher.cpp:121-124 | `An event dispatcher receives events from the window system and other sources. It then sends them to the QCoreApplication ... for processing and delivery.` | dispatcher 收事件，QCoreApplication 分发——职责划分。 |

## notify 总分发 + sendEvent 同步

源码文件：`qtbase/src/corelib/kernel/qcoreapplication.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| notify 是总分发入口：送 event 给 receiver->event()，用于 any object in any thread 的所有事件 | :1124-1128 | `Sends event to receiver: receiver->event(event).` | 所有事件分发最终都过这里。 |
| sendEvent 同步直发：用 notify()，事件不被删除，实现置 m_spont=false 再 notifyInternal2 | :1525-1547 | `The event is not deleted ... bool sendEvent(...) { event->m_spont = false; return notifyInternal2(receiver, event); }` | 同步直调，事件生命周期归调用者管（与 postEvent 对比）。 |
| notifyInternal2 强制同线程规则：注释逐字 'events can only be sent to objects in the current thread' | :1080-1106 | `// Qt enforces the rule that events can only be sent to objects in the current thread ... if (!selfRequired) return doNotify(...); return qApp->notify(receiver, event);` | sendEvent 跨线程用违规；跨线程必须 postEvent。 |

## postEvent 异步入队 + sendPostedEvents 派发

源码文件：`qtbase/src/corelib/kernel/qcoreapplication.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| postEvent 异步入队：unique_ptr 持 event 防异常泄漏，addEvent 后 release 转所有权，置 m_posted/postedEvents++/canWait=false，最后 dispatcher->wakeUp | :1622-1664 | `std::unique_ptr<QEvent> eventDeleter(event); ... data->postEventList.addEvent(...); eventDeleter.release(); event->m_posted = true; ... data->canWait = false; dispatcher->wakeUp();` | 事件所有权转交队列；wakeUp 踹醒阻塞的 dispatcher。 |
| sendPostedEvents 派发核心：取 pe 置 m_posted=false/postedEvents--，unique_ptr event_deleter(e) 接管所有权，最后 sendEvent(r,e) 同步派发 | :1860-1879 | `pe.event->m_posted = false; ... const std::unique_ptr<QEvent> event_deleter(e); QCoreApplication::sendEvent(r, e);` | 异步事件最终派发那一刻走同步 sendEvent；unique_ptr 派发完自动删事件。 |
| postEvent priority 排序规则：按 priority 降序（高先出），等优先级 FIFO，priority 可取 INT_MIN..INT_MAX | :1611-1616 | `Events are sorted in descending priority order ... The priority can be any integer value, i.e. between INT_MAX and INT_MIN ... equal priority processed in the order posted.` | 只引排序规则，addEvent 内部算法不在 scope。 |
