---
title: "Step 1：无边框置顶 + 圆角自绘"
description: "用窗口标志位组合做出无边框、置顶、不抢焦点的临时窗口，paintEvent 自绘圆角底与左侧色条。"
---

# Step 1：无边框置顶 + 圆角自绘

← [手册首页](./index.md) · 下一步 [Step 2 淡入淡出 + 定时自毁](./02-fade-and-self-destruct.md) →

## 目标

屏幕右下角弹出一个**无边框、带圆角、深灰底、左侧有浅灰竖条**的矩形窗口。它**不在任务栏占位**、**置顶**、**不抢焦点**（你在别处打字光标不会跳走）。这步先做静态的——能 show 出来、样子对、窗口行为对，动画和自毁下一步。

## 提示

- 继承 `QWidget`，构造期 `setWindowFlags(...)` 用**组合**标志位：
  `Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus`
  - 漏 `Qt::Tool` 会塞任务栏；漏 `WindowDoesNotAcceptFocus` 会抢键盘焦点
- 再 `setAttribute(Qt::WA_TranslucentBackground)`——否则圆角外区域是黑块，不是真透明
- `setAttribute(Qt::WA_ShowWithoutActivating)`——show 时不激活窗口
- `setAttribute(Qt::WA_DeleteOnClose)`——为下一步自毁铺路（这步先用不上，但加上无妨）
- 重写 `paintEvent`：`QPainter` + `QPainterPath::addRoundedRect` 画圆角底；再画一个左侧 5px 宽的色条
- 圆角矩形用 `rect().adjusted(0.5, 0.5, -0.5, -0.5)` 避免边缘 1px 抗锯齿被裁掉
- 加个 `QLabel`（wordWrap）放正文，`QVBoxLayout` 铺上去；`setFixedWidth(320)` 让高度由文字撑开
- 这步先**不接 ToastManager**，直接 `new Toast(...)` + `show()`，能出来就行

## 检查点

跑出来右下角有个**圆角深灰矩形、左侧浅灰竖条、中间有文字**，**任务栏没有它的图标**，你在别处打字**光标不跳** = 标志位全对。

> 窗口标志位不熟？先读 [QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

## 对照答案

- 窗口标志位组合：`src/toast-notification.cpp:34-39`
- 圆角底 + 左侧色条自绘：`src/toast-notification.cpp:122-137`
- 基础色（四态）取色函数：`src/toast-notification.cpp:94-119`

---

下一步：[Step 2 把它变成会淡入淡出、到点自己消失](./02-fade-and-self-destruct.md)。
