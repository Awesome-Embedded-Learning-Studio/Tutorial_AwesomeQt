---
title: "3.17 QPushButton 进阶"
description: "入门篇会调 setDefault、setMenu、setIcon、setFlat，进阶篇看 QPushButton 在 QDialog 里怎么接管 Enter 键。autoDefault 升为默认按钮的两条路径、带菜单按钮对 clicked 的抑制、flat 跟 QStyle 与 QSS 的三方关系，配套示例在 examples/advanced/03-qtwidgets/17-qpushbutton-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.17——QPushButton 进阶

[入门篇 QPushButton 基础](../../beginner/03-qtwidgets/17-qpushbutton-beginner.md) 把 setDefault、setMenu、setIcon、setFlat 连同 QSS 美化过了一遍，写小工具界面够用。控件基类六篇在 [3.16 QAbstractSpinBox 基类进阶](./16-qabstractspinbox-base-advanced.md) 收了官，从本篇起咱们转到具体控件，QPushButton 打头，QToolButton、QRadioButton、QCheckBox、QCommandLinkButton 随后，凑成按钮五连。QPushButton 在普通窗口里没什么脾气，一放进 QDialog，键盘行为就超出直觉：表单里明明摆着好几个输入框，焦点也停在输入框里，按 Enter，触发的却是某个按钮的 clicked。您要是在带多个 QLineEdit 和多个 QPushButton 的表单对话框里遇到过，别急着怀疑信号连错了地方。这是 QPushButton 在 QDialog 环境里的既定行为，来历能查到 Qt 1.x 时代，那时为了兼容 Motif 风格的对话框，Qt 引入了 autoDefault 机制。[3.12 QAbstractButton 基类进阶](./12-qabstractbutton-base-advanced.md) 讲键盘路径时留过一个尾巴，Enter 是例外，QPushButton 对它另有一套处理，本篇来兑现。

笔者把最容易把人问住的三个场面摆出来。焦点停在 QLineEdit 里，按 Enter，clicked 却从一个没碰过的按钮里冒出来。给按钮挂上 `setMenu()`，菜单能弹，连着的 clicked 槽一次都没进来过。flat 按钮看着干净，点一下，一圈焦点指示收不回去，文字链接的伪装当场穿帮。三个场面各有一套根因，本篇把三件事讲明白：autoDefault 在 QDialog 里跟焦点链怎么互动，`setMenu()` 之后 clicked 为什么沉默，flat 的状态管理跟 QStyle 与 QSS 怎么纠缠。

## 焦点在输入框，Enter 却点了按钮：autoDefault 的接管

入门篇提过 autoDefault 在 QDialog 中默认为 true，焦点停在输入框里按 Enter、按钮的 clicked 跟着响的现象，连同把 autoDefault 显式设为 false 的修法，当时也当场给过。本篇接着往机制层走，Enter 的传播次序、默认按钮的认定依据、默认边框的视觉细节，这三处入门篇都没碰。咱们跟着一枚 Enter 从输入框一路走到按钮，看它经过谁、被谁消费。

键盘事件总是先送到焦点控件手上，Enter 也不意外。焦点停在 QLineEdit 里时，输入框先处理，发一串完成输入的信号出来，returnPressed 就在其中，发完把事件标记为 ignore，让它沿着父控件链继续向上走。QDialog 的 keyPressEvent 收到这枚上传的 Enter，遍历按钮时只认 isDefault() 为真且可见的那一个，找到就 click() 它。咱们翻 qdialog.cpp 能看到，这段查找从头到尾没查过 autoDefault，查的是默认按钮这个最终身份。

那 autoDefault 在哪里起作用？它管的是谁有资格被提升成默认按钮，提升有两条路。第一条路在对话框显示的时候，QDialog 沿着焦点链找第一个 autoDefault 为 true 的按钮，把它设为默认，这段逻辑写在 qdialog.cpp 里。第二条路在按钮自己身上，autoDefault 按钮获得焦点时，QPushButton 的 focusInEvent 会把自己升为对话框的默认按钮，写在 qpushbutton.cpp 里。咱们摆一个 QDialog，放一个 QLineEdit 和一个提交按钮进去，一行 setDefault 都没写，对话框一显示，提交按钮的 isDefault() 就已经是 true 了，这就是第一条路干的活。

Enter 最终落在谁头上，咱们把常见状态列一张表：

