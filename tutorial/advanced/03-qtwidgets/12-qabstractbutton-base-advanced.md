---
title: "3.12 QAbstractButton 基类进阶"
description: "入门篇会调 checkable、autoRepeat 和四个核心信号，进阶篇翻开 qabstractbutton.cpp 看按钮的里子。释放不等于点击的根子在 checked 与 down 两个布尔的分工，三态是 QCheckBox 在基类之上包的一层，autoExclusive 的互斥边界只到同一个 parent，配套示例在 examples/advanced/03-qtwidgets/12-qabstractbutton-base-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.12——QAbstractButton 基类进阶

[入门篇 QAbstractButton 基础](../../beginner/03-qtwidgets/12-qabstractbutton-base-beginner.md) 把 checkable、autoRepeat、四个核心信号、QButtonGroup 互斥管理都过了一遍，写几个按钮控件绰绰有余。控件基类部分走到第二篇，咱们从 QWidget 再往下一层，看 QPushButton、QToolButton、QRadioButton、QCheckBox、QCommandLinkButton 共同的祖先 QAbstractButton。按下弹起、checkable 状态切换、快捷键触发、互斥管理，全是它替这些控件做好的活。平时照着文档调没有问题，可一旦继承它写自定义按钮，入门层见过的表象就开始不够用了，咱们得往下看一层。

笔者把真实项目里最容易把人问住的三类场面摆出来：自定义按钮点击后状态切换正常，外观却纹丝不动。一组 checkable 按钮要自动互斥，又不想引入 QButtonGroup。三态复选框切进切出 PartiallyChecked 时，信号怎么响、isChecked() 报什么，容易判断错。答案分别藏在 QStyle 协作绘制、autoExclusive 机制和 tristate 的包装方式里。本篇把四件事讲明白：checked 与 down 两个布尔怎么分工，QStyleOptionButton 怎么把状态交给 QStyle，三态在 QCheckBox 里怎么实现，autoExclusive 的互斥边界划在哪。

## 释放不等于点击：checked 与 down 的分工

QAbstractButton 内部维护两个独立的状态维度，`checked`（bool）与 `down`（bool）。`checked` 是持久状态，记按钮是否处于选中态。`down` 是瞬时状态，记用户此刻是不是按着不放。按钮的全部视觉表现都从二者的组合来。咱们顺着 qabstractbutton.cpp 把鼠标路径走一遍。

用户按下鼠标，`down` 被设为 true，按钮进入“按下”视觉态，`pressed()` 触发。松开时若指针仍在按钮区域内（hitButton 判定通过），基类把 `down` 清回 false，再对 `checked` 取反，`toggled()`、`released()`、`clicked()` 随之依次发出。指针已经移出按钮区域的话，`down` 同样被清零，`checked` 不变，`clicked()` 不发——入门篇那句“释放不等于点击”的底层就在这里。判定指针在不在按钮内的入口是 `hitButton()` 虚函数，自定义形状的按钮重写它，配套示例就靠它把点击区域限定在圆形里。四个信号的到场时机，咱们列一张表：

| 信号 | 到场时机 |
| -------- | -------- |
| pressed() | 按下生效那一刻，down 同时变 true |
| released() | 松开那一刻，或按住滑出按钮区域时 |
| clicked() | 按下又松开，两次都落在按钮区域内 |
| toggled(bool) | checked 翻转那一刻，参数携带新值 |

程序化调用 `setChecked()` 走的是另一条入口，直接改 `checked` 并触发 `toggled()`，不碰 `down`，也不发 `clicked()`，`checked` 与 `down` 的转换因此能被外部代码拦腰打断。咱们写自定义按钮时想在 mousePressEvent 里加点活，比如播个音效、记一下坐标，逻辑加完忘了调 `QAbstractButton::mousePressEvent(event)`，`down` 就永远不会变 true，pressed、released、clicked、toggled 全体沉默。外观倒可能被自定义绘制画成了按下态，`isChecked()` 却纹丝不动，症状指向很明确：事件被拦了，状态机没被驱动。修法就是把基类调用补上，放在自定义逻辑之前或之后都行。

