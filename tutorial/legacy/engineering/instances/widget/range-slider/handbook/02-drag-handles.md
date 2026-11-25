---
title: 02 拖动交互
description: RangeSlider 手搓第 2 步——双手柄 hit-test + 拖动阈值 + 松手吸附，复用 toggle-switch 三事件模式
---

# 02 拖动交互

← [手册首页](./index.md)　·　上一步 [01 画轨道和双手柄](./01-paint-track-handles.md)　·　下一步 [03 约束与端点夹值](./03-constraints-range.md) →

> 对照成品：`widget/range-slider/src/range_slider.cpp` 的 `hitTestHandle`（198 行起）、三个 `mouse*Event`（216 / 228 / 249 行）、`xToValue`（188 行）

## 目标

能拖任一手柄，拖动时手柄跟手、区间高亮同步变化；点在手柄附近松手，最近手柄吸到点击位置。这一步先**不碰约束**——只要能拖、能跟手即可，约束留给 step 3。

## 提示

- 加一个私有 `enum class ActiveHandle { kNone, kLower, kUpper }` 标识当前抓的手柄，再加 `press_x_`（按下时鼠标 x）、`dragging_` 两个状态成员。
- 写 `xToValue(qreal x)`：`valueToX` 的反函数。`minimum_ + (x - trackRect().left()) / trackRect().width() * (maximum_ - minimum_)`，结果 `std::round` 成 int。**这里要加除零兜底**：`span <= 0` 或 `width <= 0` 时直接返回 `minimum_`，不然窄控件会崩。
- 写 `hitTestHandle(qreal x)`：算两个手柄当前圆心 x（`valueToX(lower)` / `valueToX(upper)`），算鼠标离两者的距离，离谁近且在容差（14px 左右）内就返回谁，都不在就 `kNone`。两个手柄重合时让 lower 优先（距离相等选 lower）。
- `mousePressEvent`：左键按下记 `press_x_`、`dragging_ = false`，调 `hitTestHandle` 预选 `active_handle_`。注意这是**预选**，不是定死——真正的抓取等拖过阈值再说。
- `mouseMoveEvent`：移动距离超过 `kDragThreshold`（4px）才把 `dragging_` 置 true。一旦进入拖动，`v = xToValue(当前x)`，按 `active_handle_` 调对应的 setter（这一步你可以先直接写成员变量赋值 + `update()`，step 3 再升级成带约束的 setter）。
- `mouseReleaseEvent`：若全程 `!dragging_` 且 `active_handle_` 非 `kNone`，就把预选手柄吸到点击位置（`v = xToValue(x)` 设过去）。收尾把 `active_handle_` 重置成 `kNone`、`dragging_ = false`。
- 这套三事件 + 阈值，和 toggle-switch 是同一个模板，节奏完全一致——拖过阈值才算拖、纯点击走吸附。
- 不给整段代码，自己拼。

## 检查点

- 按住下手柄拖，下手柄跟手，区间左边界实时移动。
- 按住上手柄拖，上手柄跟手，区间右边界实时移动。
- 点在离下手柄 14px 内的地方松手（不拖），下手柄吸到点击位置。
- 手只点一下、手抖 1-2px，不会被判成拖动（手柄不乱跑）。
- 把窗口缩很窄还能拖不崩——说明 `xToValue` 的除零兜底生效了。

对了进 [03 约束与端点夹值](./03-constraints-range.md)。

## 对照答案

- `xToValue`（含除零兜底）在 `range_slider.cpp:188-196`。
- `hitTestHandle`（最近优先 + 容差）在 `range_slider.cpp:198-211`。
- 三事件 + 阈值 + 吸附在 `range_slider.cpp:216-266`。
