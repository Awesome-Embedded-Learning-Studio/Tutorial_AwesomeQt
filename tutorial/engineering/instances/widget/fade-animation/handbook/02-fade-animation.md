---
title: "Step 3-4：QPropertyAnimation 驱动 opacity（核心）"
description: "QPropertyAnimation 驱动 effect 的 opacity 做淡入淡出：持久指针复用、runFade 接力、fadeIn/fadeOut 的 show/hide 时机。"
---

# Step 3-4：淡入淡出动画（核心）

← [Step 1-2](./01-effect-and-opacity.md) · [手册首页](./index.md) · 下一步 [Step 5 demo 收尾](./03-demo-and-polish.md) →

这两步是整个控件的核心——用一个 `QPropertyAnimation` 写 step 1 挂的那个 effect 的 `opacity`，让透明度随时间过渡。诀窍在两处：**持久指针复用**（防连发悬空）和 **show/hide 时机**（防「先蹦出来再淡」）。

## Step 3：QPropertyAnimation 驱动 opacity 做淡入淡出

### 目标

加一个 `runFade(qreal end, int duration_ms)` 私有方法，把 effect 的 opacity 从**当前值**过渡到 `end`，时长 `duration_ms`。再写 `fadeIn()` / `fadeOut()` 两个入口包它。

### 提示（按顺序）

1. **构造期建持久动画指针**：`fade_anim_ = new QPropertyAnimation(effect_, "opacity", this)`——注意 target 是 `effect_`、property 是 `"opacity"`、parent 是 this 托管。配 `setEasingCurve(QEasingCurve::OutCubic)`
2. **连 finished 回调**：`QObject::connect(fade_anim_, &QPropertyAnimation::finished, this, [...]{ ... emit fadeFinished(...); })`。fadeOut 走完要 `hide()`——用一个成员 bool `fading_out_` 标记，回调里判断它
3. **写 runFade**：
   - `fade_anim_->stop()`（先停）
   - `setDuration(duration_ms)`（<1 兜底成 1）
   - `setStartValue(effect_->opacity())`——**从当前实时 opacity 接力**，不是从 0 或 1
   - `setEndValue(end)`
   - `start()`
4. **动画驱动的是 effect 的 opacity**，所以每帧 Qt 自动调 `effect_->setOpacity(中间值)` + effect 自己 update——你不用写任何逐帧回调

### 关键认知

- **持久指针，禁 `DeleteWhenStopped`**：和 status-led 同一套教训。`DeleteWhenStopped` 在连发动画时让旧指针悬空。持久指针 + stop/重配/start，从当前值接力争不跳变
- **setStartValue 取当前 opacity**：快速连点 Fade In / Fade Out 时，透明度连续过渡，不会跳回 0 或 1

### 检查点

调 `runFade(1.0, 300)`：透明度从当前值平滑过渡到完全不透明 = 动画对了。连点多次不崩、不跳变 = 接力逻辑对了。

> 动画框架不熟？[属性动画框架基础](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)、进阶 [动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)。

### 对照答案

- fade_anim_ 持久指针 new + easing + 连 finished：`src/fade_animation.cpp:21-29`
- runFade 接力（stop/duration/setStartValue/setEndValue/start）：`src/fade_animation.cpp:87-97`

---

## Step 4：fadeIn / fadeOut 入口 + show/hide 时机

### 目标

把 step 3 的 runFade 包成 `fadeIn(int duration_ms = 300)` 和 `fadeOut(int duration_ms = 300)` 两个对外入口，并处理「动画从正确起点开始」的可见性问题。

### 提示

**fadeIn**：

- 不可见时**先 `effect_->setOpacity(0.0)` 再 `show()`**——否则 effect 初值是 1.0，show 会先把全不透明画面渲染出来，再从 0 淡入，视觉上是「先蹦出来再淡」
- `fading_out_ = false`（标记在淡入，finished 时保持可见）
- `runFade(1.0, duration_ms)`

**fadeOut**：

- **先 `show()`**——必须可见才能看到淡出过程
- `fading_out_ = true`（finished 回调据此 hide）
- `runFade(0.0, duration_ms)`

### 关键认知

- 两个入口的 show 时机**相反**：fadeIn 是「先置透明再 show」，fadeOut 是「先 show 才看得见」
- finished 回调里 `if (fading_out_) hide();`——fadeOut 走完隐藏，fadeIn 走完保持可见，语义对称

### 检查点

- 点 Fade In：从全透明平滑渐显到全显示，**没有先闪一下全图** = show 时机对了
- 点 Fade Out：从当前平滑淡到全透明，结束后控件消失 = fading_out_ + hide 逻辑对了

> show/hide 与可见性不熟？[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

### 对照答案

- fadeIn（先 setOpacity(0.0) 再 show + runFade）：`src/fade_animation.cpp:32-40`
- fadeOut（先 show + fading_out_=true + runFade）：`src/fade_animation.cpp:42-49`
- finished 回调按 fading_out_ 决定 hide + emit：`src/fade_animation.cpp:23-29`

---

下一步：[Step 5 把它装进 demo——时长滑块 + 瞬时 opacity 滑块（QSignalBlocker 防环）](./03-demo-and-polish.md)。
