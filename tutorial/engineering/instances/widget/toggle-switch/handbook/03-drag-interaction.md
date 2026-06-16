---
title: 03 拖动交互
description: ToggleSwitch 手搓第 3 步——按住拖动滑块、阈值区分点击/拖动、松手吸附
---

# 03 拖动交互

> 对照成品：`widget/toggle-switch/src/toggle_switch.cpp` 的 mousePressEvent / mouseMoveEvent / mouseReleaseEvent（128/139/156 行）

## 目标

除了点击，还能按住滑块拖到任意位置，松手后吸附到最近端。

## 提示

- `mousePressEvent`：记下按下 x 和当时 handlePos，dragging 标志先 false。
- `mouseMoveEvent`：移动距离超过阈值（4px）才把 dragging 置 true，**这一刻要 stop 动画**（不然动画和拖动抢 handlePos）。dragging 后按「移动距离 / 滑块行程」换算成 handlePos 增量，直接 setHandlePos。
- `mouseReleaseEvent`：dragging 过就按 `handlePos > 0.5` 吸附到开/关；没拖（纯点击）就 `setChecked(!checked_)`。
- 鼠标 x → handlePos 的换算别忘了「控件太窄时行程为 0」的守护，别除零或算出负数。

## 检查点

- 按住滑块拖，滑块跟手，轨道颜色实时变。
- 拖到一半松手，滑块动画吸附到最近端，toggled 信号正确发出。
- 纯点一下（不动），照常切换，不被当成拖。

搓完了，回 [手册首页](./index.md) 做进阶挑战。

## 对照答案

- 阈值 + 停动画在 `toggle_switch.cpp:146-148`。
- x→pos 换算 + 窄控件守护在 `x_to_pos`（`toggle_switch.cpp:96-103`）。
- 松手吸附 vs 点击切换在 `mouseReleaseEvent`（156 行起）。
