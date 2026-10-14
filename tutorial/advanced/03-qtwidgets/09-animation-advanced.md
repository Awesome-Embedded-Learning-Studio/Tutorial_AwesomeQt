---
title: "3.9 动画进阶"
description: "手动 new 一个动画再 start，登录页一类的多状态界面撑不住，聚焦放大、按钮旋转、密码输错抖动各自为政，乱象的根子在状态管理。本篇用 QStateMachine 把切换条件和过渡动画分开声明，讲并行动画组的时长纪律与嵌套编排，再用 QVariantAnimation 的两个虚函数做自定义路径插值，顺带查清一例动画在跑控件却不动的毛病。QQuickWidget 嵌 QML 的场合两套动画管线并存，末节讲让一侧当时钟源、另一侧跟随的主从协调。配套示例在 examples/advanced/03-qtwidgets/09-animation-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.9——动画进阶

[入门篇动画框架](../../beginner/03-qtwidgets/09-animation-framework-beginner.md) 把 QPropertyAnimation 驱动属性、QEasingCurve 调缓动过了一遍，串行并行的动画组也摸过。写静态页面，存货够用。可您要是做过正式一点的界面（登录页最典型，输入框聚焦时放大，按钮点击后旋转，密码输错时整个表单抖动），很快会发现“手动 new 动画然后 start”的模式撑不住。代码一行行多起来，哪个动画管聚焦、哪个管报错，过几个月连自己都数不清。咱们把毛病看准一点，单个动画没写错，错在没人管状态。哪种界面状态下该播哪组动画，一组动画没播完用户又点了按钮怎么办，靠信号槽硬连，漏一处就是一次视觉事故。本篇把四件事接上：QStateMachine 驱动状态切换，动画组编排时序，自定义 QVariantAnimation 做非线性插值，最后讲 Widgets 混 QML 的两套动画管线怎么协调。

## QStateMachine：切换条件和过渡动画，分开声明

QStateMachine 把界面的每种视觉状态建成一个 QState，状态之间怎么走由 addTransition() 声明，写清谁的什么信号触发、切到哪个状态。属性怎么落地也有声明的地方，assignProperty() 写明进入状态时把哪个属性设成什么值，动画则挂到 transition 上，addTransition() 会返回 transition 指针，接着对它调 addAnimation() 把 QPropertyAnimation 关联上去。切换触发时动画跟着起步，属性平滑滑到目标值，没挂动画的 transition 就是硬切。三个接口的分工咱们摆成表：

| 接口 | 管什么 | 缺了它会怎样 |
| ---- | ------ | ------------ |
| `addTransition` | 切换条件，谁的信号触发、切到哪个状态 | 状态永远停在原地 |
| transition 上的 `addAnimation` | 切换过程中的过渡动画 | 属性瞬间跳到目标值 |
| `assignProperty` | 进入状态时的属性目标值 | 状态切了，界面没跟上 |

咱们把登录页的三状态写出来（Normal 宽 200，Focused 平滑放大到 260，Error 收回 200）：

```cpp
auto *machine = new QStateMachine(this);
auto *s_normal  = new QState();
auto *s_focused = new QState();
auto *s_error   = new QState();

// Focused 态声明属性目标值
s_focused->assignProperty(input_edit, "geometry", focused_rect);

// 过渡动画挂在 transition 上：Normal -> Focused 平滑放大
auto *grow_anim = new QPropertyAnimation(input_edit, "geometry");
grow_anim->setDuration(200);
grow_anim->setEasingCurve(QEasingCurve::OutCubic);
auto *to_focused =
    s_normal->addTransition(input_edit, &QLineEdit::cursorPositionChanged, s_focused);
to_focused->addAnimation(grow_anim);

// 其余切换只声明触发信号，不带动画就是硬切
s_focused->addTransition(input_edit, &QLineEdit::editingFinished, s_normal);
s_focused->addTransition(this, &LoginWidget::loginFailed, s_error);

machine->addState(s_normal);
machine->addState(s_focused);
machine->addState(s_error);
machine->setInitialState(s_normal);
machine->start();
```

切换条件和过渡效果在这儿是解耦的。咱们想换缓动曲线，改动画那几行，想换触发条件，改 transition 那几行，互不牵连。聚焦放大、错误抖动从前散落在各处的 new 和 start 里，现在摆成一张图上的几份声明。