键盘路径与鼠标在基类里合流。按钮拥有焦点（focusPolicy 设为 Qt::StrongFocus 或 Qt::TabFocus）时按空格，走的是与鼠标按下完全相同的状态机流程：`down` 变 true，`pressed()` 触发，松开空格后 `checked` 取反，`toggled()` 最先到，`released()` 跟上，`clicked()` 收尾。咱们翻 qabstractbutton.cpp 的 QAbstractButtonPrivate::click() 就能核到。`down` 先清零，`nextCheckState()` 接着跑完，`toggled()` 从这里发出，然后才是 `emitReleased()` 和 `emitClicked()`。键盘操作不需要咱们写一行额外逻辑，两条路径已经统一在同一套状态机里。Enter 是例外，QPushButton 对它另有一套 autoDefault 处理，放进 QDialog 后按 Enter 会触发哪个按钮是有讲究的，咱们留到 [3.17 QPushButton 进阶](./17-qpushbutton-advanced.md) 展开。

`down` 还拖着 autoRepeat。开启 autoRepeat 后，基类在 `down` 为 true 期间启动一个内部定时器，按 autoRepeatDelay 与 autoRepeatInterval 的参数持续触发 clicked()，定时器的生命周期完全绑在 `down` 上，用户松开、`down` 变 false，定时器立刻停。反过来，咱们要是重写了 mouseReleaseEvent 又忘调基类，`down` 清不了零，autoRepeat 就停不下来。按下、松开两个事件的重写里，基类调用都不能省。

## 手拼颜色不是正路：QStyleOptionButton 把状态交给 QStyle

入门篇写自定义按钮的 paintEvent，咱们用 isDown()、isChecked()、underMouse() 三个布尔手拼颜色来绘制。简单直接，能出效果，但完全绕过了 QStyle 系统，按钮在不同平台上的外观和行为就不统一了。

正路是把按钮当前状态打包进 QStyleOptionButton，交给 QStyle 绘制。它封装了 QStyle 绘制按钮所需的全部信息：按钮文字、图标、选中状态、按下状态、默认按钮标记。咱们重点认它承载状态的两个字段。`features` 字段可以组合 `None`、`Flat`、`HasMenu`、`DefaultButton`、`AutoDefaultButton`、`CommandLinkButton` 等标记，告诉 QStyle 当前按钮该画成什么风格。`state` 字段继承自 QStyleOption::State，包含 `State_Sunken`（按下）、`State_Raised`（弹起）、`State_On`（选中）、`State_HasFocus`（有焦点）、`State_MouseOver`（悬停）等标记。

```cpp
void paintEvent(QPaintEvent *) override
{
    QStyleOptionButton opt;
    opt.initFrom(this);  // 从 widget 继承 geometry、font、palette、state

    // 根据 QAbstractButton 的状态填充 QStyleOptionButton
    if (isDown()) {
        opt.state |= QStyle::State_Sunken;
    } else {
        opt.state |= QStyle::State_Raised;
    }
    if (isChecked()) {
        opt.state |= QStyle::State_On;
    }
    opt.text = text();
    opt.icon = icon();

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);
}
```

这样绘制，QStyle 会根据当前平台（Fusion、Windows、macOS）选择正确的绘制逻辑，圆角、阴影、动画过渡全部自动处理，indicator 的位置、焦点区域的虚线框、按下时的下沉偏移都不用咱们自己算，QStyle 全包了。只想微调某个子控件的绘制（比如只换 indicator 的图标），可以用 QProxyStyle 只覆写特定子元素，不必把整个 paintEvent 重写。

`features` 字段怎么影响绘制，咱们单独看一遍，常见标记的后果摆成表：

| features 标记 | QStyle 拿到后的绘制行为 |
| -------- | -------- |
| Flat | 跳过立体边框只画文字，QPushButton::setFlat 走的就是它 |
| HasMenu | 按钮右侧多画一个向下的小箭头，提示带下拉菜单 |
| DefaultButton | 多画一圈边框或高亮，视觉上把默认按钮标出来 |

标记不需要咱们自己画任何东西，填充 QStyleOptionButton 时设置正确，QStyle 就会自动处理。有个归属容易认错：Flat 是 QPushButton::setFlat 的机制，qpushbutton.cpp 的 initStyleOption 把 `d->flat` 填进 features。QToolButton 的扁平模式不走它，走的是 QStyleOptionToolButton 的 autoRaise。所以自定义按钮的外观和标准 QPushButton 对不上时，头一个要查的还是 features 和 state 有没有填对。

