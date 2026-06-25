---
title: "卡住怎么办"
description: "按症状查：弧方向画反、指针偏 90°、指针不转/直接跳、连切崩溃、控件压扁消失、moc 报错——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/speed-meter/`，对照着看。

## 背景弧方向画反（逆时针铺 / 开口朝上）

- 把数学角度直接塞进了 `drawArc`，忘了 drawArc 的角度是 **1/16°、0°=3 点、正值逆时针**——和我们「顺时针铺开」的约定相反
- drawArc 要用 **负扫角**：`drawArc(rect, 225*16, -270*16)`，负扫角 = 顺时针铺开。→ `src/speed_meter.cpp:201-202`
- 角度体系换算注释在 `.cpp` 顶部，对照着看 → `src/speed_meter.cpp:23-27`、`src/speed_meter.cpp:189-191`
- 进阶排查：[QPainter](../../../../../beginner/02-qtgui/01-qpainter-basic-beginner.md)、[坐标变换](../../../../../beginner/02-qtgui/02-coordinate-transform-beginner.md)

## 指针整体偏 90°（不指当前值）

- 用了 `painter.rotate(angle)`，但 rotate 的基准是 **12 点钟、顺时针为正**，而 `needle_angle_` 是数学角度（0°=3 点钟），两套「0° 朝向」差 90°
- 旋转角必须 `needle_angle_ + 90`（+90 把数学 0° 摆正到 3 点再适配 rotate 基准）。→ `src/speed_meter.cpp:258`（注释在 `src/speed_meter.cpp:253-255`）
- 验证：value=0 → rotate 315° 指左下，value=max → rotate 45° 指右下

## 指针不转 / 直接跳（没有平滑过渡）

- `needleAngle` **声明成 Q_PROPERTY 了吗**？属性名和 `QPropertyAnimation(this, "needleAngle")` 一字不差？moc 没认到属性，动画驱动了个空名字。→ `include/speed_meter.h:31`
- setValue 里**真的 stop + setStart + setEnd + start 了吗**？漏 start 不动，漏 stop 旧动画会和新的叠。→ `src/speed_meter.cpp:74-77`
- setNeedleAngle 里**有 `update()` 吗**？没 update 就不重绘。→ `src/speed_meter.cpp:93`
- 动画对象的 **parent 是 this 吗**？否则对象树不管它。→ `src/speed_meter.cpp:48`
- 进阶排查：[属性动画框架基础](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)

## Slider 连拖 / 狂点时指针闪回旧目标再出发

- 动画 `setStartValue` 取的是**上一次的目标角度**还是**当前显示角度（needle_angle_）**？取旧目标会先跳回去再出发
- 必须 `setStartValue(needle_angle_)`（当前显示值，可能是上一段动画的中间值），每次先 `stop()`。→ `src/speed_meter.cpp:74-75`
- 进阶排查：[动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)

## 快速反复 setValue 时崩溃（栈溢出）

- needleAngle 的 **WRITE 是不是错指向了 `setValue`**？动画每帧驱动 setValue → setValue 又启动画 → 无限递归栈溢出
- WRITE 必须指 `setNeedleAngle`（纯赋值+emit+update），setValue 只做业务入口算映射。→ `include/speed_meter.h:31` + `src/speed_meter.cpp:87`

## 频繁切换 / 连切时崩溃（segfault）

- needle_anim_ **用了 `DeleteWhenStopped` 吗**？stop 时对象被 delete、成员指针悬空。改成持久成员指针 + `stop()/重配/start()`。→ `src/speed_meter.cpp:46-48`
- 动画对象 **parent 设成 this 了吗**？否则对象树不管它、可能提前释放。→ `src/speed_meter.cpp:48`

## 窗口缩到极小时弧 / 指针 / 轴帽消失或乱画

- 半径（`gauge_r`、`needle_len` 等）在控件极小时**可能 ≤0**，`drawArc`/`drawEllipse`/`drawPolygon` 对负尺寸行为未定义
- 所有半径 `std::max(1.0, ...)`、`side = std::max(1, min(w,h))` 兜底。→ `src/speed_meter.cpp:180-187`

## demo 编译报 QRandomGenerator 隐式声明 / 未定义

- `demo/speed_meter_window.cpp` 用了 `QRandomGenerator::global()->bounded()` 但**没 include 它的头**。include 块加 `<QRandomGenerator>`。→ `demo/speed_meter_window.cpp:13`

## moc 报错（Q_PROPERTY 不认识）

- 头文件**有 `Q_OBJECT` 吗**？→ `include/speed_meter.h:27`
- CMake **开了 AUTOMOC 吗**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

---

实在卡死，成品 `widget/speed-meter/src/speed_meter.cpp` 就是答案——但先自己拼。