咱们顺着推一个问题：过渡动画播到一半，用户又触发了新的切换，属性正停在中间值，接下来怎么办。Qt 6.9.1 的源码（qstatemachine.cpp）给的行为分两支。新的切换触发时，旧状态挂着的动画被直接停掉，同一属性要是没有新的赋值声明，状态机把它一次写到目标值，有新的赋值就按新状态来。视觉跳变出在没有新动画接手的场合，值一步跨到位。咱们可能会猜，给 transition 关联了动画，状态机总会等它播完再切吧。实际不会。addAnimation 的语义只是切换触发时把动画 start 起来，切换本身不等人，下一次切换照样能把它打断。真要不可打断，办法退回 UI 层，动画期间禁用触发按钮，或者让新切换也带上动画，从中间值接着动。

配套示例在 `examples/advanced/03-qtwidgets/09-animation-advanced/`，可执行文件叫 09-animation-advanced，窗口类是 StateMachineDemo，`cmake -B build && cmake --build build` 构建后运行，窗口里一个蓝色方块，配“聚焦”“动画”“重置”三个按钮，底部状态标签实时报当前状态名。示例没把第三态做成 Error，做成了 Animated，点“动画”按钮切进去的就是并行动画态，下一节正好拿它当素材。两处对照值得您亲手做。把 buildStateMachine() 里 `tToFocused->addAnimation(focusAnim);` 注释掉重新编译，点“聚焦”，方块瞬间跳到放大位置，属性赋值照样生效，只是没了过渡。恢复后再看“重置”：从聚焦态点它是硬切回原位，因为 Focused 到 Normal 的 transition 没挂动画。从动画态点它则平滑收回，Animated 到 Normal 的 transition 挂了恢复动画。同一个目标状态，两条 transition 两种走法，动画挂在 transition 上，不挂在状态上。

另外您再看三个按钮的可用性，它们也写在各状态的 assignProperty 里，Normal 态“重置”是灰的，Animated 态锁到只剩“重置”能点。状态机管的不止动画属性，一套界面状态快照都可以交给它。

## 并行动画组：时长不齐，节奏就散

入门篇把 QParallelAnimationGroup 和 QSequentialAnimationGroup 的基本用法过了一遍，咱们这里补工程实践里的时序协调。并行组让所有子动画同时起步，组的总时长等于子动画里最长的 duration，短的那路播完停在 endValue，陪长的一起收尾。听着无害，混入不同 duration 就出事：一个 200ms 的位移动画播完了，一个 600ms 的透明度动画还在走，控件位置到位了，透明度还在慢慢变，看起来像卡了一帧。功能上挑不出错，体验上很难受，而且代码审查看不出来，跑起来肉眼才见。

咱们手上有两条治理路线：要么统一并行组内所有动画的 duration，节奏差异交给缓动曲线去表达，要么把不同时长的阶段归进串行组嵌套编排，让每个并行阶段内部的时间轴一致。

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

串行组里有个顺手的工具 addPause(int msecs)，咱们可以用它在两个动画之间垫一段静默，视觉上多一口呼吸。

配套示例的“动画”按钮就是统一时长的写法。state_machine_demo.cpp 里几何、透明度两路动画共用 kAnimateDuration，时长都是 600ms。几何一路把位移和缩放并进同一个 QRect 的起止值，走 OutBounce。透明度一路不能驱动 windowOpacity，那个属性只对顶层窗口生效，方块是舞台里的子控件，淡入淡出得靠挂上的 QGraphicsOpacityEffect，动画驱动它的 opacity，走 InOutQuad。您把 geometryAnim 的 setDuration 改成 200 重新编译再点“动画”，方块 200ms 就弹跳到位，剩下 400ms 只有透明度还在慢慢变淡，节奏当场散掉，改回 600ms 整组又齐了。

## 自定义插值：interpolated() 定路径，updateCurrentValue() 管落地

QPropertyAnimation 的插值逻辑固定，QEasingCurve 把时间进度映射成缓动进度，再在 startValue 和 endValue 之间做线性插值，两点之间只有一条直线。可咱们要让控件沿贝塞尔曲线运动，要让颜色按 HSL 空间渐变，直线插不出来。路子是继承 QVariantAnimation，重写 interpolated() 虚函数，把“进度 progress 时值该是多少”整个接管。updateCurrentValue() 则在每一帧带着插值结果被调（qvariantanimation.cpp 的 tick 路径逐帧调它），咱们在里面把值发出去或设到目标上：

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

