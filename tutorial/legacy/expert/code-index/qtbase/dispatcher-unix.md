---
title: UNIX 分发器——eventfd、poll 一轮与 timer 表
description: wakeUp 在 Linux 写 eventfd 值 1（pipe 写 0 字节是回退，wakeUps 原子位去重）、canWait 三条件（canWaitLocked 且未 interrupt 且 WaitForMoreEvents）、poll 超时是 QDeadlineTimer（timerWait 首个未激活 timer 剩余时间、无 timer 时 Forever、EINTR 按剩余重算）、pollfd 每轮现算、SocketNotifier 三槽 QHash 与 SockAct 两段接力（activated 发射点在 QSocketNotifier::event）、QTimerInfo 二分插入加 rotate 回插加 activateRef 防递归、CoarseTimer 主动对齐边界（5% 上限挤同一刻醒）、interrupt 置标志加 wakeUp。
---

# UNIX 分发器——eventfd、poll 一轮与 timer 表

> 本索引收录 Qt 6.9.1 源码中 QEventDispatcherUNIX 平台后端相关的已验证证据。对应专家篇《20 事件循环源码》正文平台后端下潜一节（glib 侧另见 [dispatcher-glib](./dispatcher-glib.md)）。

## wakeUp 与阻塞

源码文件：`qtbase/src/corelib/kernel/qeventdispatcher_unix.cpp` / `qcore_unix.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| wakeUp 在 Linux 写 eventfd 值 1，pipe 写 0 字节是回退 | :131-141 | `if ((wakeUps.fetchAndOrAcquire(1) & 1) == 0) { ... eventfd_write(fds[0], 1); ... char c = 0; qt_safe_write(fds[1], &c, 1); }` | init（110-121）优先 eventfd(0, EFD_NONBLOCK\|EFD_CLOEXEC)；wakeUps 原子位保证连写只落一次，check()（143-173）消费后复位。老教程「写一个字节」在 Linux 构建不成立。 |
| canWait 三条件 | :420-426 | `const bool canWait = (threadData->canWaitLocked() && !d->interrupt.loadRelaxed() && wait_for_events);` | posted events 间接参与：postEvent 置 data->canWait=false，sendPostedEvents 清空置回。 |
| poll 超时是 QDeadlineTimer，无 timer 时 Forever | :434-446 | `std::optional<nanoseconds> remaining = d->timerList.timerWait(); deadline = remaining ? QDeadlineTimer{*remaining} : QDeadlineTimer(QDeadlineTimer::Forever);` | canWait 假时用已过期 deadline 保证至少跑一轮（注释 ensures the code in the do-while loop in qt_safe_poll runs at least once）。 |
| poll 主体：pollfd 现算、唤醒管道垫尾、qt_safe_poll | :448-459 | `d->pollfds.append(d->threadPipe.prepare()); ... qt_safe_poll(d->pollfds.data(), d->pollfds.size(), deadline)` | pollfd 数组不持久维护，每轮重建；qt_safe_poll（qcore_unix.cpp:121-142）优先 ppoll，EINTR 按剩余重算。 |
| interrupt：置标志再加 wakeUp | :500-505 | `d->interrupt.storeRelaxed(1); wakeUp();` | 下一轮 processEvents 入口 412 复位。 |

## SocketNotifier 与 timer 表

源码文件：`qtbase/src/corelib/kernel/qeventdispatcher_unix.cpp` / `qeventdispatcher_unix_p.h` / `qsocketnotifier.cpp` / `qtimerinfo_unix.cpp` / `qtimerinfo_unix_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| SocketNotifier 按 fd 三槽注册进 QHash | qeventdispatcher_unix.cpp:357-366（表结构 qeventdispatcher_unix_p.h:128-142） | `QSocketNotifierSetUNIX &sn_set = d->socketNotifiers[sockfd]; sn_set.notifiers[type] = notifier;` | 三槽映射 POLLIN/POLLOUT/POLLPRI；同 fd 同类型重复注册 qWarning。 |
| 激活两段接力：markPending 入队再 sendEvent(SockAct) | :242-259 | `QEvent event(QEvent::SockAct); while (!pendingNotifiers.isEmpty()) { ... QCoreApplication::sendEvent(notifier, &event); }` | activated 信号真正发射点在 QSocketNotifier::event()（qsocketnotifier.cpp:323-332，case SockAct 里 emit activated(d->sockfd, d->sntype, QPrivateSignal())）。 |
| setEnabled 直连 dispatcher 注册注销 | qsocketnotifier.cpp:301-304 | `snenabled ? registerSocketNotifier : unregisterSocketNotifier` | 不是本地开关；析构先 setEnabled(false)（169-172）。 |
| QTimerInfo 字段与容器 | qtimerinfo_unix_p.h:37-42 | `TimePoint timeout; Duration interval; Qt::TimerId id; Qt::TimerType timerType; QObject *obj; QTimerInfo **activateRef;` | activateRef 是防递归自引用；容器 QList 按 timeout 升序。 |
| timerInsert 用 upper_bound 二分插入 | qtimerinfo_unix.cpp:49-59 | `timers.insert(std::upper_bound(timers.cbegin(), timers.cend(), ti, byTimeout), ti);` | 非旧版尾部线性扫描（5.15 对照实证为线性）。 |
| CoarseTimer 注册时边界修正 | :280-290 | 间隔 ≥20s 降 VeryCoarse、≤20ms 升 Precise | 与 11 篇 TimerType 三档互证。 |
| calculateCoarseTimerTimeout 主动对齐边界 | :96-116 | 对齐整秒/500/250/200/100/50/25ms 边界 | 误差上限 5%，多数 timer 挤同一刻醒省唤醒；roundToMillisecond ceil 向上取整绝不提前。 |
| activateTimers：find_if 判到期、sendEvent 直投、rotate 回插、activateRef 防递归 | :408-416（判到期 385-386） | `if (!currentTimerInfo->activateRef) { ... QTimerEvent e(...); QCoreApplication::sendEvent(...); }` | dispatcher 不直接调 timerEvent，经 QObject::event 转（07 篇挂链）。 |