| 按下 Enter 时对话框里的状态 | 触发对象 |
| -------- | -------- |
| 焦点停在 autoDefault 为 true 的 QPushButton 上 | 焦点所在按钮的 clicked，keyPressEvent 当场消费，事件不再上传 |
| 焦点在别处，对话框里存在默认按钮（显式设的或提升上来的都算） | 默认按钮的 clicked |
| 焦点在别处，也没有默认按钮 | 没有按钮被触发，输入框的信号在这之前已照常发过 |

表里第一行值得单独看。焦点停在 autoDefault 按钮上时，qpushbutton.cpp 的 keyPressEvent 先于 QDialog 见到 Enter，咱们能在分支里直接读到判断条件，autoDefault() 或 defaultButton 为真就 click() 自己，事件就地消费。所以显式 setDefault(true) 的按钮也压不住焦点按钮，谁握着焦点谁说了算。

```cpp
auto *dialog = new QDialog();
auto *layout = new QVBoxLayout(dialog);

auto *input = new QLineEdit();
auto *submitBtn = new QPushButton("提交");
// submitBtn 的 autoDefault 在 QDialog 中默认为 true

layout->addWidget(input);
layout->addWidget(submitBtn);

connect(input, &QLineEdit::returnPressed, []() {
    qDebug() << "输入框的 returnPressed 触发";  // 先触发，早于按钮的 clicked
});
connect(submitBtn, &QPushButton::clicked, []() {
    qDebug() << "按钮的 clicked 触发";
});
```

咱们把焦点放进输入框，按 Enter，两条日志都会出现，returnPressed 在前，clicked 紧跟其后。returnPressed 是输入框先发的。事件标记为 ignore 向上走，QDialog 一查，对话框里有默认按钮，click() 就落在 submitBtn 头上。这个默认身份没人显式设过，是显示时那条提升路径把它顶上去的。两个信号都到，要提防的反而是双响应：表单同时连着 returnPressed 和 clicked 各干各的活，一回车，提交逻辑跑了两次。只有输入框带着校验器且当前输入不合规的时候，returnPressed 才会缺席，日志里只剩按钮那一条。

您要是想让 Enter 只归输入框，得把按钮从默认位置上撤下来。在对话框显示之前把 autoDefault 设为 false，显示时那条提升路径就轮不到它。对话框已经显示过、按钮已经顶着默认身份的场合，光 setAutoDefault(false) 撤不干净，默认身份是单独一份状态，setAutoDefault 动不到它，得连 setDefault(false) 一起调。

还有一个容易忽略的细节跟视觉有关。默认按钮会比别的按钮多画一圈边框，视觉反馈由 QStyle 的 PE_FrameDefaultButton 绘制元素负责，跟 QSS 里设置的 border 是独立的。咱们要是用 QSS 给按钮自定义了 border，可能把这圈默认边框盖掉，用户就没法从视觉上判断哪个按钮正顶着默认身份。这圈边框的去留也值得单独说一句。autoDefault 按钮获得焦点时升为默认、画上边框，焦点移走后边框并不消失，对话框记着自己的主默认按钮，会把失焦的那个恢复回默认身份，isDefault() 实测保持为 true。想让边框挪个地方，得有另一个 autoDefault 按钮来接棒，接棒的瞬间边框从旧按钮挪到新按钮。

配套示例的窗口按本篇三件事分成三个分区，第一个分区摆的就是这个实验，文件在 examples/advanced/03-qtwidgets/17-qpushbutton-advanced/，可执行文件 17-qpushbutton-advanced。咱们跑一轮实验，焦点放进输入框，按 Enter，日志区多出 [returnPressed] 与 [clicked] 各一条，先后顺序一眼可辨。勾掉“autoDefault = true”复选框再按一次，这回只剩 [returnPressed]，按钮那条没了。复选框连着 auto_default_demo.cpp 里 submitBtn 的 setAutoDefault 与 setDefault 两行，勾选时两个都设 true，勾掉时两个都设 false，把正文里那条连默认身份一起撤的修法做成了手动开关。要看默认身份的那圈边框，把复选框勾回来，再用 Tab 把焦点送进提交按钮，留意按钮边缘，那圈边框就画在那一带。

### 一道调试题：autoDefault 全关，Enter 落不到按钮上

咱们再看一段摆在 QDialog 里的代码，用户在输入框里按 Enter，日志里有 [returnPressed]，按钮那边一条动静都没有，您先自己找找原因：

```cpp
auto *dialog = new QDialog();
auto *input = new QLineEdit();
auto *btn1 = new QPushButton("确定");
auto *btn2 = new QPushButton("取消");
btn1->setAutoDefault(false);
btn2->setAutoDefault(false);
// 没有按钮设为 setDefault(true)
```

