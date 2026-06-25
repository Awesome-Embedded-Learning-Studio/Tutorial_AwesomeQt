---
title: "卡住怎么办"
description: "按症状查：弧方向画反、指针偏 90°、指针不转/直接跳、连切崩溃、控件压扁消失、moc 报错——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/speed-meter/`，对照着看。

## 背景弧方向画反（逆时针铺 / 开口朝上）

- 把屏幕角 β 直接塞进了 `drawArc`，忘了 drawArc 的角度是 **1/16°、0°=3 点、正值逆时针**——和 β（顺时针为正）差一个 y 翻转
- drawArc 要用 **负扫角**：`drawArc(rect, 225*16, -270*16)`（= kArcStart16/kArcSpan16），负扫角 = 顺时针铺开。→ `src/speed_meter.cpp:207`
- 角度换算注释在 `.cpp` 顶部，对照着看 → `src/speed_meter.cpp:23-36`
- 进阶排查：[QPainter](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)、[坐标变换](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)

## 指针指反端 / 刻度上下镜像（value=0 指到了 max 那头）

- 根因是两套角度约定混了：cos/sin 在屏幕 y 朝下算位置，`drawArc` 用 y 朝上逆时针——差一个 y 翻转。若再给 rotate 叠一个 +90°「修正」，两者叠加就把刻度/指针整体上下镜像，v=0 的针怼到 max 位置
- 解法：全控件统一用屏幕角 β（3 点为 0°、顺时针为正，cos/sin/rotate 同套），`rotate(needle_angle_)` 直接转不修正；只有 drawArc 单独换算（225*16、-270*16）。→ 映射 `src/speed_meter.cpp:64-68`、rotate `src/speed_meter.cpp:272`（注释 `:268-269`）
- 验证：value=0 → rotate 135° 指左下，value=max → rotate 45° 指右下

## 指针不转 / 直接跳（没有平滑过渡）

- `needleAngle` **声明成 Q_PROPERTY 了吗**？属性名和 `QPropertyAnimation(this, "needleAngle")` 一字不差？moc 没认到属性，动画驱动了个空名字。→ `include/speed_meter.h:31`
- setValue 里**真的 stop + setStart + setEnd + start 了吗**？漏 start 不动，漏 stop 旧动画会和新的叠。→ `src/speed_meter.cpp:82-85`
- setNeedleAngle 里**有 `update()` 吗**？没 update 就不重绘。→ `src/speed_meter.cpp:95-101`
- 动画对象的 **parent 是 this 吗**？否则对象树不管它。→ `src/speed_meter.cpp:56`
- 进阶排查：[属性动画框架基础](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)

## Slider 连拖 / 狂点时指针闪回旧目标再出发

- 动画 `setStartValue` 取的是**上一次的目标角度**还是**当前显示角度（needle_angle_）**？取旧目标会先跳回去再出发
- 必须 `setStartValue(needle_angle_)`（当前显示值，可能是上一段动画的中间值），每次先 `stop()`。→ `src/speed_meter.cpp:82-83`
- 进阶排查：[动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)

## 快速反复 setValue 时崩溃（栈溢出）

- needleAngle 的 **WRITE 是不是错指向了 `setValue`**？动画每帧驱动 setValue → setValue 又启动画 → 无限递归栈溢出
- WRITE 必须指 `setNeedleAngle`（纯赋值+emit+update），setValue 只做业务入口算映射。→ `include/speed_meter.h:31` + `src/speed_meter.cpp:95`

## 频繁切换 / 连切时崩溃（segfault）

- needle_anim_ **用了 `DeleteWhenStopped` 吗**？stop 时对象被 delete、成员指针悬空。改成持久成员指针 + `stop()/重配/start()`。→ `src/speed_meter.cpp:54-56`
- 动画对象 **parent 设成 this 了吗**？否则对象树不管它、可能提前释放。→ `src/speed_meter.cpp:56`

## 窗口缩到极小时弧 / 指针 / 轴帽消失或乱画

- 半径（`gauge_r`、`needle_len` 等）在控件极小时**可能 ≤0**，`drawArc`/`drawEllipse`/`drawPolygon` 对负尺寸行为未定义
- 所有半径 `std::max(1.0, ...)`、`side = std::max(1, min(w,h))` 兜底。→ `src/speed_meter.cpp:188-195`

## demo 编译报 QRandomGenerator 隐式声明 / 未定义

- `demo/speed_meter_window.cpp` 用了 `QRandomGenerator::global()->bounded()` 但**没 include 它的头**。include 块加 `<QRandomGenerator>`。→ `demo/speed_meter_window.cpp:13`

## moc 报错（Q_PROPERTY 不认识）

- 头文件**有 `Q_OBJECT` 吗**？→ `include/speed_meter.h:27`
- CMake **开了 AUTOMOC 吗**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

---

实在卡死，成品 `widget/speed-meter/src/speed_meter.cpp` 就是答案——但先自己拼。
