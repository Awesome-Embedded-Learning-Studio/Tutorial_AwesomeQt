---
title: "Step 5：demo 收尾（时长滑块 + 瞬时 opacity + QSignalBlocker）"
description: "把 FadeWidget 装进 demo：Fade In/Out 按钮、fadeDuration 滑块、瞬时 opacity 滑块；用 QSignalBlocker 解决动画回灌滑块的信号环。"
---

# Step 5：demo 收尾（时长滑块 + 瞬时 opacity + QSignalBlocker）

← [Step 3-4](./02-fade-animation.md) · [手册首页](./index.md) →

把 step 1-4 的 FadeWidget 装进一个能交互的 demo：两个按钮 + 两个滑块。这一步的重头是**瞬时 opacity 滑块**——它会暴露一个信号环坑，正好用 `QSignalBlocker` 收掉。

## Step 5：demo UI（按钮 + 两个滑块）

### 目标

做一个主窗口，里面：①FadeWidget 容器（放带色面板 + 文字，证明内容跟着淡）；②「Fade In」/「Fade Out」按钮；③fadeDuration 滑块（100-1500ms）；④瞬时 opacity 滑块（绕过动画直接 setOpacity）。

### 提示

**容器内容**：

- `FadeWidget` 里放一个带 `QFrame`（设 stylesheet 上色 + 圆角）+ `QLabel` 文字——证明「内容跟着容器一起淡」

**按钮**：

- Fade In 按钮连 `fade_widget_->fadeIn(fade_widget_->fadeDuration())`，Fade Out 同理——用当前 fadeDuration 当时长

**fadeDuration 滑块**：

- `setRange(100, 1500)`，连 `valueChanged` → `setFadeDuration(value)` + 更新数值标签
- 这条单向同步，没环（setFadeDuration 不回灌滑块）

**瞬时 opacity 滑块（核心坑）**：

- `setRange(0, 100)`，valueChanged 里 `setOpacity(value / 100.0)`（整型映射到 0.0..1.0）+ 更新标签
- **反向同步**：连 `FadeWidget::opacityChanged` → 动画运行时也改 opacity，滑块和标签要跟着走
- 反向 setValue 滑块时，若不屏蔽信号会触发 `valueChanged`→`setOpacity`→`opacityChanged`→再 setValue，**形成信号环**
- 解法：`QSignalBlocker blocker(slider);` 包住 `slider->setValue(pct)`，且 `if (slider->value() != pct)` 才写，避免无意义写入

### 关键认知——为什么 opacity 滑块会成环

- 滑块→setOpacity→opacityChanged→滑块：这是一个完整的环
- fadeDuration 滑块没这问题，因为 `setFadeDuration` 不回灌滑块（它只 emit fadeDurationChanged，demo 没接这个信号去写滑块）
- `QSignalBlocker` 是 RAII：作用域内屏蔽，出了作用域自动恢复——比手动 `blockSignals(true/false)` 安全（不会忘恢复）
- 顺手加 `if (slider->value() != pct)` 判断：即使信号被屏蔽，也避免重复写同一个值

### 检查点

- 点 Fade In / Fade Out：内容平滑淡入淡出，且 opacity 滑块和标签**实时跟着动画走、不抖动** = QSignalBlocker 防环对了
- 拖 fadeDuration 滑块：下次淡入淡出变快/变慢 = 时长同步对了
- 拖 opacity 滑块：透明度**瞬时变**（无动画），证明 setOpacity 是真 Q_PROPERTY

> 信号槽 / QSignalBlocker 不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、进阶 [属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案

- 容器内容（带色 QFrame + 文字）：`demo/fade_animation_window.cpp:50-64`
- Fade In / Fade Out 按钮：`demo/fade_animation_window.cpp:69-78`
- fadeDuration 滑块同步：`demo/fade_animation_window.cpp:87-101`
- 瞬时 opacity 滑块：`demo/fade_animation_window.cpp:111-126`
- 反向同步 + QSignalBlocker 防环：`demo/fade_animation_window.cpp:129-137`

---

搓完了。跑 demo 对照成品：淡入淡出平滑、duration 可调、opacity 滑块瞬时生效且不与动画互相抖 = 你搓的和 repo 一致。

想再深？回 [手册首页](./index.md) 看进阶挑战（fadeOut 可选不 hide / 组合 geometry+opacity 双动画 / 下一站 toggle-switch）。
