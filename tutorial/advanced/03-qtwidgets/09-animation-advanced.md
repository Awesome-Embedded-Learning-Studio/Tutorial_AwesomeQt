---
title: "3.9 动画进阶"
description: "入门篇我们学会了 QPropertyAnimation 做属性动画和 QEasingCurve 做缓动。但到了产品级动画——状态切换、并行组合、自定义插值——这些基础操作就不够了。"
---

# 现代Qt开发教程（进阶篇）3.9——动画进阶

## 1. 前言 / 为什么入门篇的动画还不够用

入门篇我们学会了 `QPropertyAnimation` 驱动单个属性从 A 变到 B，也学会了用动画组做串行/并行编排。但如果你做过正式一点的界面——比如登录页，输入框聚焦时放大、按钮点击时旋转、密码错误时表单抖动——你会发现光靠"手动 new 动画然后 start"的模式，代码很快变成意大利面。问题不在动画本身，而在状态管理：哪种状态下播哪组动画、动画没播完用户又点了按钮怎么办，这些靠信号槽硬连不但容易漏，而且后续维护简直是灾难。

这一篇我们用 `QStateMachine` 驱动动画、用动画组编排时序、用自定义 `QVariantAnimation` 做非线性插值。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。动画框架在 QtCore 中，状态机需要额外链接 `Qt6::StateMachine` 模块。CMake 中添加 `find_package(Qt6 REQUIRED COMPONENTS StateMachine)` 并在 `target_link_libraries` 中加入 `Qt6::StateMachine` 即可。

## 3. 核心概念讲解

### 3.1 QStateMachine + QPropertyAnimation——状态驱动动画

`QStateMachine` 把界面的每种视觉状态建模为一个 `QState`，状态之间的切换通过 `addTransition()` 定义触发条件。每个 `QState` 可以通过 `assignProperty()` 设置目标对象在进入该状态时的属性值，也可以通过 `addAnimation()` 关联 `QPropertyAnimation`，让属性值平滑过渡而不是瞬间跳变。

我们用一个三状态的例子说明：Normal（宽度 200）-> Focused（宽度 260，平滑放大）-> Error（回到 200）。

```cpp
auto *machine = new QStateMachine(this);
auto *s_normal  = new QState();
auto *s_focused = new QState();
auto *s_error   = new QState();

// Focused 态关联动画 + 属性赋值
auto *grow_anim = new QPropertyAnimation(input_edit, "geometry");
grow_anim->setDuration(200);
grow_anim->setEasingCurve(QEasingCurve::OutCubic);
s_focused->addAnimation(grow_anim);
s_focused->assignProperty(input_edit, "geometry", focused_rect);

// 信号驱动状态转换
s_normal->addTransition(input_edit, &QLineEdit::cursorPositionChanged, s_focused);
s_focused->addTransition(input_edit, &QLineEdit::editingFinished, s_normal);
s_focused->addTransition(this, &LoginWidget::loginFailed, s_error);

machine->addState(s_normal);
machine->addState(s_focused);
machine->addState(s_error);
machine->setInitialState(s_normal);
machine->start();
```

你会发现关键设计在于：状态转换条件（哪个信号触发切换）和动画效果（切换过程中怎么过渡）是解耦的。`addTransition` 只管"什么时候切"，`addAnimation` 只管"切的时候怎么动"。`assignProperty` 则确保进入某个状态时属性值被正确设置——即使动画被中途打断，状态机也会把属性强制设为目标值。

### 3.2 动画组的编排——QParallelAnimationGroup 和 QSequentialAnimationGroup

入门篇我们介绍了这两种动画组的基本用法，这里补充工程实践中真正需要注意的时序协调问题。

`QParallelAnimationGroup` 让所有子动画同时开始，总时长等于子动画中最长的 duration。短的那个播完后停在 endValue，长的继续播。如果你在并行组中混入了不同 duration 的动画，视觉上就会出现"节奏不统一"——比如一个 200ms 的位移动画播完了，一个 600ms 的透明度动画还在继续，控件已经到位了但还在慢慢变透明，看起来像卡了一帧。

