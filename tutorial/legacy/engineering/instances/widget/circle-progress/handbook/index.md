---
title: "CircleProgress 手搓手册"
description: "从空 main 一行行搓出 CircleProgress：6 步打通 drawArc 弧绘制、value/progress 属性解耦、QPropertyAnimation 平滑过渡、几何 clamp 兜底。"
---

# CircleProgress 手搓手册

> **source**：成品答案在 `widget/circle-progress/`（做完对照）· **related**：自绘控件递进链第 3 环

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 CircleProgress，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **drawArc 画弧**：1/16° 角度体系、起始角 / 扫角方向、RoundCap 粗弧
- **业务属性与动画属性解耦**：value（语义）和 progress（绘制）为什么必须分两个
- **QPropertyAnimation 驱动 double**：进度弧从当前值接力平滑铺开
- **Q_PROPERTY 全套 + 几何 clamp 兜底**：自绘控件的标准收尾

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    w.resize(120, 120);
    w.show();
    return app.exec();
}
```

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

## 2. 任务清单

分 6 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 背景整圈环（drawArc 画一整圈） | [01](./01-ring-and-progress.md) |
| 2 | 进度弧从 12 点钟顺时针铺开 + 中心文字（先突变） | [01](./01-ring-and-progress.md) |
| 3 | value / progress 拆成两个属性 | [02](./02-smooth-transition.md) |
| 4 | QPropertyAnimation 让弧平滑铺开 | [02](./02-smooth-transition.md) |
| 5 | 配色 / 线宽 / 文字开关 Q_PROPERTY | [03](./03-polish-and-properties.md) |
| 6 | 几何 clamp 兜底（压极小时不崩） | [03](./03-polish-and-properties.md) |

成品对照：`widget/circle-progress/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **进度弧渐变色**：用 `QConicalGradient` 当画笔刷子，让弧从蓝过渡到绿。提示：QPen 可以 setBrush 一个 gradient。
- **不确定态（加载中转圈）**：加个 `Indeterminate` 模式，弧固定长度、整体旋转（再动画一个起始角属性）。思考：这会跟 progress 动画打架吗？两者写不同变量就正交。
- **下一站**：speed-meter——把「动画驱动 0..1 进度」换成「动画驱动角度」，再加指针 + 刻度，复用同一套解耦骨架。
