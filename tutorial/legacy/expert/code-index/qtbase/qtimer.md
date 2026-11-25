---
title: QTimer 源码索引
description: QTimer 继承 QObject 的软件定时器地基、start→QObject::startTimer→dispatcher 注册链、event case Timer→timerEvent→timeout 派发、TimerType 三档（Precise/Coarse/VeryCoarse）与容差、stop/killTimer 注销、QBasicTimer 绕过 startTimer 直接走 dispatcher、singleShot 用 QSingleShotTimer 私有类。
---

# QTimer 源码索引

> 本索引收录 Qt 6.9.1 源码中 QTimer 的已验证证据。事件循环 dispatcher 的 registerTimer/unregisterTimer/processEvents 见 [事件循环](./event-dispatch.md)（EL 块），本文件聚焦 QTimer/QBasicTimer 本身。

## QTimer 身份与 start 注册链

源码文件：`qtbase/src/corelib/kernel/qtimer.h` / `qtimer.cpp` / `qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QTimer 继承 QObject + timeout 信号 | qtimer.h:19-21, :114-115 | `class QTimer : public QObject { Q_OBJECT` / `void timeout(QPrivateSignal);` | 基于 QObject 才能<thread>挂线程、收 Timer 事件、发信号。 |
| start 调 QObject::startTimer | qtimer.cpp:209-220 | `Qt::TimerId newId{ QObject::startTimer(d->inter * 1ms, d->type) };` | TimerType 经 d->type 属性传。已激活先 stop（restart 换新 id）。 |
| QObject::startTimer 三校验 + 调 dispatcher | qobject.cpp:1886-1912 | `if (interval<0ns)... if (!hasEventDispatcher)... if (thread()!=currentThread)... dispatcher->registerTimer(interval, timerType, this); runningTimers.append(timerId);` | 三校验：非负/有 dispatcher/同线程。真正到点叫醒的是 dispatcher。 |

## Timer 事件派发与 timeout

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qtimer.cpp` / `qcoreevent.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| event case Timer 调 timerEvent | qobject.cpp:1402-1407, qcoreevent.h:63 | `case QEvent::Timer: timerEvent((QTimerEvent *)e);` / `Timer = 1` | = EL18。事件循环→QObject 入口。 |
| QTimer::timerEvent 核对 id + emit timeout | qtimer.cpp:279-287 | `if (e->id() == d->id) { if (d->single) stop(); emit timeout(QPrivateSignal()); }` | 一个 QObject 可挂多个 timer 靠 id 区分。single 先 stop 后 emit（槽里 isActive 已 false）。 |
| QObject::timerEvent 默认空 | qobject.cpp:1482-1484 | `void QObject::timerEvent(QTimerEvent *) { }` | 普通 QObject 注册了 timer 事件也被吞，除非子类重写——QTimer 存在的意义。 |

## setSingleShot

源码文件：`qtbase/src/corelib/kernel/qtimer.cpp` / `qtimer_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| single 是 bindable 属性，默认 false | qtimer.cpp:580-583, qtimer_p.h:65 | `setSingleShot(bool b) { d_func()->single = b; }` / `Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QTimerPrivate, bool, single, false)` | 真正「单次」逻辑在 timerEvent（TI05 的 if single stop）。 |

## TimerType 三档与容差

源码文件：`qtbase/src/corelib/global/qnamespace.h` / `kernel/qtimer.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| TimerType 三档（注意 VeryCoarseTimer） | qnamespace.h:1684-1688 | `enum TimerType { PreciseTimer, CoarseTimer, VeryCoarseTimer };` | StCoarseTimer 不存在（常见误记）。Qt6.8+ 加了 Qt::TimerId 强类型（:1690）。 |
| 容差数值（官方文档原文） | qtimer.cpp:80-98 | Precise 力求 1ms 且绝不提前 / Coarse 容差=间隔 5% / VeryCoarse 容差=500ms / overrun 只发一次 timeout | Precise「never earlier」只单独适用；Coarse/VeryCoarse 可提前（省唤醒，对齐 OS 唤醒点是实现层细节）。三种都可能晚到。 |

## stop / killTimer 注销链

源码文件：`qtbase/src/corelib/kernel/qtimer.cpp` / `qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| stop→killTimer 两校验 + 注销 | qtimer.cpp:265-273, qobject.cpp:1932-1958 | `QObject::killTimer(d->id)` / killTimer: 同线程校验 + runningTimers.indexOf 合法 → `dispatcher->unregisterTimer(id)` + `runningTimers.remove(at)` + `releaseTimerId` | killTimer 实质两校验（同线程 + 表内 indexOf 合法），与 startTimer 的真三校验（非负/有dispatcher/同线程）区分。id 不属该对象 qWarning 拒绝。 |

## QBasicTimer 与 singleShot

源码文件：`qtbase/src/corelib/kernel/qbasictimer.h` / `.cpp` / `qtimer.cpp` / `qsingleshottimer_p.h` / `qsingleshottimer.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QBasicTimer 不继承 QObject、绕过 startTimer | qbasictimer.h:18-28, qbasictimer.cpp:179-197 | `class QBasicTimer { Qt::TimerId m_id; ... }` / start: `m_id = eventDispatcher->registerTimer(duration, timerType, obj);` | 直接走 dispatcher，不进 QObject 的 runningTimers 簿记。自称 fast/low-level 用于 Qt 内部。 |
| singleShot 用 QSingleShotTimer 私有类 | qtimer.cpp:318-352, qsingleshottimer_p.h:32-36, qsingleshottimer.cpp:60-64 | `if (ns==0ns) invokeMethodImpl(...QueuedConnection...) else (void) new QSingleShotTimer(...)` / `class QSingleShotTimer : public QObject { Q_OBJECT; QBasicTimer timer; }` / `timerFinished() { Q_EMIT timeout(); delete this; }` | 0ms 走 invokeMethod 不创 timer；ns>0 new QSingleShotTimer（持 QBasicTimer 不是 QTimer），自毁。 |

## 查询接口

源码文件：`qtbase/src/corelib/kernel/qtimer.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| timerId/interval/remainingTime | :178-182, :640-643, :661-671 | `timerId: id() 的 int 包装（未激活 -1）` / `interval: return d_func()->inter` / `remainingTime: dispatcher->remainingTime(d->id)` | interval 读本地属性；remainingTime 实时查 dispatcher——证明 QTimer 没自己的时间内核。 |
