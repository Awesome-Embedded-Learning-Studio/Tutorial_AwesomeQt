---
title: "Step 3-4：value/progress 解耦 + 平滑过渡"
description: "把 value（业务值）和 progress（动画产物）拆成两个属性，用 QPropertyAnimation 从当前进度接力到新进度，弧平滑铺开。"
---

# Step 3-4：value/progress 解耦 + 平滑过渡

← [手册首页](./index.md) · 上一步 [Step 1-2 环与弧](./01-ring-and-progress.md) · 下一步 [Step 5-6 配色与兜底](./03-polish-and-properties.md) →

## Step 3：把 value 和 progress 拆成两个属性

### 目标

现在 progress 是临时算的（`value/100.0`）。这一步把它升级成**独立的 Q_PROPERTY**，和 value 分开。两个成员：`value_`（0..100，业务值）和 `progress_`（0..1，动画产物），paintEvent 只认 `progress_`。

### 提示

- 头文件加两个 Q_PROPERTY：`value`（READ/WRITE/NOTIFY）和 `progress`（READ/WRITE/NOTIFY）
- `progress` 的 WRITE 必须是**纯赋值**的 `setDisplayProgress(double)`：夹到 [0,1] → 赋给 `progress_` → emit → `update()`。**不要**在这里启动画
- `setValue(int)` 是业务入口：改 `value_`、emit valueChanged，然后（下一步）启动动画
- 想清楚为什么拆：如果 progress 的 WRITE 指向 setValue，动画驱动 setValue、setValue 又启动画——栈溢出。拆开才安全

### 检查点

`setDisplayProgress(0.6)` 后弧铺 60%、中间显 "60%"；`value()` 和 `progress()` 能分别取到 = 解耦对了。这时候 setValue 还没接动画，setValue 后弧会突变——正常，下一步修。

> 属性系统不熟？[Qt 属性系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。

### 对照答案

- 两个 Q_PROPERTY 声明：`include/circle_progress.h:29-30`
- `value_` / `progress_` 成员：`include/circle_progress.h:79-80`
- setDisplayProgress（纯赋值+emit+update）：`src/circle_progress.cpp:77`

---

## Step 4：QPropertyAnimation 让弧平滑铺开

### 目标

setValue 不再突变，而是 400ms `OutCubic` 把弧从**当前显示进度**接力铺到新进度。

### 提示

- 成员加 `QPropertyAnimation* progress_anim_`，构造时 `new QPropertyAnimation(this, "progress", this)`——parent=this 对象树托管，**不用 DeleteWhenStopped**
- 构造里设 `setDuration(400)` + `setEasingCurve(QEasingCurve::OutCubic)`
- setValue 里三件套：`progress_anim_->stop()` → `setStartValue(progress_)`（当前显示进度，可能是上次动画中间值）→ `setEndValue(value/100.0)` → `start()`
- 关键是 `setStartValue(progress_)` 用**当前进度**，不是 0、也不是旧 value。这样连点 setValue 弧不会跳回旧目标
- progress_anim_ 是持久指针，反复 stop()/重配/start() 复用，不每次 new

### 检查点

setValue(60) 后弧花 ~0.4 秒平滑铺到 60%；在动画中途再 setValue(20)，弧从**当前中间位置**接力往 20% 收（不跳回 100% 再下来）= 接力逻辑对了。

### 对照答案

- 动画对象初始化（持久指针、parent=this）：`src/circle_progress.cpp:47`
- setValue 的 stop/接力/start 三件套：`src/circle_progress.cpp:55`

---

下一步收尾：[Step 5-6 配色线宽 Q_PROPERTY + 几何 clamp 兜底](./03-polish-and-properties.md)。
