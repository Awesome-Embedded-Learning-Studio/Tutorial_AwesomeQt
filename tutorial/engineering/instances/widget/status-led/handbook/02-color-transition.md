---
title: "Step 3-4：Q_PROPERTY + 颜色过渡（核心）"
description: "把突变色升级成 300ms 丝滑过渡：Q_PROPERTY 暴露 color 属性，QPropertyAnimation 每帧驱动，持久指针防连切悬空。"
---

# Step 3-4：Q_PROPERTY + 颜色平滑过渡（核心）

← [Step 1-2](./01-paint-and-status.md) · [手册首页](./index.md) · 下一步 [Step 5-6 闪烁](./03-blink-and-polish.md) →

这两步是整个控件的核心——把 step 2 的「突变色」升级成「300ms 丝滑过渡」。诀窍不在动画本身，而在**用一个 Q_PROPERTY 把「当前显示色」暴露出去，让 QPropertyAnimation 每帧去写它**。

## Step 3：把 status 升级为 Q_PROPERTY

### 目标
给类加一行 `Q_PROPERTY(Status status READ status WRITE setStatus NOTIFY statusChanged)`，补上 `statusChanged` 信号。功能上和 step 2 一样，但 status 现在是「Qt 认识的属性」——能被元系统枚举、被 Designer 编辑、被动画按名字驱动。

### 提示
- Q_PROPERTY 三件：READ 一个 getter、WRITE 一个 setter、NOTIFY 一个信号
- setStatus 末尾 `emit statusChanged(s)`
- 注意：**这步 WRITE 还指 setStatus**（业务入口）。step 4 加颜色属性时，color 的 WRITE 要指向一个**纯赋值的回调**——别搞混（见 [troubleshooting](./troubleshooting.md) 的「递归栈溢出」）

### 检查点
编译过（moc 不报错）+ `metaObject()->propertyCount()` 能数到 status = Q_PROPERTY 生效了。

> Q_PROPERTY 机制不熟？[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)、进阶 [属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案
- Q_PROPERTY(status) 声明：`include/status_led.h:28`
- statusChanged 信号：`include/status_led.h` signals 区

---

## Step 4：颜色丝滑过渡（QPropertyAnimation 驱动 QColor）

### 目标
加 `Q_PROPERTY(QColor color ...)`，setStatus 时启动一个 `QPropertyAnimation` 把 color 从当前值过渡到目标状态色。切换时颜色 300ms 平滑渐变，不是突变。

### 提示（按顺序）
1. **新增成员 `QColor current_color_`**——这才是 paintEvent 实际画的色（不再直取 `statusColor(status_)`）
2. **新增 `setAnimatedColor(const QColor&)` 作为 color 的 WRITE 回调**：纯赋值 `current_color_ = c` + `emit colorChanged` + `update()`。这是动画每帧调的——**只赋值，不启动画**
3. **改 setStatus**：不再直接换色，而是
   - `color_anim_->stop()`
   - `setStartValue(current_color_)`（从**当前显示色**接力，不是从上一次的目标色）
   - `setEndValue(statusColor(new_status))`
   - `start()`
4. **color_anim_ 用持久成员指针**（构造时 `new QPropertyAnimation(this, "color", this)`，配 `setDuration(300)` + `setEasingCurve(OutCubic)`），**不要用 `DeleteWhenStopped`**——否则频繁切换时指针悬空崩（见 [troubleshooting](./troubleshooting.md)）
5. paintEvent 改成画 `current_color_`

### 关键认知
- **Qt 内置 QColor 插值**（RGB 线性），所以 `QPropertyAnimation(this, "color")` 能直接对颜色做动画，不用手写 lerp
- **WRITE 指 setAnimatedColor（纯赋值）而非 setStatus（会启动画）**——否则动画驱动 setStatus → setStatus 又启动画 → 无限递归栈溢出
- **从 current_color_ 接力**而非从目标色重启——快速连切时颜色连续过渡，不跳变

### 检查点
切换状态时颜色 **300ms 渐变过渡**（不是突变）= 动画对了。快速连点 Cycle 不崩、过渡不跳变 = 接力逻辑对了。

> 动画框架不熟？[属性动画框架基础](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)、进阶 [动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)。

### 对照答案
- color_anim_ 配置（duration/easing）：`src/status_led.cpp:58-59`
- setStatus 启动过渡（stop/setStart/setEnd/start）：`src/status_led.cpp:98-101`
- setAnimatedColor 回调（赋值+emit+update）：`src/status_led.cpp:115-117`
- paintEvent 改画 current_color_：`src/status_led.cpp:220`

---

下一步：[Step 5-6 给它加闪烁（OnOff + 呼吸）](./03-blink-and-polish.md)。
