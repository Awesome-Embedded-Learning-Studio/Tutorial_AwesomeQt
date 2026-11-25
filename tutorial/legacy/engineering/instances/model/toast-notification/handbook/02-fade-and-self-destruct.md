---
title: "Step 2：淡入淡出 + 定时自毁"
description: "用 QPropertyAnimation 驱动 windowOpacity 做淡入淡出，QTimer 定时 + deleteLater + WA_DeleteOnClose 实现到点自毁。"
---

# Step 2：淡入淡出 + 定时自毁

← [手册首页](./index.md) · 上一步 [Step 1 无边框置顶](./01-frameless-window.md) · 下一步 [Step 3 单例堆叠管理](./03-stacking-manager.md) →

## 目标

把 Step 1 的静态窗口变成：**show 时从透明淡入到不透明**，**停留若干秒**，**再淡出消失**，**淡出完对象自动销毁**。连点多次不崩。

## 提示

### 淡入淡出

- `windowOpacity` 是 QWidget **顶层窗口自带属性**（0..1），`QPropertyAnimation(this, "windowOpacity", this)` 直接驱动它。**别用** `QGraphicsOpacityEffect`——它对顶层窗口的 `WA_TranslucentBackground` 圆角透明区无效
- 持久成员指针持有 `QPropertyAnimation`，`parent=this` 让对象树托管；**禁 `DeleteWhenStopped`**（stop 时被 delete 会悬空，同 StatusLED 教训）
- `fadeIn`：`opacity_anim_->stop(); setStartValue(windowOpacity()); setEndValue(1.0); start();`——从当前值接力，连点不跳变
- `fadeOut`：同理但 `setEndValue(0.0)`
- 构造期先 `setWindowOpacity(0.0)`，初始透明

### 定时自毁

- `QTimer* display_timer_ = new QTimer(this); display_timer_->setSingleShot(true);` 到点触发 `fadeOut`
- `fadeIn` 里 `display_timer_->start(duration_)` 启动计时
- `fadeOut` 里先 `display_timer_->stop()` 再启动淡出动画
- 淡出动画 `finished` 信号回调里 `close()`——配合 Step 1 的 `WA_DeleteOnClose`，`close()` 自动触发 `deleteLater`，对象安全回收

### 防重入

- 加 `bool fading_out_` 标志：`fadeOut` 入口判 `if (fading_out_) return;`——定时器超时和别处触发可能同时进 `fadeOut`
- 进入淡出前，**先 `disconnect(opacity_anim_, &QPropertyAnimation::finished, nullptr, nullptr)`** 再 `connect` 新的 close 回调。否则 fadeIn 挂的 finished 回调和 fadeOut 挂的会同时留在对象上，淡出结束串台

## 检查点

show 出来**从透明渐显**（不是突然蹦出来）= 淡入对了；**几秒后自动淡出消失** = 定时自毁对了；连点十次按钮**不崩** = 防重入对了。

> 动画框架不熟？先读 [动画框架基础](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)；定时器读 [定时器](../../../../../beginner/01-qtbase/11-timer-beginner.md)。

## 对照答案

- 持久动画指针 + 驱动 windowOpacity：`src/toast-notification.cpp:57`
- fadeIn 接力 + 启动计时：`src/toast-notification.cpp:68-77`
- fadeOut 防重入 + disconnect/close：`src/toast-notification.cpp:79-92`
- WA_DeleteOnClose（close 触发 deleteLater）：`src/toast-notification.cpp:39`

---

下一步：[Step 3 让多条 Toast 自动堆叠不重叠](./03-stacking-manager.md)。
