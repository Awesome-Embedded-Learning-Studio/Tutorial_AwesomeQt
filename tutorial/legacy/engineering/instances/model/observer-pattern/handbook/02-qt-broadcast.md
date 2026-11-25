---
title: "阶段 2：Qt 信号槽 Subject + 一对多广播"
description: "搓 QtSubject（继承 QObject + signals valueChanged），三个面板连同一信号实现一对多广播，对照阶段 1 看信号槽怎么替代整个 notify 循环。"
---

# 阶段 2：Qt 信号槽 Subject + 一对多广播

← [手册首页](./index.md) · 上一阶段 [纯 C++ Observer](./01-classic-observer.md) · 下一阶段 [连接类型与坑](./03-connect-types-and-pitfalls.md) →

## 目标

搓一个 `QtSubject`（继承 `QObject`，带 `signals: void valueChanged(double)`），再搓三个面板（`QLabel` 数值 / `QProgressBar` 进度条 / `QPlainTextEdit` 历史），用 `connect` 把它们**全部连到同一个信号**。点按钮推一个随机温度，三个面板同时刷——这就是一对多广播。对照阶段 1：Subject 端这次**不写任何列表、不写任何 for 循环**。

### 提示

- `QtSubject : public QObject`，类里必须有 `Q_OBJECT`（否则信号 moc 不生成）
- `signals:` 段下声明 `void valueChanged(double temperature)`——**只在头文件声明，不写实现**（moc 生成）
- 公有 `setValue(double)`：存成员 `value_` 后 `emit valueChanged(temperature)`。`emit` 是个 no-op 宏，但写上表意图
- 三个面板用 Qt6 函数指针语法连：`connect(subject_, &AwesomeQt::QtSubject::valueChanged, this, [this](double t){ ... })`
- 第一个槽（面板 A）：`value_label_->setText(QString("temperature: %1 C").arg(t, 0, 'f', 2))`
- 第二个槽（面板 B）：把温度 `std::clamp(0,100)` 后映射成进度条值（`std::round` + `static_cast<int>`）
- 第三个槽（面板 C）：`history_view_->appendPlainText(...)`
- 点按钮：`subject_->setValue(随机温度)`——一行调，三个槽都跑

### 检查点

点一次「Push random temperature」按钮，**三个面板同时变**：label 显示数值、进度条跳到对应刻度、history 多一行 = 一对多广播通了。
**关键体会**：阶段 1 你手写了 attach/detach/notify 一整套；这里 `QtSubject` 完全不知道有几个面板，`setValue` 只 `emit` 一次。这就是信号槽相对 GoF observer list 的核心简化。

> 信号槽不熟？[信号与槽（入门）](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、[信号与槽（进阶）](../../../../../advanced/01-qtbase/02-signal-slot-advanced.md)。

### 对照答案

- `QtSubject` 声明 + `Q_OBJECT` + `signals`：`include/observer-pattern.h:66`、`include/observer-pattern.h:89`
- `valueChanged` 信号签名：`include/observer-pattern.h:92`
- `setValue` 的 emit（一行替代整个 notify 循环）：`src/observer-pattern.cpp:50`
- 三个面板连同一信号：`demo/observer-pattern_window.cpp:145/149/157`
- 点按钮触发广播：`demo/observer-pattern_window.cpp:167`

---

### 这一阶段的坑（必踩一个）

- **忘了 `Q_OBJECT`**：信号 moc 不生成，`connect` 编译期找不到 `&QtSubject::valueChanged`（链接期报 undefined reference）。CMake 确认 `set(CMAKE_AUTOMOC ON)`（`CMakeLists.txt` 里有）。
- **用了 `SIGNAL()/SLOT()` 字符串宏**：`connect(subject_, SIGNAL(valueChanged(double)), ...)` 能编过，但签名拼错运行期才报「连不上」。Qt6 一律函数指针语法，连错编译期就报。
- **三个面板的槽里忘了判空 / 用了悬空成员**：构造时 `connectPanels()` 必须在所有面板 `new` 完之后调，否则 lambda 捕获的 `this` 访问到的面板是野的。

下一阶段：[玩连接类型 DirectConnection/QueuedConnection + disconnect + lambda 坑](./03-connect-types-and-pitfalls.md)。
