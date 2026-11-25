---
title: "FadeWidget 手搓手册"
description: "从空 main 一行行搓出 FadeWidget：5 步打通 QGraphicsOpacityEffect 承载透明度、QPropertyAnimation 驱动 opacity、持久指针防连发悬空、QSignalBlocker 防反向同步成环。"
---

# FadeWidget 手搓手册

> **source**：成品答案在 `widget/fade-animation/`（做完对照）· **related**：动画/特效控件递进链（上一站 [status-led](../../status-led/) · 下一站 toggle-switch/circle-progress 待产）

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 FadeWidget，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **QGraphicsOpacityEffect**：把透明度这件事彻底外包给特效，控件本身一行绘制都不用写
- **QPropertyAnimation 驱动 effect 属性**：`new QPropertyAnimation(effect_, "opacity", this)` 让动画直接写 effect 的 `opacity`
- **Q_PROPERTY 的 WRITE 设计**：`setOpacity` 做纯赋值、不调动画，和 status-led 的 `setAnimatedColor` 同一套哲学——动画入口和属性 WRITE 分离
- **持久动画指针复用**：`stop()` + 重配 + `start()`，禁 `DeleteWhenStopped`，防连发悬空
- **QSignalBlocker 防反向同步成环**：动画回灌滑块时屏蔽信号，避免 valueChanged→setOpacity→opacityChanged 死循环

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

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

## 2. 任务清单

分 5 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 挂 QGraphicsOpacityEffect，setOpacity 能改透明度 | [01](./01-effect-and-opacity.md) |
| 2 | opacity 升级为 Q_PROPERTY | [01](./01-effect-and-opacity.md) |
| 3 | QPropertyAnimation 驱动 opacity 做淡入淡出 | [02](./02-fade-animation.md) |
| 4 | fadeIn/fadeOut 入口 + show/hide 时机 | [02](./02-fade-animation.md) |
| 5 | demo：时长滑块 + 瞬时 opacity 滑块（QSignalBlocker 防环） | [03](./03-demo-and-polish.md) |

成品对照：`widget/fade-animation/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **fadeOut 结束可选不 hide**：现在默认淡出后 `hide()`。加一个属性（比如 `hideOnFadeOut`）让调用方决定要不要保持可见但透明——思考：这对后续「淡出后销毁」语义会有什么影响？
- **组合 QPropertyAnimation 同时改 geometry + opacity**：做一个「边缩小边淡出」的退出动画。提示：两个动画并行，注意都用 `parent=this` 托管。
- **下一站**：toggle-switch（待产）——换皮复用这套「effect+动画+Q_PROPERTY」骨架，但引入状态机 + 滑动动画。
