---
title: "Step 3：网格 / Y 刻度数字 / 数据点 / 线下填充"
description: "showGrid 画横竖网格线，QFontMetrics 量宽右对齐 Y 刻度数字，showDots 画数据点小圆，showArea 用 QPainterPath 闭合到 plot bottom 半透明填充。"
---

# Step 3：网格 / Y 刻度数字 / 数据点 / 线下填充

← [Step 2](./02-autoscale-and-line.md) · [手册首页](./index.md) →

折线已经画出来了，这一步给它加四个外观增强：背景网格、Y 轴刻度数字、数据点小圆、线下半透明填充。每个都用一个 `show*` 布尔属性控制，配 Q_PROPERTY 让 demo 能用勾选框实时切。

## Step 3：网格 + 刻度数字 + 数据点 + 线下填充

### 目标

折线上叠：①几条横向 + 竖向淡灰网格线（`showGrid`）；②Y 轴左侧一列刻度数字（按 Y 域等分，最大值在上、最小值在下）；③每个数据点一个小圆（`showDots`）；④折线下方半透明同色填充（`showArea`）。四个开关都能在 demo 里实时勾选切换。

### 提示

**网格（`showGrid`，`src/line_chart.cpp:191-204`）**：

- Y 方向 `kGridTickCount`（4）档横线，第 i 条的 y = `plot_top + plot_height * i / kGridTickCount`
- X 方向竖线只在 `n > 1` 时画，第 i 条的 x = `plot_left + plot_width * i / (n-1)`——和折线的 X 公式一致

**Y 刻度数字（`src/line_chart.cpp:211-222`）**：

- 用 `QFontMetrics fm(p.font())`，第 i 档对应的值 v = `y_max - (y_max - y_min) * i / kGridTickCount`（i=0 在顶部对 y_max）
- `QString::number(v, 'f', 1)` 保留一位小数
- `int text_w = fm.horizontalAdvance(text)` 量实际宽，`drawText(QPointF(plot_left - text_w - 4, y + fm.ascent()/2), text)` 右对齐到 Y 轴左、垂直按 baseline 居中。**别用固定起点 drawText**，否则数字位数一变就溢进折线区

**数据点（`showDots`，`src/line_chart.cpp:258-265`）**：

- `p.setPen(Qt::NoPen); p.setBrush(lineColor);`，遍历 poly 每个点 `p.drawEllipse(pt, r, r)`，`r = 3`

**线下填充（`showArea`，`src/line_chart.cpp:239-250`）**：

- **只在 `n >= 2` 时画**（单点无面积可言）
- 用 `QPainterPath area`：`moveTo(poly.first().x(), plot_bottom)` → `addPolygon(poly)` → `lineTo(poly.last().x(), plot_bottom)` → `closeSubpath`
- fill 色 = `lineColor` 复制后 `setAlpha(60)` 半透明，和折线同色系；`p.setPen(Qt::NoPen); p.setBrush(fill); p.drawPath(area);`

### 关键认知

- **四个开关都走 Q_PROPERTY**：`showGrid` / `showDots` / `showArea` 三个 bool + `lineColor` / `axisColor` / `gridColor` 三个 QColor，setter 全是「值未变 return → 赋值 → emit → update」（`src/line_chart.cpp:63-70` 等）。没有动画，所以 setter 不夹业务，也不会有 status-led 那种「WRITE 指错 → 递归栈溢出」的风险
- **填充用 QPainterPath 而非 QPolygonF**：因为要 `moveTo(首点 x, plot_bottom)` 把起点接到 X 轴、`lineTo(末点 x, plot_bottom)` 把终点接到 X 轴，组成「折线 + 两条垂直边 + X 轴段」的闭合区域。`addPolygon` 加这两条 line 正好把首尾接到底
- **Y 刻度数字的对齐是视觉关键**：QFontMetrics 量宽这步省不得，左对齐的标签和折线糊一团是这个控件最难看的 bug

### 检查点

- 勾 `showGrid`：横竖淡灰线出现在背景，关掉消失 = 网格对了
- Y 轴左侧一列数字（最大值在上、最小值在下），数字始终在 Y 轴**左**侧、不溢进折线区 = QFontMetrics 对齐对了
- 勾 `showDots`：每个数据点出现小圆，关掉只剩线 = 数据点对了
- 勾 `showArea`：折线下方出现半透明同色填充，关掉消失 = 填充对了
- 换 `lineColor`：折线、点、填充颜色一起变（填充是同色半透明）= 同色系填充对了

> Q_PROPERTY / QPainterPath 不熟？[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)、进阶 [属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。QFontMetrics？[字体与文本渲染](../../../../../beginner/02-qtgui/04-font-text-beginner.md)。

### 对照答案

- 网格横竖线：`src/line_chart.cpp:191-204`
- Y 刻度数字 + QFontMetrics 右对齐：`src/line_chart.cpp:211-222`
- 数据点小圆：`src/line_chart.cpp:258-265`
- 线下填充 QPainterPath 闭合 + setAlpha(60)：`src/line_chart.cpp:239-250`
- showGrid/showDots/showArea setter（纯赋值+emit+update）：`src/line_chart.cpp:102-135`

---

搓完了。跑 demo 对照成品：三栏（静态 + 交互追加随机点 + 外观开关）都能复现 = 你搓的和 repo 一致。

想再深？回 [手册首页](./index.md) 看进阶挑战（逐点生长动画 / 双序列对比 / hover tooltip / 下一站圆环进度）。