progress 先过一遍 OutCubic 再去路径上取点，所以控件沿线移动的速度也是快启慢停的节奏。用法上有一处讲究，起止值仍然要设，您随手给两个同类型的占位值就行。QVariantAnimation 的推进逻辑要见到至少两个关键值才肯走，Qt 6.9.1 的 qvariantanimation.cpp 里，recalculateCurrentInterval 数不够就直接返回，updateCurrentValue 一次都不会被调。例子里重写的 interpolated() 根本不看 from 和 to，占位值只是给推进机器一个起步的理由。

### 一道调试题：动画在跑，控件纹丝不动

咱们来看一段没效果的代码，控件纹丝不动，您先自己找找原因：

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

咱们先别急着怪 Q_PROPERTY 少了 NOTIFY。在 setScale 里加一行 qDebug 或下一个断点，跑起来会看到它每帧都被调，m_scale 确实从 1.0 一路走到 2.0，动画没停工。Qt 6.9.1 的源码可以作证：QPropertyAnimation 认属性只查两样，属性存在、属性可写（qpropertyanimation.cpp 的 updateMetaProperty 就查这两项），逐帧写值走 QMetaObject::metacall 直调 WRITE 函数，全程与 NOTIFY 无关。NOTIFY 缺席在纯 Widgets 里不拦动画，它管的是绑定一侧的变更通知，属性要暴露给 QML 时才是必须。

纹丝不动的病根在另一处：setScale 只改了 m_scale，没调 update()，控件没有理由重绘。就算重绘了，paintEvent() 也没用过 m_scale，画出来的仍是原样。咱们把修法分两步，setScale 里补 update()（NOTIFY 信号等属性要上 QML 或进绑定时再声明），paintEvent 里按 m_scale 做绘制变换，动画这才看得见。

## Widgets 混 QML 的动画：一边当时钟，另一边跟随

项目里用 QQuickWidget 嵌了 QML 界面的话，动画协调会让您头疼。两套渲染管线各走各的：Widgets 的动画是 QPropertyAnimation 加定时器回调，QML 的动画是声明式的 PropertyAnimation 加场景图合成，两侧的动画帧不一定在同一时刻触发，duration 相同也可能时序对不齐。

工程上可行的策略是主从结构，主控制器放一侧。以 Widgets 为主就拿 QPropertyAnimation 做主控，通过信号通知 QML 侧启动对应动画，反过来同理。咱们别让两侧各自独立驱动，让一侧当时钟源，另一侧跟随。确实需要精确同步的场合，用 QElapsedTimer 做共享时间基准。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。动画框架在 QtCore 里，状态机要额外装 StateMachine 模块，`find_package(Qt6 REQUIRED COMPONENTS StateMachine)` 之后，在 `target_link_libraries` 里加上 `Qt6::StateMachine`。

[Qt 文档 · QStateMachine](https://doc.qt.io/qt-6/qstatemachine.html) -- 分层有限状态机，状态图的核心驱动类

[Qt 文档 · QState](https://doc.qt.io/qt-6/qstate.html) -- 状态类，addTransition / assignProperty 接口

[Qt 文档 · QVariantAnimation](https://doc.qt.io/qt-6/qvariantanimation.html) -- 动画基类，interpolated 虚函数用于自定义插值

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画类，核心 API 说明

[Qt 文档 · QParallelAnimationGroup](https://doc.qt.io/qt-6/qparallelanimationgroup.html) -- 并行动画组

[Qt 文档 · QSequentialAnimationGroup](https://doc.qt.io/qt-6/qsequentialanimationgroup.html) -- 串行动画组

[Qt 文档 · The Animation Framework](https://doc.qt.io/qt-6/animation-overview.html) -- 动画框架总览，包含状态机与动画结合的架构说明

---

动画的进阶内容就到这里。想再动手的，请您打开 StateMachineDemo 的 buildStateMachine()，五条 transition 排在一起，挑一条给它挂上或撤掉动画，重新编译看方块怎么走，挂与不挂的差别一趟就试全了。下一篇 [3.10 MDI 进阶](./10-mdi-advanced.md) 讲多文档界面，一个主窗口里铺开多份子文档的那套机制。