咱们把毛病定位到候选全空上。两个按钮的 autoDefault 都被关掉，对话框显示时那条提升路径找不到候选人，代码里也没有谁被显式设为默认，Enter 上传到 QDialog 的 keyPressEvent 之后，isDefault 的检查落空，事件就此打住，按钮侧一条日志都不会有。输入框那边一切照常，returnPressed 在事件上传之前就发过了。这里顺手把传播方向交代清楚，键盘事件只从焦点控件往父级走，QDialog 从不把键转发回子控件。想让 Enter 重新落到按钮上，保留至少一个按钮的 autoDefault 为 true，或者显式设一个 `setDefault(true)`。

## 挂上菜单，clicked 沉默：释放被菜单接管

入门篇提过带菜单的 QPushButton 点击时不触发 clicked，机制没有展开，咱们把按下之后的路径顺一遍。`setMenu()` 内部干的第一件事有点出人意料，它把按钮自己的 pressed 信号连接到一个内部槽上。按下鼠标，按钮照常进入按下状态、发出 pressed，正是这个信号把内部槽唤起来，槽里调 `menu->popup()` 把菜单异步弹出，走的不是 `QMenu::exec()` 那个同步模态循环。clicked 沉默的原因藏在释放那一步：菜单一经弹出就抓取了鼠标，随后的鼠标释放事件归菜单管，按钮凑不齐 clicked 要求的“按下后原地释放”条件，信号就一直不来。

```cpp
auto *btn = new QPushButton("操作");
auto *menu = new QMenu(btn);
menu->addAction("选项 A");
menu->addAction("选项 B");
btn->setMenu(menu);

connect(btn, &QPushButton::clicked, []() {
    qDebug() << "clicked 触发";  // 永远不会被触发
});
```

这套行为跟绘制无关，咱们把界线划清楚。QPushButton 没有重写 mousePressEvent，点击路径上看不见它的手笔，抑制完全出自 setMenu() 安装的那条信号连接。paintEvent 那一头只管画。initStyleOption 见到按钮带着 menu，就置上 HasMenu 特性，告诉样式这个按钮带着菜单，QStyle 据此在文字旁边多画一个下拉箭头，仅此而已。它跟 QToolButton 的 DelayedPopup 模式走的是两条路：DelayedPopup 允许快速点击触发 clicked、长按弹出菜单，QPushButton 没有这个中间态，点击只有弹菜单一条路。

pressed 不但活着，还是弹菜单的扳机，这条路咱们前面已经走过。想赶在菜单弹出前更新菜单项，连 pressed 做准备工作在机制上是通的，按下瞬间它先于菜单可见。更稳的入口是 QMenu 的 `aboutToShow` 信号，菜单每次将弹未弹时发出，不依赖按钮侧的信号时序，笔者建议优先用它。

要是您确实需要“点按钮主体执行默认操作，菜单只管更多选项”，路有两条。一条是改用 QToolButton，设 MenuButtonPopup 模式，按钮主体和箭头是两个独立的点击区域，主体点击触发 clicked，箭头点击弹出菜单。另一条是不用 `setMenu()`，自己处理 mousePressEvent，在事件里判断点击位置，点在主体上就手动触发 clicked，点在右侧自定义的箭头区域就弹菜单。第二条要多写不少代码，换来的灵活性也更高。

配套示例的第二个分区把“带菜单按钮”和“普通按钮（对照）”并排放着。咱们按下带菜单按钮，手先别松开，菜单已经弹出，日志里 [pressed] 先落一笔，这就是扳机的现场。松开、选一项，日志再记 [menu]。[clicked] 那条从头到尾不会来，释放那一下归了菜单。再点普通按钮看正常路径的样子，[clicked] 立刻就到。日志区每行带时间戳，比对起来方便。

## flat 的里子：跳过的绘制、残留的焦点框、不变的尺寸

入门篇把 `setFlat(true)` 当“去掉边框和背景”来用，这说的是正常状态下的效果，按钮一变到按下或选中状态，背景又会画回来一部分。咱们从绘制那一层看起。flat 属性影响的是 QStyle 对 PE_PanelButtonCommand 绘制元素的处理策略，正常状态（未按下）时跳过背景和边框的绘制，按下与选中时恢复背景绘制。悬停的反馈则要看样式，CommonStyle 和 Fusion 下压根不画背景，想要悬停效果得靠 QSS 自己补。笔者翻了 qcommonstyle.cpp 的 CE_PushButtonBevel，Flat 特性一挂上，PE_PanelButtonCommand 的绘制调用就被跳过了。

