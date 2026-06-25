---
title: "Step 1-2：背景环 + 进度弧 + 中心文字"
description: "用 drawArc 画整圈背景环，再画从 12 点钟顺时针铺开的进度弧，中间绘百分比文字。先突变，过渡下一步。"
---

# Step 1-2：背景环 + 进度弧 + 中心文字

← [手册首页](./index.md) · 下一步 [Step 3-4 平滑过渡](./02-smooth-transition.md) →

## Step 1：画一个整圈背景环

### 目标

屏幕上出现一个**浅灰色的圆环**（不是实心圆，是一圈粗线），居中，有粗细。

### 提示

- 继承 `QWidget`，重写 `protected void paintEvent(QPaintEvent*)`
- `QPainter p(this); p.setRenderHint(QPainter::Antialiasing);` 抗锯齿
- 算环半径：`r = min(width,height)/2 - 线宽/2 - 2`（留点边距，别贴边）
- 画笔设粗（`pen.setWidthF(10)`）、圆头（`pen.setCapStyle(Qt::RoundCap)`），画笔颜色就是环色
- `p.setBrush(Qt::NoBrush)`——只描线不填充
- 一整圈用 `p.drawArc(rect, 0, 360*16)`（整圈 5760，1/16°）

### 检查点

跑出来是个**居中的灰色粗圆环**，缩放窗口环跟着自适应 = drawArc 画整圈对了。

> drawArc 不熟？先读 [QPainter 绘图基础](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)。

### 对照答案

- 半径算法 + clamp：`src/circle_progress.cpp:165`
- 背景环 drawArc 整圈：`src/circle_progress.cpp:179`

---

## Step 2：进度弧 + 中心百分比文字（先突变）

### 目标

在背景环上叠一段**彩色进度弧**，从 12 点钟顺时针铺开，铺的比例 = value/100。圆环正中间绘百分比文字（如 "60%"）。**这步先做突变**——value 直接映射成弧，不要过渡，过渡留到 step 4。

### 提示

- drawArc 角度体系反直觉：**0°=3 点钟、正值逆时针、单位 1/16°**。12 点钟 = 90° = `90*16`
- 顺时针铺开 = 扫角取**负**。所以进度弧是 `drawArc(rect, 90*16, -span)`，`span = int(progress * 360*16)`
- progress 暂时直接 `= value/100.0`（step 3 才拆成独立属性），先让它能跑
- 中心文字：`QString::number(int(progress*100)) + "%"`，用 `QFontMetrics` 算宽高，在 `rect.center()` 居中 `drawText`
- value=0 时 progress=0，span=0，弧不画——加个 `if (progress > 0)` 判断，避免画一段 0 长度的弧

### 检查点

给 value 设 25/50/75/100，弧分别铺 1/4、半圈、3/4、整圈，**从 12 点钟往 3 点钟方向顺时针**，中间文字跟着变 = 角度换算对了。如果弧逆时针铺或从 3 点钟起，回去查扫角正负和起始角（翻 [卡住怎么办](./troubleshooting.md)）。

### 对照答案

- 角度常量 + 约定注释：`src/circle_progress.cpp:22-25`
- 进度弧 drawArc（起始 1440、扫角负）：`src/circle_progress.cpp:191`
- 中心文字 drawText：`src/circle_progress.cpp:206`

---

下一步是重头戏：[Step 3-4 把突变弧升级成平滑过渡](./02-smooth-transition.md)。
