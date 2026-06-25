---
title: "卡住怎么办"
description: "按症状查：淡入前先闪全图、连发崩溃、opacity 滑块与动画互相抖、duration=0 动画不动、moc 报错——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/fade-animation/`，对照着看。

## 淡入时控件先全显示，再从 0 淡入（视觉跳变）

- fadeIn 里**有没有在不可见时先 `effect_->setOpacity(0.0)` 再 `show()`**？effect 初值是 1.0，show 会先把全不透明画面渲染出来。→ `src/fade_animation.cpp:33-37`
- 进阶排查：[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)（show 与可见性）

## 连点 Fade In / Fade Out 多次后崩溃（segfault）

- fade_anim_ **是不是用了 `DeleteWhenStopped`**？stop 时对象被 delete，下次用就悬空。改成持久成员指针 + `stop()/重配/start()`。→ `src/fade_animation.cpp:21`、`src/fade_animation.cpp:92-96`
- 动画对象的 **parent 是不是设成了 this**？否则对象树不管它，可能提前释放。→ `src/fade_animation.cpp:21`
- 进阶排查：[动画框架进阶](../../../../../advanced/03-qtwidgets/09-animation-advanced.md)

## demo 里 opacity 滑块被动画拖动时，与用户操作互相抖 / 回灌

- 反向同步 `opacityChanged` → setValue 滑块时**有没有用 `QSignalBlocker`**？没屏蔽会触发 valueChanged→setOpacity→opacityChanged 死循环。→ `demo/fade_animation_window.cpp:129-137`
- 是不是忘了 `if (slider->value() != pct)` 判断？重复写同值也容易引发抖动。→ `demo/fade_animation_window.cpp:132-136`
- 进阶排查：[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

## duration 传 0 时动画不动

- QPropertyAnimation **时长为 0 不启动**。在 fadeIn / fadeOut / setFadeDuration / runFade 里都做 `<1 兜底成 1`。→ `src/fade_animation.cpp:52-54`、`src/fade_animation.cpp:88-90`

## 以为调 setOpacity 会触发淡入淡出

- 这不是 bug，是设计：`setOpacity` 是 Q_PROPERTY 的 WRITE 回调，**只做纯赋值 + clamp + emit，不调动画**。动画由 fadeIn/fadeOut 经 runFade 启动。→ `src/fade_animation.cpp:70-81`
- 若你把动画逻辑写进 setOpacity，外部滑块拖一下就会触发动画——这正是要避免的。把它拆出去。
- 进阶排查：[属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)

## moc 报错（Q_PROPERTY 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/fade_animation.h:29`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- Q_PROPERTY 的 qreal 类型 moc 能直接识别，不用额外声明；若换了自定义类型才需要 `Q_DECLARE_METATYPE`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
