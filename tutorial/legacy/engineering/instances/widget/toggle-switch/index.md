---
title: ToggleSwitch 成品导览
description: AwesomeQt::ToggleSwitch 自绘滑动开关——点击/拖动切换，滑块滑动 + 轨道变色过渡
---

# ToggleSwitch 成品导览

> **source**：成品代码在 `widget/toggle-switch/` · **related**：[自定义控件绘制入门](../../../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)、[动画框架入门](../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)

## 1. 它做什么

一个自绘的滑动开关（iOS 风格 toggle）：点一下或拖一下，滑块从左滑到右、轨道颜色从灰过渡到绿（或你设的任意色）。比 QCheckBox 直观，做设置页的开关项特别合适。

demo（`widget/build/toggle-switch/demo/toggle_switch_demo`）三组：基本开/关静态展示、交互（点击/拖动 + 程序化 setChecked 按钮 + 状态标签）、自定义配色（蓝/红/紫）。

## 2. 架构总览

一个控件类 + 一个演示窗口，跟 status-led 同一套结构：

| 文件 | 职责 |
|---|---|
| `widget/toggle-switch/include/toggle_switch.h` | 控件声明：`AwesomeQt::ToggleSwitch`、4 个 Q_PROPERTY、拖动/点击事件 |
| `widget/toggle-switch/src/toggle_switch.cpp` | 实现：`paintEvent` 自绘 + `QPropertyAnimation` 滑动 + 鼠标交互 |
| `widget/toggle-switch/demo/toggle_switch_window.{h,cpp}` | 演示窗口：三组布局 |
| `widget/toggle-switch/demo/main.cpp` | 程序入口 |

`ToggleSwitch` 直接继承 `QWidget`（`toggle_switch.h:18`），平铺一个类，交互全靠重写三个鼠标事件 + 一个动画属性。

## 3. 关键设计决策

1. **动画属性是 `handlePos`，不是 `checked`**。`checked`（`toggle_switch.h:21`）是逻辑状态，`handlePos`（`toggle_switch.h:22`，0=关 1=开）才是 `QPropertyAnimation` 每帧驱动的那个。`setChecked` 触发动画、动画驱动 `setHandlePos`、`paintEvent` 按 `handlePos` 画——职责分清，`checked` 不会被中间帧污染。

2. **轨道颜色由 `handlePos` 现算，不另存动画色**。`paintEvent` 里 `blend(track_off_, track_on_, handle_pos_)` 实时插值——滑块滑到一半，轨道正好半灰半绿，过渡和滑块完全同步。比给颜色也开一个动画属性省事，且不会出现「颜色到了、滑块还没到」的错位。

3. **点击和拖动用移动阈值区分**。按下不算切换，移动超过 `kDragThreshold`（4px，`toggle_switch.cpp:11`）才算拖动；松手时拖过就按位置吸附、没拖就当点击切换（`toggle_switch.cpp:156`）。这样点一下不会被手抖当成拖，拖一半松手也能正确归位。

4. **配色走 Q_PROPERTY，可被 Designer / 外部改**。`trackColorOn` / `trackColorOff`（`toggle_switch.h:23-24`）是真属性，demo 里蓝/红/紫三个开关就是 `setTrackColorOn` 设出来的。想换主题色不用改控件源码。

5. **半径/行程都从控件尺寸现算**。`handle_range()`（`toggle_switch.cpp:91`）按轨道宽高算滑块行程，`sizeHint` 给默认 52×28、`minimumSizeHint` 36×20。控件被布局缩放时滑块自动跟着比例走，不写死像素。

## 4. 怎么读这份 code

1. 先 `toggle_switch.h`（18 行起）——看清「checked / handlePos 两个属性 + toggled 信号 + 三个鼠标事件」这张公共面孔。
2. 再 `toggle_switch.cpp` 的 `paintEvent`（106 行起）——控件灵魂，看轨道圆角 + 颜色插值 + 滑块定位怎么从 `handlePos` 推出来。
3. 然后 `animate_to`（40 行）+ `setChecked`（49 行）——状态切换怎么触发动画。
4. 最后三个 `mouse*Event`（128/139/156 行）——点击/拖动/松手的交互状态机，这是最容易踩坑的部分。

入口：`demo/main.cpp` → `ToggleSwitchWindow` → 三组布局。

## 5. 踩坑

这几个坑都是实现这个控件时真处理过的，代码里能逐条对上。

**坑 1：点击被当成拖动，开关乱跳**
按下后只要鼠标稍微动一点（哪怕手抖 1-2 像素），不设阈值就会被判成拖动，松手时归位逻辑和点击切换逻辑打架，开关状态乱。后果是用户明明只点一下，开关却不动或反跳。解法是设 `kDragThreshold`（`toggle_switch.cpp:11`），移动没超过 4px 一律当点击（`toggle_switch.cpp:146`），超了才进拖动模式。

**坑 2：拖动时没停动画，滑块跟手抢位置**
如果拖动开始不 `stop()` 滑动动画，你手往左拖、动画还在往右走，两个一起改 `handlePos`，滑块来回抽搐。后果是拖动手感完全错乱。解法是进入拖动模式那一刻先 `handle_anim_->stop()`（`toggle_switch.cpp:148`），让拖动独占 `handlePos` 的控制权。

**坑 3：控件被布局压太窄，滑块反向乱跑**
`handle_range` 是「轨道宽 - 留白 - 滑块直径」。控件太窄时这个值会变 0 甚至负数，滑块位置 `handle_pos_ * range` 算出负坐标或反向，滑块飞出轨道或左右颠倒。后果是小尺寸下控件完全错乱。解法是 `range <= 0` 时直接保持原位（`toggle_switch.cpp:102`），绘制时也把负行程夹成 0（`toggle_switch.cpp:122`），宁可滑块不动也别乱跑。

**坑 4：快速反复 setChecked，动画叠在一起**
连续调 `setChecked` 时，如果上一段动画没结束就启动新的，两段动画抢同一个属性，滑块抖。后果是快速点击时滑块一顿一顿。解法是 `animate_to` 里每次先 `handle_anim_->stop()`（`toggle_switch.cpp:41`）再设新起止值启动，保证任一时刻只有一段动画在跑。

## 6. 官方文档链接

- [QWidget（自绘控件基类）](https://doc.qt.io/qt-6/qwidget.html)
- [QPainter（绘图引擎）](https://doc.qt.io/qt-6/qpainter.html)
- [QPropertyAnimation（属性动画，驱动滑块滑动）](https://doc.qt.io/qt-6/qpropertyanimation.html)
- [The Property System（Q_PROPERTY）](https://doc.qt.io/qt-6/properties.html)
- [QMouseEvent（鼠标交互）](https://doc.qt.io/qt-6/qmouseevent.html)

---

想跟着自己搓一个？看 [手搓手册](./handbook/)。成品就是你的答案钥匙。
