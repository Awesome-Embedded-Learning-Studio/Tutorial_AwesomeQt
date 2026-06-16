---
title: 01 画轨道和滑块
description: ToggleSwitch 手搓第 1 步——paintEvent 画圆角轨道 + 圆形滑块
---

# 01 画轨道和滑块

> 对照成品：`widget/toggle-switch/src/toggle_switch.cpp` 的 `paintEvent`（106 行起）

## 目标

画出滑动开关的静态样子：一个圆角「药丸」轨道（pill 形），里面一个白色圆形滑块靠左。

## 提示

- `paintEvent` 里 `QPainter p(this)`，开抗锯齿。
- 轨道用 `drawRoundedRect`，圆角半径取「轨道高度的一半」就是完美药丸形。
- 滑块用 `drawEllipse`，直径取「轨道高 - 2×留白」（留白 3px 左右），圆心 y 取轨道中线，圆心 x 先固定在左侧。
- 加个私有成员 `qreal handle_pos_{0.0}`（0=左 1=右），暂时写死 0 看靠左效果。颜色随便给个灰。
- 不给整段代码，自己拼。

## 检查点

- 窗口里一个灰色药丸，左边贴一个白圆，比例协调、四角圆润、无锯齿。
- 拉伸窗口（只要别太窄），轨道药丸形不变形。

对了进 [02 点击切换与滑动动画](./02-click-toggle.md)。

## 对照答案

- 轨道绘制在 `toggle_switch.cpp:117`（`drawRoundedRect`，radius = 高/2）。
- 滑块定位在 `toggle_switch.cpp:120-125`（直径、半径、圆心 x 随 `handle_pos_`）。