解决方法有两种：统一并行组中所有动画的 duration，差别只体现在缓动曲线上；或者把不同时长的动画拆分到串行组中嵌套编排，让每个并行组内部的时间轴一致。

```cpp
auto *seq = new QSequentialAnimationGroup(this);
auto *par1 = new QParallelAnimationGroup();
// 并行阶段：位移 + 透明度，duration 统一 300ms
auto *move = new QPropertyAnimation(widget, "pos");
move->setDuration(300);
move->setEndValue(QPoint(200, 100));
auto *fade = new QPropertyAnimation(widget, "windowOpacity");
fade->setDuration(300);
fade->setEndValue(1.0);
par1->addAnimation(move);
par1->addAnimation(fade);
seq->addAnimation(par1);
// 串行阶段：缩放
auto *scale = new QPropertyAnimation(widget, "geometry");
scale->setDuration(200);
scale->setEndValue(target_rect);
seq->addAnimation(scale);
seq->start(QAbstractAnimation::DeleteWhenStopped);
```

`QSequentialAnimationGroup` 的 `addPause(int msecs)` 也很实用——在两个动画之间插入静默间隔让视觉上有呼吸感。

### 3.3 自定义 QVariantAnimation——updateCurrentValue 实现非线性插值

`QPropertyAnimation` 的插值逻辑是固定的：根据 `QEasingCurve` 映射时间进度，然后在 startValue 和 endValue 之间做线性插值。但有些场景你需要"按自定义路径从 A 到 B"——比如让控件沿贝塞尔曲线运动，或者颜色按 HSL 空间渐变。这时候需要继承 `QVariantAnimation`，重写 `interpolated()` 虚函数来完全控制"进度 progress 时值应该是多少"。`updateCurrentValue` 在每一帧被调用，拿到插值结果后你可以手动设置到目标对象上。

```cpp
class CurvePathAnimation : public QVariantAnimation {
    Q_OBJECT
public:
    void setPath(const QPainterPath &path) { m_path = path; }
protected:
    QVariant interpolated(const QVariant &from, const QVariant &to,
                          qreal progress) const override {
        return m_path.pointAtPercent(
            QEasingCurve::OutCubic.valueForProgress(progress));
    }
    void updateCurrentValue(const QVariant &value) override {
        if (value.canConvert<QPointF>())
            emit positionChanged(value.toPointF());
    }
signals:
    void positionChanged(const QPointF &pos);
private:
    QPainterPath m_path;
};
```

这个例子用 `QPainterPath::pointAtPercent()` 让控件沿任意曲线路径运动，progress 经过 `OutCubic` 缓动后再去路径上取点，所以移动速度也是"快启慢停"的节奏。

现在有一道调试题给大家。下面这段代码运行之后动画没有任何效果，控件纹丝不动，为什么？

```cpp
class MyWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
public:
    qreal scale() const { return m_scale; }
    void setScale(qreal s) { m_scale = s; }
private:
    qreal m_scale = 1.0;
};
// 外部
auto *anim = new QPropertyAnimation(widget, "scale");
anim->setDuration(500);
anim->setStartValue(1.0);
anim->setEndValue(2.0);
anim->start(QAbstractAnimation::DeleteWhenStopped);
```

问题出在两个地方。第一，`Q_PROPERTY` 声明缺少 `NOTIFY` 信号——缺少它会导致属性系统在某些路径下无法正确识别这个属性为"可动画的"。第二，`setScale` 里没有调用 `update()`，即使 scale 值在变化控件也不会重绘。正确做法是声明 NOTIFY 信号，在 WRITE 函数里 emit 它，并在 `paintEvent` 中根据 m_scale 做绘制变换。

### 3.4 Widgets 和 QML 混合界面中的动画协调

如果你在项目中混合使用了 QtWidgets 和 QML（比如用 `QQuickWidget` 嵌入 QML 界面），动画协调会变成一个让人头疼的问题。核心矛盾在于两者使用了不同的渲染管线：Widgets 的动画依赖 `QPropertyAnimation` + 定时器回调，QML 的动画依赖声明式的 `PropertyAnimation` + 场景图合成。两侧的动画帧不一定是同一时刻触发的，即使 duration 相同也会有时序不同步的问题。

