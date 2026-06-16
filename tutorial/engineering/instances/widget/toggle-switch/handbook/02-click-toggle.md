---
title: 02 点击切换与滑动动画
description: ToggleSwitch 手搓第 2 步——setChecked + QPropertyAnimation 驱动 handlePos 滑动
---

# 02 点击切换与滑动动画

> 对照成品：`widget/toggle-switch/src/toggle_switch.cpp` 的 `init_animation` / `animate_to` / `setChecked` / `mouseReleaseEvent`

## 目标

点一下控件，滑块带动画从左滑到右（或反过来），同时轨道颜色从灰过渡到绿。

## 提示

- 关键：把 `handlePos`（0..1）声明成 Q_PROPERTY（READ/WRITE/NOTIFY），`checked` 也声明成 Q_PROPERTY。两个属性。
- 建一个 `QPropertyAnimation(this, "handlePos", this)`，设时长（150ms 左右）、缓动 `OutCubic`。构造时建好、持久持有（别每次 new）。
- `setChecked(bool)`：状态没变就 return；变了就改成员、启动动画（startValue=当前 handlePos、endValue=1 或 0）、emit toggled。
- 轨道颜色别单独开动画——`paintEvent` 里直接按 handlePos 在灰/绿间插值，滑块和颜色自然同步。
- `mouseReleaseEvent` 里：左键松开就 `setChecked(!checked_)`（拖动下一步再加）。

## 检查点

- 点一下，滑块 150ms 顺滑滑到另一端，轨道颜色跟着渐变。
- 连点几下，每次都顺滑、不抖（提示：启动新动画前先 stop 旧的）。

对了进 [03 拖动交互](./03-drag-interaction.md)。

## 对照答案

- 动画初始化在 `toggle_switch.cpp:34-37`。
- 切换触发动画在 `setChecked`（49 行）+ `animate_to`（40 行，先 stop 再 start）。
- 颜色插值在 `toggle_switch.cpp:111`（blend）。
