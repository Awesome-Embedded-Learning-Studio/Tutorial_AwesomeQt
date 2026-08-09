---
title: "卡住怎么办"
description: "按症状查：圆角外黑块、抢焦点、任务栏刷屏、连点崩溃、淡出串台、不自动消失、多条重叠、消失留空洞——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `model/12-design-patterns/toast-notification/`，对照着看。

## 圆角外是黑块不是透明

- 没设 `WA_TranslucentBackground`？圆角矩形外的那四个角，窗口默认用背景色填，看着就是黑矩形。→ `src/toast-notification.cpp:37`
- 进阶排查：[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)

## Toast 一弹出，任务栏塞满图标

- 漏了 `Qt::Tool` 标志？没它，每个顶层窗口都进任务栏。→ `src/toast-notification.cpp:34`
- 进阶排查：[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)

## Toast 一弹，正在输入的输入框丢了光标（抢焦点）

- 漏了 `Qt::WindowDoesNotAcceptFocus`？或者 `WA_ShowWithoutActivating` 没设？两者一起才彻底不抢焦点。→ `src/toast-notification.cpp:34-38`

## 连点多次按钮偶发崩溃

- `fadeOut` 没防重入？`display_timer_` 超时和别处触发可能同时进 `fadeOut`，重复启动淡出 / 二次 close。加 `fading_out_` 标志。→ `src/toast-notification.cpp:80`
- 淡入、淡出都连了 `opacity_anim_::finished` 没断开？淡出结束会串台触发淡入回调。进 fadeOut 前先 `disconnect`。→ `src/toast-notification.cpp:89`
- 进阶排查：[动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)

## Toast 不自动消失 / 内存泄漏

- 没设 `WA_DeleteOnClose`？`close()` 不会释放对象，长时间运行内存涨。→ `src/toast-notification.cpp:39`
- 淡出动画 `finished` 回调里没调 `close()`？没 close 就不触发 DeleteOnClose，对象赖着不销毁。→ `src/toast-notification.cpp:90`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

## 多条 Toast 叠在同一点重叠

- 没维护活动队列？各自独立定位就都落到角落同一点。要单例 `ToastManager` 持 `QList<Toast*>`，累计 y 偏移定位。→ `src/toast-notification.cpp:188`
- 外部绕过 `showToast` 直接 `new Toast`？那它不进队列，堆叠管不到。**只走 `ToastManager::showToast`**。→ `src/toast-notification.cpp:155`

## 一条消失后余下不回填角落（留空洞）

- 自毁后没触发重排？Toast 消失了队列还按老位置摆，越堆越散。挂 `destroyed` 信号回调，移出队列后 `relayout`。→ `src/toast-notification.cpp:255`
- `destroyed` 回调里访问了正在析构对象的成员？不安全。先 `removeAll`（只比指针），再遍历**余下存活**的。→ `src/toast-notification.cpp:243-253`

## 单例构造崩（qApp 还没就绪）

- 用了裸静态局部 `static ToastManager instance;`？QObject 构造时 `qApp` 可能未就绪。改成惰性指针 + `setParent(qApp)`。→ `src/toast-notification.cpp:143-151`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
