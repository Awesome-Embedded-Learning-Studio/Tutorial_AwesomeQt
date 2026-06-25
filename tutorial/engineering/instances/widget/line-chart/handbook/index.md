---
title: "LineChart 手搓手册"
description: "从空 main 一行行搓出 LineChart：3 步打通纯 QPainter 折线图骨架、Y 轴 auto-scale、网格/数据点/线下填充与边界防护。"
---

# LineChart 手搓手册

> **source**：成品答案在 `widget/line-chart/`（做完对照）· **related**：自绘控件递进链（status-led · toggle-switch · **line-chart**）· 教程层 [自定义控件绘制入门](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)、[QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 LineChart，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **纯 QPainter 自绘图表**：继承 QWidget + 重写 `paintEvent`，用 `drawLine` / `drawPolyline` / `drawEllipse` / `QPainterPath` 画出网格、坐标轴、折线、点、填充
- **坐标变换**：把「数据值」映射成「像素坐标」的 `value → py` 闭包，理解 plot 区、margin、auto-scale 三者关系
- **Q_PROPERTY 外观开关**：六个属性（lineColor / axisColor / gridColor / showGrid / showDots / showArea）配 READ / WRITE / NOTIFY，可被 Designer / 外部直接驱动
- **边界全防护**：空数据 early return、单点居中、`min==max` 防 `±padding` 防除零——这是图表控件和普通自绘控件最不一样的地方
- **QFontMetrics 做文本对齐**：量刻度数字宽度做右对齐，避免标签溢进折线区

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

分 3 步，每步对应控件的一个真实实现阶段。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | plot 区 + 坐标轴 + 数据 API（先画出一个空框架，能 setData 触发重绘） | [01](./01-axes-and-data.md) |
| 2 | Y 轴 auto-scale + 折线（数据值映射成像素，画出真正的折线 + 边界防护） | [02](./02-autoscale-and-line.md) |
| 3 | 网格 / Y 刻度数字 / 数据点 / 线下填充（外观开关 + QFontMetrics 对齐） | [03](./03-grid-dots-area.md) |

成品对照：`widget/line-chart/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **逐点生长动画**：给折线加一个「点数从 0 长到 n」的 `QPropertyAnimation`，让折线像心电图一样画出来。提示：暴露一个 `int visibleCount` 属性，paintEvent 只画前 `visibleCount` 个点。
- **双序列对比**：加第二条折线，思考两条线共用一个 Y 域还是各自 auto-scale——这会牵出 `values_` 从单 `QVector` 变成「序列集合」的结构改动。
- **鼠标 hover 显示数值**：重写 `mouseMoveEvent`，反查最近的数据点、画个 tooltip。提示：要存 poly 的像素坐标或在 hover 时重算。
- **下一站**：圆环进度（circle-progress）或仪表盘（speed-meter）——换皮复用 QPainter 自绘骨架，但引入角度绘制和指针动画。