状态管理上另有一个微妙的问题。flat 按钮按下并释放后，某些 QStyle 实现会残留一个“焦点框”。按钮为什么一按就有焦点？QPushButton 的默认焦点策略是 StrongFocus，这个默认值来自 QStyle 的样式提示 SH_Button_FocusPolicy，qcommonstyle.cpp 里给的就是 StrongFocus。StrongFocus 同时包含 TabFocus 与 ClickFocus，Tab 能把焦点切进来，鼠标点击也能，所以手一按焦点就落在按钮上，释放后焦点仍在，QStyle 就会画出焦点指示器。您要是不希望 flat 按钮显示焦点框，把焦点策略改掉：

```cpp
auto *flatBtn = new QPushButton("文字链接");
flatBtn->setFlat(true);
flatBtn->setFocusPolicy(Qt::NoFocus);  // 防止残留焦点框
```

有个取舍咱们得交代：NoFocus 意味着用户没法用 Tab 切到这个按钮，界面有无障碍访问要求时，焦点能力得保留，改用 QSS 的 `outline: none` 隐藏焦点框更合适。

尺寸计算是另一处反直觉。普通 QPushButton 的 sizeHint 含着 QStyle 绘制的边框和内边距，flat 按钮不画边框，sizeHint 的计算却没有变，照样含着这些空间。咱们拿两个文字相同的按钮对比，一个 flat 一个不 flat，sizeHint 一样大。想让 flat 按钮紧凑些，得手动 `setContentsMargins`，或者用 QSS 的 padding 把内边距压小。

flat 跟 QSS 还有一层交互。入门篇提过 QSS 设置 background 和 border 后会覆盖 flat 的效果，原因还是在绘制权的归属上。QSS 定义了 background 和 border，QStyle 的绘制逻辑就被完全绕过，flat 开关从此不参与绘制，拨不拨都一个样。您要是需要按状态动态切换 flat 与非 flat 外观，别用 `setFlat()`，用 QSS 的伪状态选择器（`:hover`、`:pressed`、`:checked`）控制 background 和 border 的显隐更可靠。

实际项目里 flat 按钮最常见的用法是导航栏里的文字按钮：flat、纯文字、无焦点框，鼠标悬停时浮一层浅色背景。做出来除了 `setFlat(true)` 和 `setFocusPolicy(Qt::NoFocus)`，还得用 QSS 控制 hover 状态的背景色，flat 按钮默认的悬停效果指望不上，CommonStyle 和 Fusion 下干脆不画，这层 QSS 基本省不掉。笔者推荐的 QSS 模板是正常状态透明背景，hover 时加浅灰色背景（比如 rgba(0,0,0,0.05)），pressed 时加深一档。这样 flat 按钮跟普通文字在视觉上就有了明确的区分。

配套示例第三个分区摆了两个 flat 按钮，右边那个出厂就是 NoFocus。咱们点左边的“flat 按钮（可获焦）”，点完看边缘，焦点指示停在按钮上收不回去。勾上“左侧按钮启用 NoFocus”复选框再点，边缘干净了。两个按钮的 clicked 都会进日志，焦点框的差别得用眼睛看。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QPushButton 属于 QtWidgets 模块，内容涉及 QMenu 和 QStyle 的协作，示例可在任何支持 Qt 6 的桌面平台上编译运行，autoDefault 的行为只在 QDialog 中生效。

[Qt 文档 · QPushButton](https://doc.qt.io/qt-6/qpushbutton.html) -- 推送按钮，包含 default/autoDefault 属性说明

[Qt 文档 · QAbstractButton](https://doc.qt.io/qt-6/qabstractbutton.html) -- 按钮基类，clicked/pressed/released 信号的触发条件

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- 对话框基类，默认按钮的键盘事件处理机制

[Qt 文档 · QMenu](https://doc.qt.io/qt-6/qmenu.html) -- 菜单控件，popup() 与 exec() 两种弹出行为

---

3.12 留下的 Enter 话题到这里有了着落。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/17-qpushbutton-advanced/，可执行文件 17-qpushbutton-advanced）完整玩一轮。输入框里按 Enter，勾掉复选框再按一次，两轮日志对照看。带菜单按钮和普通按钮各点一遍，看两类按钮的日志差别。flat 按钮在勾选 NoFocus 前后各点一次，焦点框的来去肉眼比对。日志区带时间戳，每条信号几时到的都留了底。下一篇 [3.18 QToolButton 进阶](./18-qtoolbutton-advanced.md) 讲按钮五连的第二个 QToolButton，本篇两处请您留意的替代方案都落在它身上，DelayedPopup 与 MenuButtonPopup 的弹出模式比 QPushButton 细分得多。
