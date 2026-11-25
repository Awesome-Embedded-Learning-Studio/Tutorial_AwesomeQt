---
title: exec 深处——异常安全 RAII 与退出协议
description: QEventLoop::exec 无 try/catch、异常安全由栈上 LoopReference RAII 承担（exceptionCaught 默认真、异常穿过打警告并复原 loopLevel，异常继续上抛）、循环唯一判据 exit 原子标志、QEventLoopPrivate 生来 exit=true、exec 进循环前清残留 Quit、入口持 QThreadPrivate::mutex 查 quitNow 防与 QThread::exit 竞态、QCoreApplication::quit 走 QEvent::Quit 事件链（in_exec 门禁、主线程同步跨线程投递、compressEvent 压缩）、exec 两道 qWarning 闸非断言。
---

# exec 深处——异常安全 RAII 与退出协议

> 本索引收录 Qt 6.9.1 源码中事件循环 exec 深水区相关的已验证证据。对应专家篇《20 事件循环源码》正文 exec 的深处一节。07 篇的 while 心脏与 exit/interrupt 主线证据见 [event-loop-exec](./event-loop-exec.md)，不重复收录。

## 异常安全：LoopReference RAII

源码文件：`qtbase/src/corelib/kernel/qeventloop.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| exec 全函数体无 try/catch，异常安全押在栈上 LoopReference | :145-160 | `struct LoopReference { ... bool exceptionCaught; LoopReference(...) : exceptionCaught(true) { d->inExec = true; d->exit.storeRelease(false); ++threadData->loopLevel; threadData->eventLoops.push(...); locker.unlock(); }` | 构造时 exceptionCaught 默认真、复位 exit、抬 loopLevel、压 eventLoops 栈。 |
| 析构收尾：异常穿过时打警告并复原，异常本身继续上抛 | :162-176 | `if (exceptionCaught) { qWarning("Qt has caught an exception thrown from an event handler..."); ...--threadData->loopLevel; ...}` | 正常跑完在循环外置 exceptionCaught=false，异常路径不吞不捕。 |
| 循环唯一判据是 exit 原子标志，processEvents 返回值被丢弃 | :185-189 | `while (!d->exit.loadAcquire()) processEvents(flags \| WaitForMoreEvents \| EventLoopExec); ref.exceptionCaught = false; return d->returnCode.loadRelaxed();` | 返回 false 不退出循环。 |
| exec 进循环前清掉上一轮残留的 Quit 事件 | :180-183 | 清残留 Quit 的收尾动作 | 与 QCoreApplication::quit 的 postEvent 路径呼应。 |
| 入口持 QThreadPrivate::mutex 检查 quitNow，防与 QThread::exit 竞态 | :135-143 | `QMutexLocker locker(...QThreadPrivate...mutex); if (threadData->quitNow) return -1;` | 注释原话 race condition with QThread::exit。 |

## QEventLoopPrivate 与 quit 家族

源码文件：`qtbase/src/corelib/kernel/qeventloop_p.h` / `qcoreapplication.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QEventLoopPrivate 四字段，构造置 returnCode=-1、exit=true | qeventloop_p.h:27-39 | `QAtomicInt quitLockRef; QBasicAtomicInt exit; QBasicAtomicInt returnCode; bool inExec;` | 生来「未运行」，exec 的 LoopReference 才置 false；isRunning() 即 !exit。 |
| quitLockRef 配合 QEventLoopLocker，deref 时 postEvent(QEvent::Quit) | qeventloop_p.h:46-51 | deref 投递 Quit | 锁存器清零即请求退出。 |
| QCoreApplication::quit：in_exec 门禁后主线程同步 sendEvent、跨线程 postEvent QEvent::Quit | qcoreapplication.cpp:2090-2111 | `if (!self->d_func()->in_exec) return; ... QEvent quitEvent(QEvent::Quit); sendEvent / postEvent` | 不是静态直拿当前线程 eventLoop；event 收到 Quit 转 exit(0)。 |
| QCoreApplication::exit 先发 aboutToQuit 再遍历 eventLoops 逐个 exit | qcoreapplication.cpp:1502-1517 | aboutToQuit + 遍历 eventLoops | 真正逐个退场的是它。 |
| exec 两道闸是非主线程与「已在跑」，qWarning + return -1 非断言 | qcoreapplication.cpp:1435-1443 | `if (threadData != QThreadData::current()) { qWarning("...Must be called from the main thread"); return -1; }` | release 版也过但拿 -1。 |
