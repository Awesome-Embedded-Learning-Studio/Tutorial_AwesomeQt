---
title: "阶段 3：连接类型 + 断开 + lambda 生命周期坑"
description: "搓连接类型切换（DirectConnection/QueuedConnection）、用 QMetaObject::Connection 断开面板、演示 lambda 连接必须给 context 对象的生命周期坑。"
---

# 阶段 3：连接类型 + 断开 + lambda 生命周期坑

← [手册首页](./index.md) · 上一阶段 [Qt 信号槽广播](./02-qt-broadcast.md) →

## 目标

把信号槽的三个进阶点演全：①切换 `DirectConnection`(同步)/`QueuedConnection`(异步) 重连面板 C；②用 `QMetaObject::Connection` 断开面板 C（演示最稳的断开方式）；③演示 lambda 连接不给 context 的生命周期坑，并给出 context 守护的正路。

## Step 1：连接类型切换（同步 vs 异步）

### 目标

两个单选框（同步/异步），点「Reconnect」按钮把面板 C 按当前选择重连。重连后 history 日志带 `[qt-direct]` 或 `[qt-queued]` 前缀，能看出走的是哪条路。

### 提示

- 定义个枚举（成品叫 `ConnectionFlavor { kSynchronous, kAsynchronous }`），放 `QtSubject` 里 + `Q_ENUM`
- 单选框用 `QButtonGroup` 管，`connect(group, &QButtonGroup::idClicked, ...)` 监听切换，更新当前选择
- 重连：先 `disconnect(history_conn_)`，再 `connect(...)` 时第 5 个参传 `Qt::ConnectionType`：`Qt::QueuedConnection` 或 `Qt::ConnectionType::DirectConnection`
- `connect` 带 `ConnectionType` 的重载是 5 参版（sender, sig, context, functor, type）——**必须给 context**（这里给 `this`），否则 lambda 重载和 type 参数匹配不上

### 检查点

切到「QueuedConnection」后点 Reconnect，再推温度，面板 C 日志每行带 `[qt-queued]` = 连接类型切过去了。
**关键体会**：同步是 emit 处直接调槽、阻塞返回；异步是投事件到事件循环、emit 立刻返回。同线程里两者结果一样，但跨线程时只有 `QueuedConnection` 安全（槽跑在 receiver 线程）。

### 对照答案

- `ConnectionFlavor` 枚举 + `Q_ENUM`：`include/observer-pattern.h:72/76`
- 重连时按选择传 `ConnectionType`：`demo/observer-pattern_window.cpp:183`
- 5 参 connect（带 context + type）：`demo/observer-pattern_window.cpp:186`
- 单选框 `QButtonGroup::idClicked` 监听：`demo/observer-pattern_window.cpp:135`

---

## Step 2：断开连接（最稳的 disconnect）

### 目标

点「Disconnect panel C」按钮，面板 C 不再接收后续更新。要求用**最稳的断开方式**。

### 提示

- `connect` 的返回值类型是 `QMetaObject::Connection`——把它存进成员 `history_conn_`
- 断开就一行 `disconnect(history_conn_)`——无歧义，最稳
- 想想：为什么不写 `disconnect(subject_, &QtSubject::valueChanged, this, lambda)`？因为 lambda 没有 `operator==`，拿不回原 functor，编译器找不回那条连接
- 断开后给个布尔标志 `history_connected_`，重复点 disconnect 时给「已经断开了」提示，别崩

### 检查点

点 Disconnect，history 出现「panel C disconnected」；再推温度，面板 A/B 还在动，面板 C 不再变 = 断开对了。

### 对照答案

- 存 `connect` 返回值：`demo/observer-pattern_window.cpp:156`（`history_conn_ = connect(...)`）
- `disconnect(conn)` 一行断：`demo/observer-pattern_window.cpp:209`
- 注释解释为什么不能按 lambda 断：`demo/observer-pattern_window.cpp:203`

---

## Step 3：lambda 连接的生命周期坑（context 守护）

### 目标

演示：lambda 连接捕获了对象指针，若该对象先销毁、信号还在发，回调就崩。给出正路——connect 第 3 参给 context 对象，context 销毁时连接自动失效。

### 提示

- `new QLabel()` 一个临时 context（**不给父对象**，模拟独立生命周期）
- `connect(subject_, &QtSubject::valueChanged, context_label, [context_label, this](double t){ ... })`——第 3 参是 context
- 先触发一次（`subject_->setValue(...)`）：context 还活着，槽跑，日志出现
- `context_label->deleteLater()`：context 进了删除队列，连接随之失效
- 再触发信号：槽**不会**被调（context 没了，连接断了），不崩
- 对比坑：如果把 `context_label` 换成无 context 的 `connect(subject_, sig, lambda)`，lambda 捕获的 `context_label` 被 delete 后，再 emit 就回调到野指针 → **segfault**

### 检查点

点「Demo: lambda trap」按钮，lambda 日志先出现一行（context 活着时），再出现「context scheduled for deletion」提示 = 守护机制演完了。
**关键体会**：给 context 的连接是「跟着 context 走」的，context 没了连接自动断——这是 Qt 信号槽相对纯 C++ observer list 的又一层安全网（阶段 1 你得自己保证 detach）。

> 连接 / 断开 / 连接类型进阶？[信号与槽（进阶）](../../../../../advanced/01-qtbase/02-signal-slot-advanced.md)。

### 对照答案

- 临时 context + 带守护的 connect：`demo/observer-pattern_window.cpp:224`
- 触发后 deleteLater（自动断连）：`demo/observer-pattern_window.cpp:236`
- 注释解释无 context 的坑：`demo/observer-pattern_window.cpp:214-218`

---

## 收尾

搓完这三阶段，你已经把观察者模式的 Qt 实现和纯 C++ 实现都过了一遍，能说清：信号槽 = 编译期类型安全 + 自动生命周期管理 + 一行 emit 广播的观察者。
卡住了去 [卡住怎么办](./troubleshooting.md)。想再深一层（桥接两条路 / 多线程温度源）回 [手册首页](./index.md) 的进阶挑战。