工程上可行的协调策略是把动画的主控制器放在一侧——以 Widgets 为主就用 `QPropertyAnimation` 做主控，通过信号通知 QML 侧启动对应动画，反过来也一样。关键是不要让两侧各自独立驱动，而是让一侧作为"时钟源"，另一侧跟随。如果确实需要精确同步，可以用 `QElapsedTimer` 做共享时间基准。

## 4. 踩坑预防

第一个坑是 `Q_PROPERTY` 没有 NOTIFY 信号导致动画看起来没效果。这个坑前面调试题里已经讲过了——后果是动画确实在驱动属性值变化，但控件表面毫无反应。解决方案：所有需要做动画的自定义属性，必须声明 NOTIFY 信号，并且在 WRITE 函数中 emit 它。

第二个坑是并行动画组中子动画 duration 差异导致视觉节奏错乱。并行组中短动画提前结束，控件"到位"后长时间静止等待长动画完成，看起来像卡顿。这个后果不是功能错误而是体验很差，而且很难在代码审查中发现——只有跑起来才能看到。解决方案：并行组中的动画尽量统一 duration，或者用嵌套串行组把不同时长的阶段隔开。

第三个坑是状态机 transition 中动画未结束就切换状态。用户快速操作可能在一个状态的入场动画还没播完时就触发了新的 transition，属性当前值（可能还在中间态）被新状态的 `assignProperty` 覆盖，导致视觉瞬间跳变。解决方案是给需要"不可打断"的 transition 通过 `QAbstractTransition::addAnimation()` 关联动画——状态机会等待动画播完后再执行切换。或者在 UI 层面禁用按钮直到动画结束。

## 5. 练习项目

练习项目：带状态机的登录界面动画。实现一个登录窗口，包含用户名输入框、密码输入框和登录按钮。输入框聚焦时缓缓放大（宽度 200->260），失去焦点时缩回。点击登录按钮后按钮旋转 360 度并显示加载动画。登录失败时整个表单水平抖动。

完成标准是状态切换流畅无闪烁，快速点击不导致动画叠加或属性跳变，抖动结束后自动回正常态。提示几个关键点：用 `QStateMachine` 建模 Normal / Focused / LoggingIn / Error 四个状态，Error 态用自定义 `QVariantAnimation` 做水平偏移往复抖动（在 `updateCurrentValue` 中用正弦函数计算偏移量）。

## 6. 官方文档参考链接

[Qt 文档 · QStateMachine](https://doc.qt.io/qt-6/qstatemachine.html) -- 分层有限状态机，状态图的核心驱动类

[Qt 文档 · QState](https://doc.qt.io/qt-6/qstate.html) -- 状态类，addTransition / assignProperty / addAnimation 接口

[Qt 文档 · QVariantAnimation](https://doc.qt.io/qt-6/qvariantanimation.html) -- 动画基类，interpolated 虚函数用于自定义插值

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画类，核心 API 说明

[Qt 文档 · QParallelAnimationGroup](https://doc.qt.io/qt-6/qparallelanimationgroup.html) -- 并行动画组

[Qt 文档 · QSequentialAnimationGroup](https://doc.qt.io/qt-6/qsequentialanimationgroup.html) -- 串行动画组

[Qt 文档 · The Animation Framework](https://doc.qt.io/qt-6/animation-overview.html) -- 动画框架总览，包含状态机与动画结合的架构说明

---

到这里，动画进阶的内容就过了一遍。`QStateMachine` + `QPropertyAnimation` 的组合把"什么时候播动画"和"播什么动画"彻底解耦了，动画组的嵌套编排让复杂时序变得可控，自定义 `QVariantAnimation` 子类则打开了任意插值路径的大门。产品级动画的核心难点从来不在单个动画怎么写，而在多个动画之间怎么协调、怎么管理状态切换时的边界情况——这些才是真正拉开差距的地方。下一篇我们来看 Qt 的多文档界面（MDI）。
