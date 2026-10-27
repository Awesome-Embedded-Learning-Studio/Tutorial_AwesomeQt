---
title: "3.11 QWidget 基类进阶"
description: "入门篇认识了 QWidget 的窗口属性、显示控制和尺寸策略，进阶篇把 setAttribute(Qt::WA_*) 管的那批窗口属性讲清楚。属性设了窗口却透得不见踪影，根子在 paintEvent 的绘制责任、WA_NoSystemBackground 的预填充和 WA_OpaquePaintEvent 的清除承诺三方的分工。本篇讲明白属性三角的联动、无边框窗口的手动拖动、setAttribute 的调用开销，以及 WindowFlags 与 WA_* 属性的互相牵制，配套示例在 examples/advanced/03-qtwidgets/11-qwidget-base-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.11——QWidget 基类进阶

[入门篇 QWidget 基础](../../beginner/03-qtwidgets/11-qwidget-base-beginner.md) 把窗口属性、show/hide、sizePolicy 和 WindowFlags 过了一遍，这些接口咱们每天写界面都在用。其中的 `setAttribute(Qt::WA_*)` 窗口属性，入门篇只在无边框示例里带过一行，没有展开。教程从这里进入控件基类部分，QWidget 打头，它是所有控件的共同祖先，WA_* 属性就是它代表全体控件跟操作系统窗口系统打交道的地方。平时咱们不必碰它们，可一旦要做半透明窗口、无边框窗口、自定义形状窗口，就一条都绕不开。

绕不开的时候，最常见的场面是 WA_TranslucentBackground 明明设了，窗口却不见踪影，内容区整个透掉，桌面直接露出来。咱们可能会问，属性不就是开个开关吗，怎么开了之后窗口像没跑起来一样。答案藏在三处，paintEvent 的绘制责任、几个属性之间的联动、窗口标志的配合。本篇把四件事讲明白：WA_TranslucentBackground、WA_NoSystemBackground、WA_OpaquePaintEvent 三个属性怎么分工，无边框窗口的拖动怎么用鼠标事件自己接管，setAttribute 反复调用的开销在哪，WindowFlags 和 WA_* 属性之间怎么互相牵制。

## 半透明不是单开关：三个 WA_* 属性的分工与联动

`Qt::WA_TranslucentBackground` 管的是总开关，它告诉 Qt，这个窗口要支持半透明，窗口的某些区域可以半透明或完全透明，让底下的其他窗口透过来。不设置它，Qt 会拿默认的不透明背景色把窗口填满。设置之后，这层填充没有了，窗口里画什么，由咱们在 `paintEvent` 里用带 alpha 通道的 QColor 主动绘制。半透明最终要靠操作系统的合成器把多层窗口混在一起，Windows 上是 DWM，macOS 上是 Quartz，Linux 上是各桌面环境的合成器，在禁用合成器的 Linux 窗口管理器上，半透明可能不生效。还有一层要当心，单独设 WA_TranslucentBackground 可能不够，怎么个不够法，咱们在后面的调试题里亲手复现一遍。

`Qt::WA_NoSystemBackground` 管预填充。正常流程里，Qt 在调用咱们的 `paintEvent` 之前，会先用系统默认背景色（通常是白色或系统主题色）填充整个控件区域。设置 WA_NoSystemBackground 后，预填充被跳过，paintEvent 执行之前控件区域的内容是未定义的，上一帧的像素可能还留在那里。设 WA_TranslucentBackground 时，Qt 内部会自动补设 WA_NoSystemBackground，这一步是无条件的，笔者翻过 qwidget.cpp 的 QWidget::setAttribute 实现，走进 WA_TranslucentBackground 那个处理分支，头一件事就是补设 WA_NoSystemBackground。稳妥的做法是两个都显式写上，不依赖隐式行为。

`Qt::WA_OpaquePaintEvent` 管旧内容的清除。它向 Qt 承诺，我的 paintEvent 会完整重绘整个控件区域，不需要你帮我擦旧内容。它和 WA_NoSystemBackground 的分界就在这里，后者跳过系统背景的预填充，前者跳过旧内容的清除。对完全不透明的自定义绘制控件，比如游戏画面、自绘渲染引擎，咱们设上它可以省掉不必要的背景清除，绘制能快一点。用在半透明窗口上就不行了，半透明窗口恰恰要保留清除步骤才能正确合成透明区域，设了 WA_OpaquePaintEvent，透明区域的合成就会出错。

这个承诺还有另一面要您留意，设了它，paintEvent 就必须真的覆盖每一个像素。要是咱们的 paintEvent 只重绘局部，比如只更新一个小矩形，没被重绘的区域会保留上一帧的残影，控件看起来像有噪点或拖影，每帧都在闪。所以纪律是两条，半透明窗口不设 WA_OpaquePaintEvent，设了它的控件必须整面重绘，做局部更新（比如拿 `event->rect()` 裁剪绘制区域）的控件不要设。

### 一道调试题：属性设了，窗口却不见了

