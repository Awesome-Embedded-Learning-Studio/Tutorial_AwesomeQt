---
title: 事件循环骨架——exec 与退出
description: QCoreApplication::exec 栈上构造 QEventLoop 调 eventLoop.exec(ApplicationExec)；QEventLoop::exec 心脏是 while(!exit.loadAcquire()) 反复 processEvents(WaitForMore+EventLoopExec)；exit 用 storeRelease 置标志 + dispatcher->interrupt() 踹醒阻塞；ApplicationExec=0x80 / EventLoopExec=0x20 区分主循环/嵌套循环。
---

# 事件循环骨架——exec 与退出

> 本索引收录 Qt 6.9.1 源码中事件循环骨架与退出机制相关的已验证证据。对应专家篇《07 事件循环源码拆解》§3.1-§3.2。

## exec 入口

源码文件：`qtbase/src/corelib/kernel/qcoreapplication.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QCoreApplication::exec 栈上构造 QEventLoop，调 eventLoop.exec(ApplicationExec)，返回后 execCleanup 兜底 | :1430-1455 | `QEventLoop eventLoop; ... int returnCode = eventLoop.exec(QEventLoop::ApplicationExec); ... self->d_func()->execCleanup();` | 每条线程一个栈上 QEventLoop；execCleanup 是 DeferredDelete 最后兑现机会。 |

## 循环心脏

源码文件：`qtbase/src/corelib/kernel/qeventloop.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QEventLoop::exec 心脏是 while(!d->exit.loadAcquire()) 反复调 processEvents(WaitForMoreEvents\|EventLoopExec) | :185-186 | `while (!d->exit.loadAcquire()) processEvents(flags \| WaitForMoreEvents \| EventLoopExec);` | 原子 exit 标志驱动轮转；loadAcquire 与 exit 的 storeRelease 配对。 |
| exit 用 storeRelease 置标志 + storeRelaxed 存 returnCode + dispatcher->interrupt() 踹醒阻塞轮 | :253-262 | `d->returnCode.storeRelaxed(returnCode); d->exit.storeRelease(true); threadData->eventDispatcher.loadRelaxed()->interrupt();` | interrupt 是退出关键一脚——光置标志不行，要踹醒阻塞的 processEvents。 |

## 循环类型标记

源码文件：`qtbase/src/corelib/kernel/qeventloop.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ApplicationExec=0x80 标记主应用循环，EventLoopExec=0x20 标记普通 QEventLoop exec | :31,33 | `EventLoopExec = 0x20, ... ApplicationExec = 0x80,` | dispatcher 据此区分主循环 / 嵌套循环。 |
