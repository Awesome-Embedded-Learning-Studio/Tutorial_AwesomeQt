---
title: "Step 2：自绘 CPU 历史曲线"
description: "CpuHistoryView 自绘 QWidget：滚动窗口（vector 满则丢最旧）、QPainter 画折线（QPainterPath 无效点断开留缺口）+ 半透明面积填充（关路径到底边）+ 25/50/75/100% 参考网格 + 深色底对比。"
---

# Step 2：自绘 CPU 历史曲线

← 上一步 [Step 1 系统读取+采样](./01-system-read-and-sampling.md) · [手册首页](./index.md) · 下一步 [Step 3 主窗口+定时器](./03-window-and-timer.md) →

## Step 2：CpuHistoryView 自绘滚动曲线

### 目标

写一个 `cpu_history_view.h/.cpp`，做一个 `CpuHistoryView : public QWidget` 自绘控件：构造时给容量（默认 60 点），`push(int percent)` 推一个 0..100 采样点（-1 表无效），`paintEvent` 把已有样本画成折线 + 半透明面积填充 + 25/50/75/100% 参考网格。满容量后丢最旧的（滑动窗口）。这一步**控件能独立测试**——写个临时窗口定时 `push(random)` 看曲线滚动。

### 提示

- **数据怎么存？** `std::vector<int> samples_` + `int capacity_`。`push` 时先判断 `size() >= capacity_` 就 `erase(begin())` 丢最旧，再 `push_back`，最后 `update()` 触发重绘。构造里给 `capacity_` 一个下限（`> 4 ? capacity : 4`），免得传 0 崩。
- **深色底怎么设？** `setAutoFillBackground(true)` + 改 `QPalette::Window` 为深色（如 `QColor(24,24,28)`），亮色折线对比强。`setMinimumSize(220,90)`。
- **`paintEvent` 先画什么？** ① 算画布 `r = rect().adjusted(0,0,-1,-1)`（-1 留边框像素）② 画 25/50/75/100% 水平参考线（点状浅灰，`Qt::DotLine`），y = `r.top + h - h*pct/100` ③ 按「连续有效段」画折线 + 填充 ④ 画边框。开 `Antialiasing` 让线平滑。
- **折线怎么画、无效点怎么断开？** 关键是**按「连续有效段」分组**：遍历样本，先跳过无效点（-1 / 越界）找到段首，再连续收集有效点到段尾，得到 `[seg_start, seg_end)`。每段单独构造一个 `QPainterPath`：段首 `moveTo`、段内后续点 `lineTo`。x = `r.left + i * dx`（`dx = w / (size-1)`，**按已有样本数等分铺满宽度**，窗口未满也铺满直观）；y = `r.top + h - h*v/100`。**千万别**把折线连续画过 -1 点（画到 y=0 会让人以为 CPU 真的空载），缺口要断开。
- **面积填充怎么画、别越过缺口？** 每段折线画完后，**复制该段的折线** `QPainterPath fill = seg`，再 `lineTo(段尾x, r.bottom)` + `lineTo(段首x, r.bottom)` + `closeSubpath()` 关成多边形，`fillPath(fill, 半透明色)`（如 `QColor(80,180,240,70)`）增强「面积感」。**坑**：若用整条路径闭合填充，缺口下方会照样涂色——折线说「这里没数据」、填充却涂上了面积，语义矛盾。要每段独立闭合，缺口下不画填充。
- **样本数 < 2 / 段内点 < 2 怎么办？** 折线至少要两个点；整个样本 < 2 不进绘制，某段有效点 < 2（单点）`continue` 跳过该段——单点不画也合理，要知道。
- **`sizeHint` 怎么给？** `{capacity_ * 4 + 4, 120}`——给布局一个合理建议尺寸，别让控件被压扁。

### 检查点

写个临时窗口，`QTimer` 每 500ms `push` 一个 `QRandomGenerator::bounded(0,100)`：窗口里出现一条深色底、亮色折线 + 半透明面积 + 参考网格的曲线，随采样滚动（最左旧、最右新）。**push(-1) 测试缺口**：曲线在缺口处断开、且缺口下方**没有**填充面积（折线与填充同段同断）。曲线能滚动 + 缺口处折线断开、填充也断开 = 控件通了。

> QPainter 自绘不熟？先读 [QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)。
> 自定义控件 paintEvent 不熟？先读 [自绘控件基础](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)。
> QPainterPath 不熟？看 [QPainterPath 文档](https://doc.qt.io/qt-6/qpainterpath.html)。

### 对照答案

- `CpuHistoryView` 接口（samples_/capacity_ + push/paintEvent + sizeHint）：`demo/cpu_history_view.h:15-31`
- 构造（capacity 下限 + 深色底 + minimumSize）：`demo/cpu_history_view.cpp:14-22`
- `push`（满则丢最旧 + update）：`demo/cpu_history_view.cpp:24-30`
- `sizeHint`：`demo/cpu_history_view.cpp:32-34`
- `paintEvent` 参考网格（25/50/75/100% DotLine）：`demo/cpu_history_view.cpp:45-50`
- `paintEvent` 折线分组（跳过无效点 → 收集连续有效段 → 每段单独 QPainterPath 折线）：`demo/cpu_history_view.cpp:60-83`
- `paintEvent` 面积填充（每段折线复制一份、下到段首闭合、半透明 fillPath，不越过缺口）：`demo/cpu_history_view.cpp:84-89`
- `paintEvent` 折线描边 + 边框：`demo/cpu_history_view.cpp:90-99`

---

下一步：主窗口装配（QProgressBar + QGroupBox 分组）+ QTimer 1s 节拍 + 无效态降级——[Step 3 主窗口+定时器](./03-window-and-timer.md)。
