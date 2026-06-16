---
title: 卡住怎么办
description: ToggleSwitch 手搓常见错——动画不滑、滑块乱跑、拖动打架——指向成品，不给完整答案
---

# 卡住怎么办

按症状找，只指方向。答案在成品 `widget/toggle-switch/`。

## 动画根本不滑 / 滑块不动

`handlePos` 没声明成 Q_PROPERTY，或 `QPropertyAnimation(this, "handlePos")` 的属性名和 Q_PROPERTY 名对不上。moc 没认到属性，动画驱动了个空名字。对照 `toggle_switch.h:22`，属性名一字不差。

## 颜色和滑块错位（颜色先变完，滑块还没到）

给颜色和位置分别开了两个动画，时长/起止对不齐。别开两个——`paintEvent` 里直接按 handlePos 插值颜色，一个属性驱动一切。对照 `toggle_switch.cpp:111`。

## 快速点击滑块抖

启动新动画前没 stop 旧的，两段动画叠在 handlePos 上。`animate_to` 里先 stop 再 start。对照 `toggle_switch.cpp:41`。

## 拖动时滑块抽搐 / 跟手抢

拖动开始没停动画，动画还在往目标走、你手往回拖，两个一起改 handlePos。进拖动模式时立刻 stop 动画。对照 `toggle_switch.cpp:148`。

## 点一下被当成拖动，状态乱

没设移动阈值，手抖 1-2 像素就被判拖动。设个 `kDragThreshold`（4px），没超过就当点击。对照 `toggle_switch.cpp:11,146`。

## 控件压窄后滑块飞出 / 反向

滑块行程 = 轨道宽 - 留白 - 滑块直径，太窄会变 0 或负，乘 handlePos 算出负坐标。`range <= 0` 时保持原位、绘制时夹成 0。对照 `toggle_switch.cpp:102,122`。

## lambda 赋给 `auto*` 编不过（demo 里）

`auto* f = [&]{...};` 错——lambda 是对象不是指针，要写 `auto f = [&]{...};`。这是写 demo 时真踩到的。

---

实在卡死，成品 `widget/toggle-switch/src/toggle_switch.cpp` 就是答案——但先自己拼。
