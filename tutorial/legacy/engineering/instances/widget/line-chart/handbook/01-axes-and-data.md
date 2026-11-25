---
title: "Step 1：plot 区 + 坐标轴 + 数据 API"
description: "定 margin 算 plot 矩形，画左 Y 轴和下 X 轴，加 setData/appendPoint/clear 三个数据入口触发 update——折线图的空骨架。"
---

# Step 1：plot 区 + 坐标轴 + 数据 API

← [手册首页](./index.md) · 下一步 [Step 2 auto-scale + 折线](./02-autoscale-and-line.md) →

## Step 1：画出折线图的空骨架

### 目标

屏幕上出现一个**左 Y 轴 + 下 X 轴**围出来的 L 形坐标系（还不用画折线），并且能 `setData` 喂数据、控件收到数据就请求重绘。这一步先把「坐标变换的舞台」和「数据入口」搭起来——折线和 auto-scale 留到 step 2。

### 提示

- 继承 `QWidget`，重写 `protected void paintEvent(QPaintEvent*)`，开头 `QPainter p(this); p.setRenderHint(QPainter::Antialiasing);`
- 定义四个 margin 常量：左留大（48，给 Y 刻度数字）、下留中（28，给 X 标签）、上下右留小（12）。参考 `src/line_chart.cpp:19-22`
- 算 plot 区：`plot_left = kMarginLeft`、`plot_top = kMarginTop`、`plot_width = w - 左 - 右`、`plot_height = h - 上 - 下`、`plot_bottom = plot_top + plot_height`。宽高记得 `std::max(1.0, ...)` clamp（`src/line_chart.cpp:164-165`），否则窗口压极小时算出负值
- 画坐标轴：左 Y 轴是一条从 `(plot_left, plot_top)` 到 `(plot_left, plot_bottom)` 的竖线，下 X 轴是从 `(plot_left, plot_bottom)` 到 `(plot_left+plot_width, plot_bottom)` 的横线（`src/line_chart.cpp:208-209`），用 `QPen(axisColor, 1)`
- 加数据成员 `QVector<qreal> values_` 和三个入口：`appendPoint`（`append` + `update`）、`setData`（赋值 + `update`）、`clear`（空就 return、否则 `clear` + `update`）。注意 `clear` 要先判空再 return，省一次无谓重绘（`src/line_chart.cpp:48-54`）

### 检查点

- 跑出来是 L 形坐标系（左竖线 + 下横线），窗口缩放 L 形跟着变、不会出负值 = plot 区算对了
- `setData({1,2,3})` 后控件没崩（这步还没画折线，所以看不到线很正常），但你能通过调试或 step 2 验证数据进来了
- `clear()` 后再画不崩

> QPainter 不熟？先读 [QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md) 和 [自定义绘制 Widget 基础](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)。

### 对照答案

- 四个 margin 常量：`src/line_chart.cpp:19-22`
- plot 区 + clamp：`src/line_chart.cpp:162-166`
- 坐标轴两条线：`src/line_chart.cpp:207-209`
- 数据三个入口 + clear 判空：`src/line_chart.cpp:38-54`

---

下一步是重头戏：[Step 2 加 Y 轴 auto-scale 和真正的折线](./02-autoscale-and-line.md)。
