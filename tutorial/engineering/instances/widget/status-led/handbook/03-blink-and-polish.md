---
title: "Step 5-6：BlinkMode 闪烁 + minimumSizeHint"
description: "QVariantAnimation 做正弦呼吸、QTimer 做 OnOff 明灭，过渡色与呼吸因子解耦合成；补 minimumSizeHint 收尾。"
---

# Step 5-6：BlinkMode 闪烁 + minimumSizeHint

← [Step 3-4](./02-color-transition.md) · [手册首页](./index.md) →

## Step 5：BlinkMode（None / OnOff / Breathing）

### 目标
加 `enum class BlinkMode { None, OnOff, Breathing }`。OnOff 是老式生硬明灭（QTimer 翻转），Breathing 是正弦呼吸（QVariantAnimation 无限循环驱动一个 0..1 亮度因子）。

### 提示

**OnOff（简单）**：
- `QTimer` 500ms，timeout 翻转一个 `onoff_visible_` 布尔 + `update()`
- paintEvent 里 `if (!onoff_visible_) c = c.darker(400)`

**Breathing（核心）**：
- 用 `QVariantAnimation`，`setLoopCount(-1)` 无限循环
- 配成 0→1→0：`setStartValue(0.0); setEndValue(0.0); setKeyValueAt(0.5, 1.0);`
- `setEasingCurve(InOutSine)`——天然正弦感
- 回调里 `breathing_factor_ = value.toDouble(); update();`
- paintEvent 里在 `base.darker(280)`（暗）↔ `base.lighter(140)`（亮）间按 factor 做 r/g/b 线性插值

### 关键认知——为什么呼吸和过渡不打架
- 过渡写 `current_color_`，呼吸写 `breathing_factor_`，**两个独立变量**
- paintEvent 入口 `applyDisplayTransform(current_color_)` 才合成：先用过渡色做 base，再叠呼吸亮度
- 所以「边过渡颜色边呼吸」天然并行，不用为「过渡中要不要暂停呼吸」设计状态机

抽个私有 `applyDisplayTransform(const QColor& base)` 专门干合成，paintEvent 只管画它的返回值。

### 检查点
- OnOff：500ms 一明一灭（生硬）
- Breathing：亮度正弦式起伏（InOutSine 节奏，不是线性）
- 同时开 Breathing 并切状态：颜色过渡 + 呼吸并行、不乱 = 解耦对了

> 定时器不熟？[定时器](../../../../../beginner/01-qtbase/11-timer-beginner.md)、进阶 [定时器进阶](../../../../../advanced/01-qtbase/11-qtimer-advanced.md)。自定义控件进阶？[自定义 Widget 进阶](../../../../../advanced/03-qtwidgets/05-custom-widget-advanced.md)。

### 对照答案
- breathing 动画配置（0→1→0 + InOutSine + 无限循环）：`src/status_led.cpp:65-70`
- breathing 回调写 factor：`src/status_led.cpp:73-74`
- setBlinkMode 分发（停所有 → 按模式启）：`src/status_led.cpp:127`
- applyDisplayTransform 呼吸插值：`src/status_led.cpp:200-202`

---

## Step 6：minimumSizeHint 收尾

### 目标
实现 `minimumSizeHint()`，让布局系统知道这个控件最小能缩到多小。缩窗时 LED 不会被裁切到看不见。

### 提示
- 返回合理最小尺寸，比如 `std::max(8, led_size_ / 2)`
- 配合已有 `sizeHint()`（返回 `led_size_`），布局系统就能在 sizeHint 和 minimumSizeHint 之间缩放

### 检查点
把 LED 放进布局、缩窗口到很小：LED 缩到最小尺寸但不消失 / 不裁切成怪形状 = minimumSizeHint 生效了。

### 对照答案
- minimumSizeHint：`src/status_led.cpp:185-186`

---

🎉 搓完了。跑 demo 对照成品：颜色过渡 + 呼吸 + OnOff 三种模式都能复现 = 你搓的和 repo 一致。

想再深？回 [手册首页](./index.md) 看进阶挑战（HSV 插值器消除暗褐中间色 / 状态色可配置 / 下一站 toggle-switch）。
