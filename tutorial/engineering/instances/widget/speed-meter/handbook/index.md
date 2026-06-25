---
title: "SpeedMeter 手搓手册"
description: "从空 main 一行行搓出 SpeedMeter：3 步打通自绘表盘骨架、value/needleAngle 解耦动画、双角度体系自洽。"
---

# SpeedMeter 手搓手册

> **source**：成品答案在 `widget/speed-meter/`（做完对照）· **related**：自绘控件递进链（[status-led](../../status-led/) · [toggle-switch](../../toggle-switch/) 已产）· 教程：[自定义控件绘制入门](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)、[动画框架入门](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)

这份手册带你从零搓一个 `AwesomeQt::SpeedMeter`——能 setValue 让指针平滑旋转、带主次刻度和数字读数的速度表盘。前两个控件（status-led / toggle-switch）已经把「Q_PROPERTY + 动画驱动」这套骨架趟平了，SpeedMeter 在它上面重点练一件新东西：**在一个 paintEvent 里把两套角度体系（三角函数数学角度 vs QPainter drawArc 的 1/16° 角度）各自安顿好、又对得上**。

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 SpeedMeter，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **自绘表盘的几何**：用三角函数（`cos`/`sin`）把角度换算成弧上点的坐标，画背景弧、主次刻度、指针
- **value / needleAngle 解耦**：业务属性（速度值）和动画属性（绘制角度）分开，setValue 启动动画、动画驱动 setNeedleAngle——这套和 status-led 的 color / setAnimatedColor 完全同构
- **两套角度体系各自安顿**：三角函数用数学角度（0°=3 点、逆时针为正），`drawArc` 用 1/16° 且正值逆时针；同一个 paintEvent 里不混着用，换算关系注释写死
- **QPainter 坐标变换画指针**：`save / translate(center) / rotate / restore` + `QPolygonF` 画根粗尖细多边形，比手算两端 drawLine 省事（注意 rotate 基准是 12 点钟、要 +90° 修正）
- **持久动画对象 + 接力**：stop() / setStartValue(当前显示角度) / start() 复用同一对象，Slider 连拖 / 狂点都不跳变

## 1. 起点

先有个能跑的空壳。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    w.resize(200, 200);
    w.show();
    return app.exec();
}
```

弹出空白窗 = 环境通了，往下走。Qt 环境不熟先看 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。这里要 `find_package(Qt6 COMPONENTS Widgets)`，链接 `Qt6::Widgets`（QPropertyAnimation 属于 Core，但用到了一起链就行）。

## 2. 任务清单

分 3 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 画静态表盘：背景弧 + 主/次刻度（先把角度体系自洽跑通） | [01](./01-static-gauge.md) |
| 2 | value→角度映射 + 指针 + 数字读数（静态指到当前值） | [02](./02-needle-and-mapping.md) |
| 3 | value/needleAngle 解耦 + QPropertyAnimation 接力（指针平滑旋转） | [03](./03-smooth-animation.md) |

成品对照：`widget/speed-meter/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **指针换颜色档位**：value 越接近 maxValue 指针越红（绿→黄→红渐变）。提示：paintEvent 里按 `value / maxValue` 在多边形颜色上插值，比单纯红色更有「危险区」视觉提示。思考：这是不是该做成 `Q_PROPERTY` 让外部配色，还是控件内部自己定？
- **刻度分段配色**：高转速区（如 180 以上）刻度/弧段改色。提示：分段 `drawArc` 画不同颜色的弧段，得想清楚每段的角度边界怎么算（别又和数学角度打架）。
- **加一个 setValue 的「过冲回弹」**：指针到目标后微微回弹一下再稳住（车表的真实手感）。提示：`QEasingCurve::OutBack` 或自定义曲线，但要确认回弹不会让 angleForValue 之外的角度画出怪样子。
- **下一站**：circle-progress（待产）——同样是弧 + 角度体系，但用来画进度条而非指针，可以复用这套角度换算注释。
