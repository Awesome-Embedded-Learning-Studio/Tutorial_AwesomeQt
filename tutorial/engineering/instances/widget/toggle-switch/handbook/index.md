---
title: ToggleSwitch 手搓手册
description: 跟着手搓一个自绘滑动开关——打通 QPainter 自绘 + QPropertyAnimation + 鼠标交互
---

# ToggleSwitch 手搓手册

> **source**：做完对照成品 `widget/toggle-switch/` · **related**：[自定义控件绘制入门](../../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)、[动画框架入门](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)

这份手册带你从零搓一个 `AwesomeQt::ToggleSwitch`——能点能拖、滑块带滑动动画、轨道颜色过渡的滑动开关。成品代码就是答案钥匙。

## 0. 你将学到

- **自绘控件**：继承 QWidget 重写 paintEvent，画圆角轨道 + 圆形滑块。
- **属性动画**：QPropertyAnimation 驱动一个 [0,1] 的位置属性，做出滑块滑动。
- **鼠标交互状态机**：mousePress/Move/Release 三件套，区分点击和拖动。

## 1. 起点

新建工程，链接 `Qt6::Widgets`，写最小 `ToggleSwitch` 类——继承 QWidget，paintEvent 先画个占位（一个灰矩形就行），main 里 new 出来 show。空壳能跑再进下一步。

## 2. 任务清单

| 步 | 目标 | 手册 |
|---|---|---|
| 1 | 画出圆角轨道 + 滑块 | [01 画轨道和滑块](./01-paint-the-switch.md) |
| 2 | 点击切换 + 滑动动画 | [02 点击切换与滑动动画](./02-click-toggle.md) |
| 3 | 按住拖动滑块 | [03 拖动交互](./03-drag-interaction.md) |

卡住去 [卡住怎么办](./troubleshooting.md)。

## 3. 进阶挑战

- 加 `Q_PROPERTY` 暴露轨道开/关两色，外部能换主题色（成品就是这么做的）。
- 滑块加阴影/高光（多画一层径向渐变），更接近 iOS 质感。
- 动画时长做成可配属性——往 `circle-progress` 这种带弧度的控件过渡。
