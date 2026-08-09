---
title: "Observer Pattern 手搓手册"
description: "从空 main 一行行搓出观察者模式：先搓纯 C++ Observer 接口做对照基准，再用 Qt 信号槽做一对多广播，最后玩连接类型与 lambda 生命周期坑。"
---

# Observer Pattern 手搓手册

> **source**：成品答案在 `model/12-design-patterns/observer-pattern/`（做完对照）· **related**：[信号与槽（入门）](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 observer，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **观察者模式的两种实现**：纯 C++ `Observer` 接口 + `attach/detach/notify`（GoF 原版）vs Qt 信号槽——理解后者为什么是「编译期类型安全的观察者」
- **一对多广播**：一个信号连多个槽，Subject 端零感知
- **Qt6 函数指针连接语法**：`connect(sender, &Cls::sig, receiver, &Cls::slot)`，编译期类型检查
- **连接类型**：`DirectConnection`(同步) vs `QueuedConnection`(异步)
- **断开连接的几种方式** + **lambda 连接的生命周期坑**（context 守护）

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    w.resize(200, 120);
    w.show();
    return app.exec();
}
```

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。

## 2. 任务清单

分 3 阶段，每阶段：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| 阶段 | 目标 | 进 |
|---|---|---|
| 1 | 搓纯 C++ `Observer` 接口 + `ClassicSubject`（对照基准） | [01](./01-classic-observer.md) |
| 2 | 搓 Qt 信号槽 `QtSubject` + 一对多广播（三个面板连同一信号） | [02](./02-qt-broadcast.md) |
| 3 | 连接类型切换 + disconnect + lambda 生命周期坑 | [03](./03-connect-types-and-pitfalls.md) |

成品对照：`model/12-design-patterns/observer-pattern/`（按 [成品导览](../) 的「怎么读」顺序对照）。

**为什么先搓纯 C++ 再搓 Qt？** 不先搓一遍纯 C++ 的 `attach/detach/notify`，你就体会不到信号槽省掉了多少手动列表管理——先痛一遍再看 Qt，对比才深刻。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **桥接两条路**：写个 adapter，把一个 `Observer*` 包成 lambda 连到 `QtSubject::valueChanged`，让老式 observer 也能挂进 Qt 世界。思考：这个 adapter 谁管 observer 的生命周期？
- **多线程温度源**：把 `QtSubject` 挪到工作线程（`QThread` + `moveToThread`），面板留在主线程，用 `QueuedConnection` 刷新。验证：把连接类型写死 `DirectConnection`，槽跑在哪个线程？为什么会崩？
- **下一站**：state-machine（待产）——状态机内部大量用信号槽做转移触发，观察者是其基础。
