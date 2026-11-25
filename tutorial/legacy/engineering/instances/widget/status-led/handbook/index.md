---
title: "StatusLED 手搓手册"
description: "从空 main 一行行搓出 StatusLED：6 步打通自绘控件骨架、Q_PROPERTY、QPropertyAnimation 颜色过渡、呼吸动画。"
---

# StatusLED 手搓手册

> **source**：成品答案在 `widget/status-led/`（做完对照）· **related**：自绘控件递进链第 1 环

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 StatusLED，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **自定义控件骨架**：继承 QWidget + 重写 `paintEvent` + 实现 `sizeHint` / `minimumSizeHint`
- **Q_PROPERTY 全套**：READ / WRITE / NOTIFY 三件 + `Q_ENUM`，让属性被 Qt 元系统、动画、Designer 识别
- **QPropertyAnimation 驱动 QColor**：颜色平滑过渡（核心重头，step 4）
- **QVariantAnimation 做无限循环动画**：呼吸效果（step 5）
- **多动画对象在 paintEvent 解耦合成**：过渡色与呼吸因子正交并行

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    w.resize(100, 100);
    w.show();
    return app.exec();
}
```

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

## 2. 任务清单

分 6 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 画一个带高光的静态圆盘 | [01](./01-paint-and-status.md) |
| 2 | Status 枚举 + 切色（先突变） | [01](./01-paint-and-status.md) |
| 3 | status 升级为 Q_PROPERTY | [02](./02-color-transition.md) |
| 4 | 颜色丝滑过渡（QPropertyAnimation） | [02](./02-color-transition.md) |
| 5 | BlinkMode：OnOff + Breathing 呼吸 | [03](./03-blink-and-polish.md) |
| 6 | minimumSizeHint 收尾 | [03](./03-blink-and-polish.md) |

成品对照：`widget/status-led/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **消除绿→红过渡的暗褐中间色**：用 `qRegisterAnimationInterpolator<QColor>` 注册 HSV 插值器，让颜色走色相环而非 RGB 直线。提示：色相接近时 HSV 过渡更鲜艳。
- **状态色可配置**：加 `setStatusColor(Status, QColor)` 让不同应用自定义配色。思考：这会跟未来的 theme-system 重叠吗？边界在哪？
- **下一站**：toggle-switch（待产）——换皮复用同一套 Q_PROPERTY + 动画骨架，但引入状态机 + 滑动动画。
