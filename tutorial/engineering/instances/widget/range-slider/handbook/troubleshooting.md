---
title: 卡住怎么办
description: RangeSlider 手搓常见错——手柄不跟手、越过彼此、改端点约束破、窄控件崩——指向成品，不给完整答案
---

# 卡住怎么办

按症状找，只指方向。答案在成品 `widget/range-slider/`。

## 手柄画在固定位置 / 不跟着值走

`paintEvent` 里手柄圆心 x 写死成常量了，没用 `valueToX`。圆心 x 必须是 `valueToX(lower_value_)` / `valueToX(upper_value_)`，值变了手柄才挪。对照 `range_slider.cpp:287-288,301-302`。

## 拖动时手柄飞出轨道 / 越过另一手柄

setter 里没 clamp，直接把 `xToValue` 的结果写进了成员变量。`setLowerValue` 要夹到 `[minimum_, upper_value_ - gap]`、`setUpperValue` 夹到 `[lower_value_ + gap, maximum_]`。对照 `range_slider.cpp:113,126`。

## 改完 minimum / maximum 后约束破了（lower 跑到区间外）

端点 setter 只改了 `minimum_` / `maximum_`，没回夹现有 `lower_value_` / `upper_value_`。setter 收尾要把两值重新 `std::clamp(minimum_, x, maximum_)`，值变了补发信号。对照 `range_slider.cpp:44-53,69-77`。

## 分别 setMinimum / setMaximum 时中间态崩

先 `setMinimum(80)` 再 `setMaximum(20)`，第一步之后 `minimum > maximum` 就已经非法。用 `setRange` 一次性设两端（反着传会自动 swap），避开中间态。对照 `range_slider.cpp:82-104`。

## 两手柄重合后点一下抓错手柄

`hitTestHandle` 在两距离相等时没定优先级，或容差太大导致两个都命中。让 lower 优先（`d_lower <= d_upper` 时选 lower），且只有真正在容差内才命中。对照 `range_slider.cpp:204-209`。

## 两手柄重合时区间绘制异常

`x_upper == x_lower` 时 `drawRoundedRect` 拿到宽度 0 可能退化。区间宽度一律 `std::max(0.0, |x_upper - x_lower|)`。对照 `range_slider.cpp:291`。

## 点一下被当成拖动，手柄乱跑

没设移动阈值，手抖就被判拖。`mouseMoveEvent` 里移动没超过 `kDragThreshold`（4px）一律不进拖动模式；纯点击走松手吸附。对照 `range_slider.cpp:234-237,254-262`。

## 窗口缩很窄时崩 / 手柄飞掉

`xToValue` 里 `(x - left) / width` 在 `width <= 0` 或 `span <= 0` 时除零得 NaN。两个映射函数都得判兜底。对照 `range_slider.cpp:181-183,191-193`。

## valueToX 把端点手柄画到控件边界外（圆心越界）

`trackRect` 没缩进。左右各让出半个手柄直径，端点值的圆心才正好落在轨道两端。对照 `range_slider.cpp:172-176`。

## 拖动卡顿 / 掉帧

setter 里用了 `repaint()` 同步重绘，没走事件循环合并。一律 `update()` 异步重绘。对照 `range_slider.cpp:120,133`。

---

实在卡死，成品 `widget/range-slider/src/range_slider.cpp` 就是答案——但先自己拼。
