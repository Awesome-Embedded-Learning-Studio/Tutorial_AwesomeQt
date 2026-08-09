---
title: "阶段 1：纯 C++ Observer 接口 + ClassicSubject"
description: "搓 GoF 原版观察者：Observer 纯虚接口 + ClassicSubject 维护裸指针列表 + attach/detach/notify 三件套，作为对照基准。"
---

# 阶段 1：纯 C++ Observer 接口 + ClassicSubject

← [手册首页](./index.md) · 下一阶段 [Qt 信号槽广播](./02-qt-broadcast.md) →

## 目标

搓出 GoF 教科书版的观察者：一个 `Observer` 接口（纯虚 `onUpdate`）+ 一个 `ClassicSubject`（维护 `vector<Observer*>`，提供 `attach/detach/notify`）。再搓一个 `TextObserver` 把收到的温度写进一个 `QPlainTextEdit`。这一步**完全不碰 Qt 信号槽**——它是后面 Qt 路径的对照基准。

### 提示

- `Observer` 放 `AwesomeQt::` 命名空间，只一个 `virtual void onUpdate(double) = 0`，虚析构给 `default`（接口必备）
- `ClassicSubject` 不继承任何东西（纯 C++ 类），成员 `std::vector<Observer*> observers_`——**裸指针，不持有所有权**
- `attach(Observer*)`：`push_back` 之前防御一下空指针
- `detach(Observer*)`：`std::find` + `erase`，只删第一个匹配（GoF 语义）
- `notify(double)`：range-based for 遍历，逐个 `observer->onUpdate(temp)`——**禁 Q_FOREACH**
- `TextObserver : public Observer`，构造接一个 `QPlainTextEdit*` 存成员，`onUpdate` 里 `appendPlainText`
- demo 里 `new` 一个 `ClassicSubject`（用 `std::unique_ptr` 持）、`new` 一个 `TextObserver`（传面板指针），`subject->attach(observer.get())`

### 检查点

手动调 `classic_subject_->notify(42.5)`，面板 D 里出现一行 `[classic] temperature = 42.50` = 通知链通了。
然后 `detach` 再 `notify`，面板不再新增行 = 注销对了。
**关键体会**：你现在手写了 attach/detach/notify 全套——记住这个工作量，下一阶段看 Qt 怎么一行 `emit` 替代掉它。

> 信号槽 / 虚函数回调机制不熟？先读 [信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。

### 对照答案

- `Observer` 纯虚接口：`include/observer-pattern.h:28`
- `ClassicSubject` 三件套声明：`include/observer-pattern.h:47/51/55`
- `attach/detach/notify` 实现：`src/observer-pattern.cpp:17/24/32`
- `notify` 的裸指针 for 循环（悬空风险点）：`src/observer-pattern.cpp:35`
- demo 里 attach + 手动 notify：`demo/observer-pattern_window.cpp:49` 与 `demo/observer-pattern_window.cpp:246`
- `TextObserver::onUpdate`：`demo/observer-pattern_window.cpp:29`

---

### 这一阶段的坑（必踩一个）

- **detach 忘了调，observer 先析构**：`notify` 遍历到悬空指针 → **segfault**。纯 C++ 路径没有 Qt 对象树那种自动失效，必须严格配对 attach/detach。成品在析构前没 detach 是因为 `unique_ptr` 成员析构顺序保证了 subject 先于 observer 销毁——但这是个脆平衡，别在生产代码里赌。

下一阶段：[把 Qt 信号槽版搓出来，一行 emit 替代整个 notify 循环](./02-qt-broadcast.md)。
