---
title: 03 约束与端点夹值
description: RangeSlider 手搓第 3 步——lower<=upper 约束 + 端点 setter 统一夹值 + setRange 一次性设两端
---

# 03 约束与端点夹值

← [手册首页](./index.md)　·　上一步 [02 拖动交互](./02-drag-handles.md) →

> 对照成品：`widget/range-slider/src/range_slider.cpp` 的 `setLowerValue` / `setUpperValue`（111 / 125 行）、`setMinimum` / `setMaximum` / `setRange`（34 / 60 / 82 行）

## 目标

把"约束"做扎实：不管外部怎么喂值、怎么改端点，控件永远维持 `lower <= upper` 且两值都在 `[minimum, maximum]` 内。这一步是把 step 2 里直接赋值的地方升级成带约束的 setter，再补全端点 setter 的统一夹值流程。

## 提示

- **手柄值 setter 内 clamp**：`setLowerValue` 把入参 `std::clamp(value, minimum_, upper_value_ - kHandleGap)`——下界是 minimum，上界是 `upper - gap`，这样 lower 永不越过 upper。`setUpperValue` 对称地夹到 `[lower + gap, maximum_]`。`kHandleGap` 默认 0 允许两手柄重合，想留间距调大它。clamp 后若值没变就 `return`（防无谓 `update`），变了就赋值 + emit 对应的 `*Changed` + `rangeChanged` + `update()`。
- **`setMinimum` / `setMaximum` 改端点要回夹现有值**：`setMinimum(50)` 之后旧的 `lower = 20` 就跑到区间外了。两个 setter 收尾都得把 `lower_value_` / `upper_value_` 重新 `std::clamp(minimum_, x, maximum_)`，值变了补发 `lowerValueChanged` / `upperValueChanged` / `rangeChanged`，没变只发 `minimumChanged` / `maximumChanged`。别漏这个夹值步骤，否则端点改了约束就破了。额外细节：`setMinimum` 若新值越过当前 maximum，把 maximum 也顶上去（`setMaximum` 对称处理）。
- **`setRange` 一次性设两端**：分别 `setMinimum` 再 `setMaximum` 会有"刚设完 minimum、maximum 还没跟上"的中间非法态。`setRange` 先静默写两端（`min > max` 自动 `std::swap`，别断言——纠正比报错友好），再走统一夹值流程、按需补发信号。
- 把 step 2 里直接改成员变量的地方，全换成调这些带约束的 setter。拖动时 `setLowerValue(v)` / `setUpperValue(v)`，约束自动兜住——拖过头会被夹住、不会越过另一手柄。
- 这一坨 setter 很重复，写完一个照葫芦画瓢即可，关键是想清楚"每个 setter 写完后，不变式还成立吗"。
- 不给整段代码，自己拼。

## 检查点

- 拖下手柄往右追上手柄，到达重合点（gap=0）就停，不会越过。
- 拖上手柄往左追下手柄同理停住。
- `setLowerValue(999)`（远超 maximum），手柄夹到合法上界，不越界、不崩。
- `setMinimum(50)`（高于当前 lower=20），lower 被夹到 50，标签 / 信号正确更新。
- `setRange(100, 0)`（反着传），自动交换成 `[0, 100]`，不报错不崩。
- demo 的程序化按钮组：连点 `setLowerValue(10)` 再点 `setUpperValue(90)`，每次都落在合法范围内，区间高亮方向正确。

搓完了，回 [手册首页](./index.md) 做进阶挑战。

## 对照答案

- 手柄值 setter 内 clamp 在 `range_slider.cpp:111-121` / `125-134`。
- `setMinimum` / `setMaximum` 回夹现有值在 `range_slider.cpp:34-56` / `60-80`。
- `setRange` 一次性设两端 + 自动交换在 `range_slider.cpp:82-104`。
