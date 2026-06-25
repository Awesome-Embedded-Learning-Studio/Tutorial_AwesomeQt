---
title: 01 画轨道和双手柄
description: RangeSlider 手搓第 1 步——paintEvent 画圆角轨道 + 选中区间 + 两个圆手柄，并写通值/像素映射
---

# 01 画轨道和双手柄

← [手册首页](./index.md)　·　下一步 [02 拖动交互](./02-drag-handles.md) →

> 对照成品：`widget/range-slider/src/range_slider.cpp` 的 `paintEvent`（271 行起）、`trackRect` / `valueToX`（172 / 178 行）

## 目标

画出范围滑块的静态样子：一条圆角轨道（灰色药丸），中间一段高亮（选中区间），左右各一个白色圆形手柄。程序里改 `lower_value_` / `upper_value_`，两个手柄和区间跟着挪。

## 提示

- 先备好成员：`minimum_` / `maximum_`（默认 0 / 100）、`lower_value_` / `upper_value_`（默认 20 / 80），再加三个配色成员。
- 写 `trackRect()`：在 `rect()` 基础上左右各缩进「半个手柄直径」。想想为什么缩进——为了让 `minimum` / `maximum` 这两个端点值的圆心正好落在轨道两端，拖到尽头手柄不会越界。手柄直径先定个常量（16px）。
- 写 `valueToX(int value)`：`trackRect().left() + (value - minimum_) / (maximum_ - minimum_) * trackRect().width()`。**先别管除零**，下一步的约束 step 会补，但留个心眼。
- `paintEvent` 里：开抗锯齿；轨道条高度取 `kHandleDiameter * 0.55`（比手柄略瘦，手柄才凸出可点）；画底层轨道圆角条，再画选中区间（`lower` 到 `upper` 之间那一段），最后两个圆手柄。
- 区间那段是个 `drawRoundedRect`，圆角半径取「条高度的一半」就是完美药丸两端。
- 手柄圆心 x = `valueToX(lower_value_)` 和 `valueToX(upper_value_)`，y 取控件中线。
- `sizeHint` 给个 `{200, 24}`、`minimumSizeHint` 给 `{120, 20}`，别让布局把它压成 0。
- 不给整段代码，自己拼。

## 检查点

- 窗口里一条灰色药丸轨道，中间一段高亮，左右各一个白圆手柄，比例协调、四角圆润、无锯齿。
- 在构造函数里把 `lower_value_` 改成 40 重新跑，下手柄往右挪了，高亮区间变窄——说明 `valueToX` 映射通了。
- 拉伸窗口，轨道随手柄间距按比例伸缩，手柄圆心始终落在缩进后的轨道两端范围内。

对了进 [02 拖动交互](./02-drag-handles.md)。

## 对照答案

- 轨道 + 区间 + 手柄三层绘制在 `range_slider.cpp:281-302`。
- `trackRect`（缩进逻辑）在 `range_slider.cpp:172-176`。
- `valueToX` 映射在 `range_slider.cpp:178-186`。
- 尺寸契约在 `range_slider.cpp:166-167`。
