---
title: QThread 源码索引
description: QThread 是管理者非线程本身、run/exec 双核、start→平台创建→QThreadPrivate::start 入口、QThreadData 线程身份包、started/finished 信号时机、quit/exit/协作式中断/terminate 危险、wait/setPriority、QThread::create lambda 工厂、QObject 构造绑 threadData（线程亲和根源）。
---

# QThread 源码索引

> 本索引收录 Qt 6.9.1 源码中 QThread 的已验证证据。QObject 的线程亲和（thread()/moveToThread）在 [对象树所有权](./object-tree-thread-affinity.md) 已有覆盖，本文件聚焦 QThread 本身的 lifecycle。事件循环 QEventLoop 的内部见 [事件循环](./event-loop-exec.md)。

## QThread 身份与线程亲和根源

源码文件：`qtbase/src/corelib/thread/qthread.h` / `kernel/qobject.cpp` / `kernel/qobject_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QThread 继承 QObject（管理者非线程本身） | qthread.h:27-29 | `class Q_CORE_EXPORT QThread : public QObject { Q_OBJECT` | QThread 实例本身是个 QObject，住在创建它的线程（通常主线程），管理的那个新线程是另一回事。 |
| QObject 构造从当前线程绑 threadData | qobject.cpp:946-955 | `auto threadData = (parent && !parent->thread()) ? parent->d_func()->threadData.loadRelaxed() : QThreadData::current(); threadData->ref(); d->threadData.storeRelaxed(threadData);` | 「在哪个线程 new 这 QObject，它就属于哪个线程」。三目：parent 存在且 parent 无亲和性时继承 parent 的 TD，否则取当前线程。thread() 读它（qobject.cpp:1610）。 |

## run/exec 双核

源码文件：`qtbase/src/corelib/thread/qthread.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| run 虚函数默认调 exec | qthread.h:112, qthread.cpp:778 | `virtual void run();` / `void QThread::run() { (void) exec(); }` | 子类重写 run 不调 exec 就没事件循环——worker 模式 vs subclass run 模式分水岭。 |
| exec 栈上构造 QEventLoop | qthread.cpp:644-665 | `QEventLoop eventLoop; int returnCode = eventLoop.exec();` | 非 virtual 不能重写。进入前清 quitNow，查 exited 决定是否直接返回旧 retcode。 |

## start→平台创建→入口

源码文件：`qtbase/src/corelib/thread/qthread_unix.cpp` / `qthread_win.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| start 经 pthread_create / CreateThread | unix:847, win:358 | `pthread_create(&threadId, &attr, QThreadPrivate::start, this)` / `CreateThread(..., QThreadPrivate::start, this, CREATE_SUSPENDED, ...)` | Win 用 CREATE_SUSPENDED 挂起创建、设完优先级再 ResumeThread。失败 threadState 回退 NotStarted。 |
| QThreadPrivate::start 平台入口流程 | unix:382-441, win:143-191 | `set_thread_data(data); data->ensureEventDispatcher(); emit thr->started(QPrivateSignal()); thr->run(); finish()` | 两平台基本一致。Unix 用 pthread_cleanup_push/pop 保证 pthread_cancel 也调 finish；入口先 DISABLE cancel、emit started 后才 ENABLE + pthread_testcancel。 |

## QThreadData 线程身份包

源码文件：`qtbase/src/corelib/thread/qthread_p.h` / `qthread.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QThreadData 结构（每线程一个） | qthread_p.h:302-376 | `QStack<QEventLoop *> eventLoops; QPostEventList postEventList; QAtomicPointer<QThread> thread; QAtomicPointer<void> threadId; QAtomicPointer<QAbstractEventDispatcher> eventDispatcher; QList<void *> tls; bool quitNow; bool canWait; bool isAdopted;` | eventLoops 栈支持嵌套 exec；postEventList 优先级排序；isAdopted 区分 Qt 创建 vs 外部收养。还含 loopLevel/scopeLevel/requiresCoreApplication。 |
| currentThread 经 thread-local | qthread.cpp:387-392 | `QThreadData *data = QThreadData::current(); return data->thread.loadAcquire();` | QThreadData::current 先查 thread-local，查不到就 createCurrentThreadData + new QAdoptedThread（无 Qt 的外部线程也能用）。 |

## 信号时机

源码文件：`qtbase/src/corelib/thread/qthread_unix.cpp` / `qthread_win.cpp` / `qthread.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| started 在新线程 run 之前发 | unix:429, win:185 | `emit thr->started(QThread::QPrivateSignal()); ... thr->run();` | QPrivateSignal 禁外部手动 emit。新线程发，cross-thread 槽可能延迟到 run 已开始后才到。 |
| finished 在 finish() 里 Finishing 后立刻发 | unix:443-461, win:237-249 | `d->threadState = Finishing; emit thr->finished(QPrivateSignal()); QCoreApplication::sendPostedEvents(nullptr, DeferredDelete);` | 紧跟 DeferredDelete——让 deleteLater 对象在事件循环已停后仍能清。**注意**：terminate() 路径下 finished 发送线程 undefined（qthread.cpp:353-354 `\note`）。 |

## 退出、中断、terminate

源码文件：`qtbase/src/corelib/thread/qthread.cpp` / `qthread_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| quit/exit 设标志 + 遍历退 eventLoops | qthread.cpp:741-752 | `d->exited=true; d->returnCode=returnCode; d->data->quitNow=true; for (...) eventLoop->exit(returnCode);` | 支持嵌套 exec。不真停线程，run 没调 exec 则无效。 |
| requestInterruption 协作式（atomic bool） | qthread.cpp:1275-1286, qthread_p.h:199 | `if (d->threadId() == theMainThreadId.loadAcquire()) { qWarning(...); return; } ... d->interruptionRequested.store(true, relaxed);` | 不碰调度，靠 run() 自己 poll isInterruptionRequested()。主线程拒绝判定基于 theMainThreadId（QCoreApplication 创建线程，无 GUI 也生效）。isInterruptionRequested fast/slow 双路径。 |
| terminate 危险 + 平台差异 | unix:868-899, win:415-428, qthread.cpp:904 | Unix `if (d->terminated) return; pthread_cancel(...)`（防 ABA）/ Win `TerminateThread(d->handle, 0); d->finish(false)` / Android 空实现 | 文档 `\warning` 危险且不鼓励。Win 的 finish(false) lockAnyway=false 因可能从 wait/terminate 已持锁。 |

## wait / setPriority

源码文件：`qtbase/src/corelib/thread/qthread.cpp` / `qthread_unix.cpp` / `qthread_win.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| wait 三门 + 平台 join | qthread.cpp:950-962, unix:914, win:430 | NotStarted/Finished 直返 / 自等自 qWarning / 否则 `d->wait(locker, deadline)` | Unix pthread_clockjoin_np / pthread_timedjoin_np，Win WaitForSingleObject。 |
| Priority 7 档 + setPriority 守卫 | qthread.h:40-52, qthread.cpp:802-815 | `enum Priority { IdlePriority, ..., TimeCriticalPriority, InheritPriority }` / setPriority `if (priority == InheritPriority) return; if (threadState != Running) return;` | InheritPriority 是 start 参数语义不能运行中改；非 Running 拒绝。 |

## QThread::create 现代 lambda 工厂

源码文件：`qtbase/src/corelib/thread/qthread.h` / `qthread.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| create 经 std::async(deferred) + QThreadCreateThread 子类 | qthread.h:84-85+131-144, qthread.cpp:1347-1369 | `[[nodiscard]] static QThread *create(Function &&f, Args &&... args)` / `createThreadImpl(std::async(std::launch::deferred, ...))` / `QThreadCreateThread::run() { m_future.get(); }` / `~QThreadCreateThread() { requestInterruption(); quit(); wait(); }` | lambda 经 deferred 包成 future，子类 run 调 get() 触发在新线程执行。[[nodiscard]] 禁忽略，析构自动安全收尾。start 只能调一次。 |