咱们看一段能编译能运行、效果却不对的代码，您先自己找找原因：

```cpp
auto *window = new QWidget;
window->setWindowFlags(Qt::FramelessWindowHint);
window->setAttribute(Qt::WA_TranslucentBackground);
window->resize(400, 300);
window->show();
```

运行之后，窗口不见了：内容区完全透明，桌面直接透出来，找不出一块有颜色的地方。毛病不在属性，在 paintEvent。默认的 paintEvent 什么都不画，WA_TranslucentBackground 又顺手关掉了系统背景的预填充，窗口里画什么仍然是咱们自己的责任，不在 paintEvent 里用半透明颜色主动填背景，交给合成器的画面里就一个带颜色的像素都没有，窗口自然透得干干净净。修法是重写 paintEvent，用半透明颜色填充窗口：

```cpp
void paintEvent(QPaintEvent*) override
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 128));  // 半透明黑色
}
```

这类问题编译器帮不上忙，属性设了，代码能跑，唯独观感不对，咱们只能回到机制里找原因。配套示例 frameless_widget.cpp 的构造函数把三件事一次做齐，setWindowFlags(Qt::FramelessWindowHint)、setAttribute(Qt::WA_TranslucentBackground)、setAttribute(Qt::WA_NoSystemBackground)，paintEvent 里再用带 alpha 的颜色画圆角背景，这是完整可用的组合。

## 标题栏没了，拖动得自己接：从 mousePressEvent 到事件穿透

`Qt::FramelessWindowHint` 入门篇用过去掉标题栏和边框。去掉的同时，窗口也拖不动了，系统搬窗口的入口跟着标题栏一起没了，实际项目里，咱们得自己把它接上。

实现思路很直接，咱们在 mousePressEvent 里记录鼠标按下时的全局坐标和窗口位置，mouseMoveEvent 里计算鼠标移动的偏移量，用 move() 更新窗口位置。mouseMoveEvent 默认只在有按键按住时触发（前提是没有开启 mouseTracking），所以不必额外判断有没有按键，代码里再用 buttons() 把范围收窄到左键：

```cpp
void mousePressEvent(QMouseEvent* event) override
{
    if (event->button() == Qt::LeftButton) {
        drag_position_ = event->globalPosition().toPoint()
                       - frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void mouseMoveEvent(QMouseEvent* event) override
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - drag_position_);
    }
    QWidget::mouseMoveEvent(event);
}
```

两个坐标细节，咱们对着上面的代码看。取鼠标位置用 `globalPosition().toPoint()` 而不是 `pos()`，因为拖动要的是鼠标在屏幕上的绝对坐标，`pos()` 给的是相对控件的坐标。窗口那头取 `frameGeometry().topLeft()`，它是窗口含边框左上角的屏幕坐标，无边框窗口没有可见边框，frameGeometry 和 geometry 此时相等，取哪个都行。mousePressEvent 里减出偏移量，mouseMoveEvent 里再把它减回去，为的是保持鼠标点击位置和窗口左上角的相对距离不变，不减的话，窗口会直接跳到鼠标所在的位置。

配套示例把拖动限定在标题栏区域，文件在 examples/advanced/03-qtwidgets/11-qwidget-base-advanced/，frameless_widget.cpp 的 mousePressEvent 里多一个条件，`event->position().y() < 40`，只有顶部 40 像素内按下左键才记录偏移量。咱们运行示例试试，在标题栏按住拖动，窗口跟手走，在内容区按住拖动，窗口纹丝不动，差别全在一个判断上。

拖动接上了，还有一半交互丢了。FramelessWindowHint 去掉系统边框，用户拖窗口边缘调整大小的能力也一起没了，窗口大小永远停在创建时设定的值，想拉大一点都拉不动，体验非常差。补法是咱们在窗口边缘自己做 resize 区域，mouseMoveEvent 里检测鼠标是否落在边缘（比如距离边框 5 像素以内），是就换上对应方向的光标形状（SizeHorCursor、SizeVerCursor、SizeFDiagCursor 等），再在 mousePressEvent 与 mouseMoveEvent 里，处理边缘拖拽的逻辑。这块逻辑代码量不小，边缘和角要分开对待，值得封装成可复用的基类。示例只实现了拖动，缩放留给您照着思路补全。

鼠标事件这块还有一个属性常用于覆盖层，`Qt::WA_TransparentForMouseEvents`。设置它之后，鼠标事件不再送达这个控件，直接传递给它下面的控件，仿佛它不存在。做 overlay 时特别顺手，比如一个半透明的信息提示浮在主界面上方，咱们不希望它拦截鼠标点击，设上 `setAttribute(Qt::WA_TransparentForMouseEvents)` 就行。它只管鼠标事件，键盘事件不受影响。

## setAttribute 不是免费调用：位掩码后面连着平台插件