### 一道调试题：点击前后画出来一个样

咱们看一段能编译能运行、效果却不对的 paintEvent，您先自己找找原因：

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(isChecked() ? QColor("#42A5F5") : QColor("#90CAF9"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect());
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter, text());
}
```

毛病在缺 `isDown()` 的判断。用户按下按钮，按钮进入 down 状态，视觉上应该显示“按下”效果，这段代码只检查了 `isChecked()`，没有处理 `isDown()`。更要紧的是，对 non-checkable 的按钮，`isChecked()` 始终为 false，点击前后画出来的效果完全一样。修法是加入 `isDown()` 的判断分支，或者更正规地走上面 QStyleOptionButton 的路子，把状态交给 QStyle。配套示例 tri_state_button.cpp 的 paintEvent 里就留着按下反馈的三行 if 块，`if (isDown()) bgColor = bgColor.darker(130);`，咱们把这个分支注释掉重新编译：按住按钮，颜色不再变深，松开后状态照样切换。`down` 管瞬时的视觉反馈，`checked` 才是数据，做完实验再回头看两个布尔的分工，各自的分量就摸清了。

## 第三态不在基类里：QCheckBox 在 checked 之上做包装

咱们在基类里找不到 tristate 的概念，`checked` 本身只有 true/false，没有第三态的位置。第三态是 QCheckBox 在基类之上又包的一层，靠 `setTristate(bool)` 开启。开启后状态维度从 bool 升级为 `Qt::CheckState` 枚举，`Unchecked`、`PartiallyChecked`、`Checked` 三个值。

三态切换靠点击走一个循环：从 Unchecked 点一下到 PartiallyChecked，再点到 Checked，再点回 Unchecked，循环只在用户点击时生效。程序化调用 `setCheckState()` 可以直接跳到任意状态，不受循环限制。tristate 关闭时，点击只在 Unchecked 与 Checked 之间翻，开了才进三态循环，这就是 `setTristate(true)` 必须显式调用的原因。咱们要是只调了 `setCheckable(true)`（QCheckBox 本来就 checkable，一般不用再调）而没开 tristate，点击永远到不了 PartiallyChecked，没有编译警告，没有运行时报错，状态就是少一档，非常隐蔽。

读取也有讲究。PartiallyChecked 在 `isChecked()` 眼里是 true，跟 Checked 一样，qcheckbox.cpp 的 setCheckState 对一切非 Unchecked 状态调的都是 setChecked(true)。所以想区分三态，咱们得用 `checkState()`，光靠这个 bool 分不出第三档。

信号那头更容易混。`toggled(bool)` 继承自基类，挂在 `checked` 上，而 checked 在 Checked 与 PartiallyChecked 之间恒为 true，两态互切时它一声不响。它只在 checked 真翻转时开口：从 Unchecked 点进 PartiallyChecked 发一次 toggled(true)，从 PartiallyChecked 或 Checked 切回 Unchecked 发一次 toggled(false)。咱们要监听三态变化就别指望它，QCheckBox 自己的信号才报得出第三态，带 `Qt::CheckState` 枚举值（0、1、2）的 `stateChanged(int)`。这个信号从 Qt 6.9 起标了废弃，接替的 `checkStateChanged(Qt::CheckState)` 参数直接是枚举，新代码用后者。

配套示例干脆不用 QCheckBox，直接在 QAbstractButton 上自管三态，文件在 examples/advanced/03-qtwidgets/12-qabstractbutton-base-advanced/。界面是三个 TriStateButton（选项 A、B、C）排成一行，每个按钮下方一个状态标签，窗口底部一条汇总栏。咱们把 TriStateButton 的骨架数一遍：它自己维护一个 `Qt::CheckState` 成员，重写 `nextCheckState()` 实现点击循环，重写 `hitButton()` 把命中限定在圆形区域。往基类回写的地方在 `setCheckState()`，一句 `QAbstractButton::setChecked(state == Qt::Checked)` 把三态同步回 checked。映射规则是示例自己定的，只有 Checked 映 true，PartiallyChecked 映 false，跟 QCheckBox 那套“非 Unchecked 都映 true”正好相反，咱们别把两边混了。监听端在 main.cpp，状态标签和汇总栏接的是 TriStateButton 自定义的 `stateChanged(int)` 信号，跟 QCheckBox 那个废弃信号重名，归属是另一回事。咱们把两个实验各跑一遍：把 `nextCheckState()` 里 Unchecked 的分支改成直达 Checked，点击循环退化成两态，三个按钮的状态标签从三档变两档。再在 main.cpp 里对某个按钮直接 `setCheckState(Qt::PartiallyChecked)`，一步落进第三态。点击走循环、程序化直跳的边界，亲手摸一遍就清楚了。

## autoExclusive 只认同一个 parent：轻量互斥的边界

入门篇咱们用 QButtonGroup 管理互斥，那是正统方案。基类还留了一个轻量替代，`autoExclusive` 属性。咱们把某个按钮的 `autoExclusive` 设为 true，它被选中时就会自动取消同一 parent 下所有其他 `autoExclusive` 按钮的选中状态。

机制在 `setChecked(true)` 的路径上：基类检查 autoExclusive 是否开启，开了就遍历 parent 的子控件，找到同为 QAbstractButton 且同样开了 autoExclusive 的兄弟按钮，把它们的 `checked` 置 false。笔者翻了 qabstractbutton.cpp 的 queryButtonList，候选列表就是 `parent->findChildren<QAbstractButton*>()` 过滤出来的。互斥范围因此严格限定在同一个 parent 下，互斥按钮分布在不同的容器里就管不到了，那时必须回到 QButtonGroup。

程序化调用 `setChecked(true)` 同样会触发互斥，与 QButtonGroup 的行为一致。但 autoExclusive 没有 `idClicked(int)` 信号，咱们没法像 QButtonGroup 那样用整数 id 区分按钮，互斥组需要知道“选了哪个”，就得在每个按钮的 `toggled` 槽里自己遍历兄弟来确定。笔者的工程判断：互斥逻辑超过三个按钮，或者需要 id 管理，老老实实用 QButtonGroup。

autoExclusive 和 QButtonGroup 允许同时存在，但两套机制管同一件事，咱们以后读这段代码就得先猜互斥到底归谁管，维护成本凭空多出来。笔者的做法是二选一，不同时使用。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QAbstractButton 在 QtWidgets 模块，QStyle 绘制部分在不同桌面平台（Fusion、Windows Vista、macOS）上行为一致，像素级渲染效果随当前 Style 变化。

[Qt 文档 · QAbstractButton](https://doc.qt.io/qt-6/qabstractbutton.html) -- 按钮基类，checked/down 状态机与四个核心信号的定义

[Qt 文档 · QStyleOptionButton](https://doc.qt.io/qt-6/qstyleoptionbutton.html) -- 按钮绘制参数包，features 和 state 字段说明

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- QStyle 绘制系统，drawControl 和 ControlElement 枚举

[Qt 文档 · QCheckBox](https://doc.qt.io/qt-6/qcheckbox.html) -- 三态复选框，setTristate 和 checkState 的完整说明

[Qt 文档 · QButtonGroup](https://doc.qt.io/qt-6/qbuttongroup.html) -- 按钮分组互斥管理，idClicked 信号和 exclusive 属性

---

鼠标和键盘两条路径合流进同一套状态机，QAbstractButton 替这些控件做的活到这里就看清了。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/12-qabstractbutton-base-advanced/，可执行文件 12-qabstractbutton-base-advanced）完整玩一轮。依次点三个圆形按钮，圆面上的字在 Off、Half、On 之间换，按钮下方的状态标签报的是 Unchecked、PartiallyChecked、Checked，汇总栏跟着重新计数。按住不放，`isDown()` 那三行 if 块把颜色压深，松开看状态跳一档，用眼睛确认 down 与 checked 的分工。再点按钮本体的四个角，控件是方形，圆画在中央，四角毫无反应，那是 `hitButton()` 把命中圈在了圆内，与正文里“移出按钮区域不算点击”是同一个判定。下一篇 [3.13 QFrame 基类进阶](./13-qframe-base-advanced.md) 讲 QFrame，看它怎么做带阴影和圆角的自定义容器。
