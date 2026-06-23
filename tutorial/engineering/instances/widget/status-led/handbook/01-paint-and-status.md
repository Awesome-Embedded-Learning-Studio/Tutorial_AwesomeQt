---
title: "Step 1-2：画静态圆盘 + Status 切色"
description: "重写 paintEvent 用 QRadialGradient 画高光圆盘，加 Status 枚举实现状态切色（先突变，过渡下一步）。"
---

# Step 1-2：画静态圆盘 + Status 切色

← [手册首页](./index.md) · 下一步 [Step 3-4 颜色过渡](./02-color-transition.md) →

## Step 1：画一个带高光的静态圆盘

### 目标

屏幕上出现一个**绿色圆点**，带径向渐变高光（中心亮、边缘暗），有立体感，不是纯色平圆。

### 提示

- 继承 `QWidget`，重写 `protected void paintEvent(QPaintEvent*)`
- `QPainter painter(this);` 开头，`painter.setRenderHint(QPainter::Antialiasing)` 抗锯齿
- 填充用 `QRadialGradient`，三档色：中心 `c.lighter(160)`、`0.6` 处原色 `c`、边缘 `c.darker(150)`
- 半径 `std::min(width(), height()) / 2 - 1`（别忘了外面包 `std::max(1, ...)`，否则窗口极小时半径为负）
- `drawEllipse(rect().center(), r, r)` 画圆

### 检查点

跑出来是**有高光、边缘渐暗**的绿圆，缩放窗口圆点自适应 = 绘制对了。

> QPainter 不熟？先读 [QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md) 和 [自定义绘制 Widget 基础](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)。

### 对照答案

- 径向渐变三档色：`src/status_led.cpp:225-228`
- 半径 clamp 兜底：`src/status_led.cpp:223`

---

## Step 2：Status 枚举 + 切色（先突变）

### 目标

定义 `enum class Status { NORMAL, WARNING, ERROR, OFFLINE }`，加 `setStatus(Status)`，调一下颜色就变。**这步先做突变**（直接换色，不要动画），过渡留到 step 4。

### 提示

- 枚举放在类里，紧跟 `Q_ENUM(Status)`——为后面 Q_PROPERTY 铺路（moc 要认得这个枚举）
- 写私有 `statusColor(Status) const` 返回各状态代表色（绿 / 琥珀 / 红 / 灰）
- `setStatus` 里改成员 `status_`、`update()`（异步请求重绘）
- `paintEvent` 里用 `statusColor(status_)` 取当前色画

### 检查点

`setStatus(WARNING)` 后圆点变琥珀，`setStatus(ERROR)` 变红 = 切换对了（先不管有没有过渡动画）。

> 信号槽 / update 刷新机制不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

### 对照答案

- statusColor 四态代表色：`src/status_led.cpp:24`
- Q_ENUM 声明：`include/status_led.h:35-36`

---

下一步是重头戏：[Step 3-4 把突变色升级成丝滑过渡](./02-color-transition.md)。
