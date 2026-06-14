---
title: "卡住怎么办"
description: "按症状查：颜色不过渡、连切崩溃、呼吸卡顿、RGB 中间色浑浊、moc 报错——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/status-led/`，对照着看。

## 颜色不过渡、直接跳

- color 的 `Q_PROPERTY` 的 **WRITE 是不是指向了 `setAnimatedColor`**（纯赋值+update），而不是 setStatus？指错会导致动画驱动 setStatus、setStatus 又启动画、无限递归。→ `include/status_led.h:29`
- setStatus 里**有没有真的 `start()` 了 color_anim_**？→ `src/status_led.cpp:98-101`
- setAnimatedColor 里**有没有 `update()`**？没 update 就不重绘。→ `src/status_led.cpp:117`
- 进阶排查：[属性动画框架基础](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)、[属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)

## 频繁切换状态时崩溃（segfault）

- color_anim_ **是不是用了 `DeleteWhenStopped`**？stop 时对象被 delete，下次用就悬空。改成持久成员指针 + `stop()/重配/start()`。→ `src/status_led.cpp:55-56`、`src/status_led.cpp:98-101`
- 动画对象的 **parent 是不是设成了 this**？否则对象树不管它，可能提前释放。→ `src/status_led.cpp:56`
- 进阶排查：[动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)

## 呼吸卡顿 / 掉帧

- 动画回调里**是不是误用了 `repaint()`**？repaint 同步立即重绘，会卡。一律用 `update()`（异步合并）。→ `src/status_led.cpp:74`
- breathing_anim_ 的 `setLoopCount(-1)` 设了吗？没设就只跑一次。
- 进阶排查：[定时器](../../../../../beginner/01-qtbase/11-timer-beginner.md)

## 过渡时颜色「脏」（绿→红中间偏暗褐）

- 这不是 bug：Qt 对 QColor 默认 **RGB 线性插值**，绿红中间是橄榄色。300ms+OutCubic 快速逼近终点，肉眼基本无感。
- 要鲜艳：注册 HSV 插值器（见 [手册首页](./index.md) 进阶挑战）。

## 窗口缩到极小时 LED 消失

- 半径 `std::min(w,h)/2-1` 在 w/h 很小时**为负或 0**，drawEllipse 行为未定义。`std::max(1, ...)` 兜底。→ `src/status_led.cpp:223`

## moc 报错（Q_PROPERTY / Q_ENUM 不认识）

- 头文件**有没有 `Q_OBJECT`**？
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- Q_ENUM 的枚举**是不是在类里**、Q_ENUM 紧跟其后？→ `include/status_led.h:35-40`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
