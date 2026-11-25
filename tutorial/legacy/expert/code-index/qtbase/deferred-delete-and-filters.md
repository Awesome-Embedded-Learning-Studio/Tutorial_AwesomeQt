---
title: DeferredDelete 全链、timerEvent 与事件过滤器
description: DeferredDelete=52；QObject::event case DeferredDelete 直接 delete this；时机判定读 loopLevel/scopeLevel，不满足则 re-post 不丢失；execCleanup 兜底 flush DeferredDelete；Timer→timerEvent；notify_helper 双层过滤器（应用级仅主线程+对象级）任一返回 true 短路；installEventFilter 同线程校验+prepend 头插形成「后装先拦」。
---

# DeferredDelete 全链、timerEvent 与事件过滤器

> 本索引收录 Qt 6.9.1 源码中 DeferredDelete 链路、定时器入口与事件过滤器机制相关的已验证证据。对应专家篇《07 事件循环源码拆解》§3.6-§3.8。

## DeferredDelete 全链路

源码文件：`qtbase/src/corelib/kernel/qcoreevent.h` / `qobject.cpp` / `qcoreapplication.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| DeferredDelete 事件类型常量值 = 52 | qcoreevent.h:104 | `DeferredDelete = 52,` | deleteLater 投递的就是这个类型的事件。 |
| QObject::event 收 DeferredDelete 直接 delete this 就地销毁 | qobject.cpp:1415-1417 | `case QEvent::DeferredDelete: delete this; break;` | 完成deleteLater→postEvent→sendPostedEvents→notify→event→case→delete this 全链。 |
| DeferredDelete 时机判定读 QDeferredDeleteEvent 的 loopLevel/scopeLevel 与 data 比较，allowDeferredDelete 不满足则 pe_copy 重新 addEvent 不丢失 | qcoreapplication.cpp:1824-1857 | `const bool allowDeferredDelete = (eventLoopLevel + eventScopeLevel > data->loopLevel + data->scopeLevel \|\| ...); if (!allowDeferredDelete) { data->postEventList.addEvent(pe_copy); continue; }` | 嵌套循环层级不满足时推迟到下一圈，事件不丢。 |
| execCleanup 兜底：主循环返回后显式 sendPostedEvents(nullptr, DeferredDelete) 强制 flush | qcoreapplication.cpp:1464-1468 | `void execCleanup() { ... QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); }` | 保证 deleteLater 在 app 退出前兑现。 |

## timerEvent 入口

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QObject::event case Timer 转调 timerEvent，默认 timerEvent 是空实现（子类重写才收到） | :1402-1407 | `case QEvent::Timer: timerEvent((QTimerEvent *)e); break;` | dispatcher 注册系统定时器的平台侧不在 scope。 |

## 事件过滤器双层拦截

源码文件：`qtbase/src/corelib/kernel/qcoreapplication.cpp` / `qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| notify_helper 双层过滤器：先 sendThroughApplicationEventFilters（仅主线程），再 sendThroughObjectEventFilters，任一 true 短路，都不拦才 receiver->event | qcoreapplication.cpp:1255-1279 | `if (QThread::isMainThread() && ... sendThroughApplicationEventFilters(...)) { filtered = true; return filtered; } if (sendThroughObjectEventFilters(...)) { ...; return filtered; } consumed = receiver->event(event);` | 应用级（qApp 装的，主线程）+ 对象级（receiver 装的）两道，前者优先。 |
| sendThroughObjectEventFilters 遍历 receiver 的 eventFilters 逐个调 obj->eventFilter(receiver,event) | qcoreapplication.cpp:1232-1248 | `for (...) { QObject *obj = ...eventFilters.at(i); if (obj->eventFilter(receiver, event)) return true; }` | eventFilter 虚函数的实际触发处。 |
| installEventFilter 同线程校验（不同线程 qWarning 拒绝）+ 去重后 prepend 头插，配合从头遍历形成「后装先拦」 | qobject.cpp:2350-2366 | `if (d->threadData.loadRelaxed() != obj->d_func()->threadData.loadRelaxed()) { qWarning("...different thread."); return; } ... d->extraData->eventFilters.prepend(obj);` | 头插+从头遍历=后装的先被调用，多过滤器顺序与直觉相反。 |
