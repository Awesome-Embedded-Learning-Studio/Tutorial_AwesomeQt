---
title: "Step 2：Y 轴 auto-scale + 折线（核心）"
description: "用 std::minmax_element 算 Y 域，value→像素 to_y 闭包，QPolygonF + drawPolyline 画折线，单点居中 + min==max 防 ±padding 防除零。"
---

# Step 2：Y 轴 auto-scale + 折线（核心）

← [Step 1](./01-axes-and-data.md) · [手册首页](./index.md) · 下一步 [Step 3 网格/点/填充](./03-grid-dots-area.md) →

这一步是整个控件的核心——把 step 1 的「空坐标系」升级成「真正画出折线」。诀窍不在画线本身，而在**每次 paintEvent 现算 Y 域、用一个 `to_y` 闭包把数据值映射成像素 y**。

## Step 2：Y 轴 auto-scale + 画折线

### 目标

`setData({12, 28, 19, 45, 33})` 后，控件画出一条折线，Y 轴自动按数据的最小最大值缩放（最小值贴底、最大值贴顶），X 轴按索引均匀分布。并且：单点居中、`min==max` 不崩、空数据不崩。

### 提示（按顺序）

1. **Y 轴 auto-scale**：`values_` 非空时，用 `std::minmax_element(values_.constBegin(), values_.constEnd())` 一次扫描同时拿 min/max（`src/line_chart.cpp:174`），比两次串行 `std::min` / `std::max` 干净
2. **防除零**：拿到 min/max 后判 `qFuzzyCompare(y_min, y_max)`，成立（常量序列或单点）就 `y_min -= kFlatPadding; y_max += kFlatPadding`（`kFlatPadding = 1.0`，`src/line_chart.cpp:177-181`）。否则后面 `to_y` 分母 `y_range` 为零，点画飞
3. **写 `to_y` 闭包**：`return plot_bottom - (v - y_min) / y_range * plot_height;`（`src/line_chart.cpp:186-188`）。注意是 `plot_bottom - ...`——Y 轴像素坐标往下递增，而数据值往上递增，所以要翻转
4. **空数据 early return**：算完 Y 域、画完坐标轴后，若 `!has_data` 直接 return（`src/line_chart.cpp:225-227`），别去建空 QPolygonF
5. **建 QPolygonF**：`n==1` 时点居中（`plot_left + plot_width/2`），`n>=2` 时 `x = plot_left + plot_width * i / (n-1)`（`src/line_chart.cpp:233-234`）。**单点单独写分支**，否则 `n-1` 为零 NaN
6. **画线**：`p.setPen(QPen(lineColor, 2)); p.setBrush(Qt::NoBrush); p.drawPolyline(poly);`（`src/line_chart.cpp:253-255`）

### 关键认知

- **auto-scale 每次 paintEvent 现算**，不缓存 min/max。数据一变 `update()` 一调，下一帧 Y 轴自动跟着值域走——这是 demo「追加随机点看 Y 轴漂移」的机制
- **Y 轴像素要翻转**：`(v - y_min) / y_range` 算出的是「从下往上 0..1 的比例」，但 Qt 坐标系 y 往下递增，所以 `plot_bottom - 比例*plot_height` 才对
- **三种退化情况都要防**：空数据（return）、单点（居中）、`min==max`（±padding）。任一个漏掉都会出 NaN 或崩溃，这是图表控件最容易踩的坑

### 检查点

- `setData({12, 28, 19, 45, 33, 52, 40, 61, 55, 48})` 画出一条起伏折线，最小值贴底、最大值贴顶 = auto-scale 对了
- `setData({5})` 单点居中显示、不飞、不崩 = 单点分支对了
- `setData({5,5,5})` 常量序列画成一条横线（不是飞到屏外）、不崩 = ±padding 对了
- `clear()` 后只剩坐标轴、不崩 = 空数据 return 对了

> QPainterPath / 坐标变换不熟？[坐标变换](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)。容器遍历用 STL 算法？[容器](../../../../../beginner/01-qtbase/04-container-beginner.md)。

### 对照答案

- `std::minmax_element` 拿 Y 域：`src/line_chart.cpp:174-176`
- `qFuzzyCompare` + ±padding 防除零：`src/line_chart.cpp:177-181`
- `to_y` 闭包（含 Y 翻转）：`src/line_chart.cpp:186-188`
- 空数据 early return：`src/line_chart.cpp:225-227`
- 单点居中 vs 均匀分布分支：`src/line_chart.cpp:233-234`
- drawPolyline 画折线：`src/line_chart.cpp:253-255`

---

下一步：[Step 3 加网格、Y 刻度数字、数据点、线下填充](./03-grid-dots-area.md)。
