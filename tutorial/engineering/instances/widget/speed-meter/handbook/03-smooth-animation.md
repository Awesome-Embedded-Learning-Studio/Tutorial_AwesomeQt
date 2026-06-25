---
title: "Step 3：平滑旋转动画（value/needleAngle 解耦——核心）"
description: "把 step 2 的突变指针升级成 400ms 平滑旋转：Q_PROPERTY 暴露 needleAngle，QPropertyAnimation 每帧驱动，value 业务属性与 needleAngle 动画属性解耦，持久指针 + 接力防连切跳变与悬空。"
---

# Step 3：平滑旋转动画（value/needleAngle 解耦——核心）

← [Step 2](./02-needle-and-mapping.md) · [手册首页](./index.md) →

这步是整个控件的核心——把 step 2 的「突变指针」升级成「400ms 平滑旋转」。诀窍和 status-led / toggle-switch 一脉相承：**把「当前指针角度」做成一个 Q_PROPERTY，让 QPropertyAnimation 每帧去写它**，而 value 只当业务入口。

## 目标

加 `Q_PROPERTY(qreal needleAngle ...)`，setValue 时启动一个 `QPropertyAnimation` 把 needleAngle 从当前角度接力到 `angleForValue(新value)`，指针平滑旋转。Slider 连拖 / 狂点不跳变、不崩。

## 提示（按顺序）

1. **把 needleAngle 声明成 Q_PROPERTY**：`Q_PROPERTY(qreal needleAngle READ needleAngle WRITE setNeedleAngle NOTIFY needleAngleChanged)`，补 `needleAngleChanged` 信号
2. **`setNeedleAngle(qreal)` 作为 WRITE 回调**——这是动画每帧调的：纯赋值 `needle_angle_ = angle` + `emit needleAngleChanged` + `update()`。**只赋值，不启动画**。用 `qFuzzyCompare` 去重防抖（相邻帧角度几乎相同就不重绘）
3. **改 setValue**：不再直接设 `needle_angle_`，而是
   - `needle_anim_->stop()`
   - `setStartValue(needle_angle_)`（从**当前显示角度**接力，不是从上次的目标角度）
   - `setEndValue(angleForValue(value))`
   - `start()`
4. **needle_anim_ 用持久成员指针**：构造时 `new QPropertyAnimation(this, "needleAngle", this)`（parent=this 托管），配 `setDuration(400)` + `setEasingCurve(QEasingCurve::OutCubic)`，**不要用 `DeleteWhenStopped`**——频繁切换时指针悬空崩
5. value 也做成 Q_PROPERTY（READ/WRITE/NOTIFY），但 value 的 WRITE 指 setValue（业务入口）；needleAngle 的 WRITE 指 setNeedleAngle（纯赋值）——别搞反

## 关键认知

- **value 与 needleAngle 解耦的根因**：如果 needleAngle 的 WRITE 指向 setValue，动画每帧驱动 setValue → setValue 又启动画 → 无限递归栈溢出。所以 WRITE 必须指那个**纯赋值的回调**（setNeedleAngle），setValue 只是业务入口算映射后启动画。这套和 status-led 的 setAnimatedColor 同构，[troubleshooting](./troubleshooting.md) 有专门一节讲这个崩溃。
- **从 needle_angle_ 接力**而非从目标角度重启——快速连切（Slider 拖、Random Jump 狂点）时指针从它此刻停的位置直接转到新目标，不会先闪回旧目标。这就是 demo 里 Random Jump 狂点看着顺滑的原因。
- **持久指针 vs DeleteWhenStopped**：DeleteWhenStopped 在 stop 时 delete 对象，成员指针就悬空了；持久指针 + stop()/重配/start() 复用同一对象，没有 new/delete 抖动、不悬空。

## 检查点

- `setValue(180)` 后指针 **400ms 平滑旋转**到 180（不是瞬移）= 动画对了
- 用 QSlider 连续拖：指针连贯追踪、不卡顿不跳变 = 接力逻辑对了
- Random Jump 狂点：指针平滑改向、不闪回旧目标 = setStartValue 取当前显示值对了
- 快速反复 setValue 不崩 = 持久指针对了（没用 DeleteWhenStopped）

搓到这，SpeedMeter 主体就齐了。想再深一层（指针危险区渐红 / 高转速刻度分段配色 / 过冲回弹）回 [手册首页](./index.md) 看进阶挑战。

> 动画框架不熟？[属性动画框架基础](../../../../../beginner/03-qtwidgets/09-animation-framework-beginner.md)、进阶 [动画框架进阶](../../../../../advanced/03-qtwidgets/09-advanced.md)。Q_PROPERTY 机制不熟？[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)、进阶 [属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

## 对照答案

- needleAngle 的 Q_PROPERTY（WRITE 指 setNeedleAngle）：`include/speed_meter.h:31`
- needle_anim_ 初始化（持久指针 + parent=this + duration/easing）：`src/speed_meter.cpp:54-58`
- setValue 启动过渡（stop/setStart(needle_angle_)/setEnd/start）：`src/speed_meter.cpp:74-86`
- setNeedleAngle 回调（qFuzzyCompare 去重 + 赋值 + emit + update）：`src/speed_meter.cpp:95-101`

---

搓完了。跑 demo 对照成品：静态几档 + Cycle 接力 + Slider 追踪 + Random Jump 狂点都能复现 = 你搓的和 repo 一致。完整 demo 见 `demo/speed_meter_window.cpp` 四组布局。