`setAttribute()` 每次调用都有成本。咱们每改一个 `WA_*` 属性，Qt 内部要做几件事，更新 `QWidgetPrivate` 中的属性位掩码，判断属性变更要不要触发布局或重绘，涉及窗口系统层面的变更（比如 WA_TranslucentBackground）时，还要调用平台插件更新原生窗口的属性。所以在每一帧的 paintEvent 或定时器回调里频繁切换 `WA_*` 属性，开销是可观的，不只是 CPU 计算，还可能让窗口被反复重建。

一个典型的反面案例是穿透模式切换。有人想用 timer 回调交替调用 `setAttribute(Qt::WA_TransparentForMouseEvents, true)` 和 `setAttribute(Qt::WA_TransparentForMouseEvents, false)`，让控件周期性地穿透和拦截。每切一次就触发一次属性更新和重绘通知，高频定时器下就是明显的性能问题和界面闪烁。正确的做法是咱们在初始化时一次性设好所有属性，运行时不要修改。确实需要动态切换鼠标穿透，考虑用 `setMouseTracking()` 和事件过滤器配合条件判断来实现，别反复调 setAttribute。

## 标志与属性互相牵制：WindowFlags 与 WA_* 的联动

咱们不能把 WindowFlags 和 `WA_*` 属性当两套互不相干的开关用，某些 WindowFlags 会隐式触发 `WA_*` 属性的变更，反过来，某些`WA_*` 属性也要特定的 WindowFlags 配合才生效。

最典型的联动是 `Qt::FramelessWindowHint` 和 `Qt::WA_TranslucentBackground` 的配合。在 Windows 上做半透明窗口，必须两个同时设。原因是 Windows 的 DWM（Desktop Window Manager）对分层窗口（Layered Window）的实现要求窗口必须是 Frameless 的，说得更准确些，Qt 在 Windows 上通过 WS_EX_LAYERED 扩展样式实现半透明，这个扩展样式和标准窗口边框有冲突。macOS 和 Linux 上限制不那么严格，但为了跨平台一致，咱们还是两个都设，换平台不用改代码。配套示例的构造函数里两个都在。

另外两组联动平时不起眼，需要时却缺一不可，咱们摆成表：

| 联动组合                                            | 分工与生效条件                                                                                         |
| --------------------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| Qt::WindowStaysOnTopHint + WA_ShowWithoutActivating | 标志让窗口始终位于最上层，属性让 show() 不触发激活，浮动通知要置顶又不抢焦点时缺一不可                 |
| Qt::Tool + WA_MacAlwaysShowToolWindow               | macOS 上 Tool 窗口随应用失焦自动隐藏，做常驻浮动工具面板时要设这个属性保持始终可见，其他平台上没有效果 |

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QWidget 在 QtWidgets 模块，半透明效果依赖各平台合成器，示例在 Windows 10+、macOS 12+、主流 Linux 桌面环境下均可编译运行。

[Qt 文档 · QWidget](https://doc.qt.io/qt-6/qwidget.html) -- QWidget 基类，包含 setAttribute / windowFlags / paintEvent 等核心接口

[Qt 文档 · Qt::WidgetAttribute](https://doc.qt.io/qt-6/qt.html#WidgetAttribute-enum) -- 所有 WA_* 属性的完整列表和说明

[Qt 文档 · Qt::WindowType](https://doc.qt.io/qt-6/qt.html#WindowType-enum) -- 窗口类型和窗口标志的完整枚举

[Qt 文档 · Window Flags Example](https://doc.qt.io/qt-6/qtwidgets-widgets-windowflags-example.html) -- 官方窗口标志示例，演示各种标志组合的效果

[Qt 文档 · Shaped Clock Example](https://doc.qt.io/qt-6/qtwidgets-widgets-shapedclock-example.html) -- 半透明异形窗口的官方示例

---

QWidget 在窗口层替所有控件做的工作就补到这里：一个 WA_* 属性设下去，写进 QWidgetPrivate 里的位掩码，再由平台插件同步到原生窗口。拖动接管和标志联动，咱们也都过了一遍。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/11-qwidget-base-advanced/，可执行文件 11-qwidget-base-advanced）完整玩一轮。按住标题栏拖一圈，看窗口怎么跟手，再到内容区按住拖一次，对比 frameless_widget.cpp 里 y < 40 那行判断拦下了什么，最后点右上角 X 关掉窗口，确认关闭按钮走的还是普通 clicked 到 close() 的路。胆子大一点的，把构造函数里 setAttribute(Qt::WA_NoSystemBackground) 一行注释掉重新编译，窗口依旧半透明，因为 Qt 6 设 WA_TranslucentBackground 时会自动补上它，看完再放回去，显式写上终究更稳。再把 frameless_widget.cpp 顶部的私有常量 kBgAlpha 从 217 改到 60 上下，重新编译，看桌面能透过窗口多少。下一篇 [3.12 QAbstractButton 基类进阶](./12-qabstractbutton-base-advanced.md) 讲 QPushButton、QCheckBox、QRadioButton 的共同基类，按钮的按下弹起、checkable 状态和互斥管理都由它提供。
