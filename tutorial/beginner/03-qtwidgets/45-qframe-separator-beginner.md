# 现代Qt开发教程（新手篇）3.45——QFrame 作为分隔线的用法

## 1. 前言 / 那条被低估的视觉分界线

我们在前面第 13 篇已经系统地拆解过 QFrame 的边框系统——Shape、Shadow、lineWidth、midLineWidth 那一套。这篇不再讲理论，而是聚焦于 QFrame 在实际界面开发中最频繁的一个用途：充当分隔线。别小看这条线，一个复杂界面如果缺少合理的视觉分隔，控件挤在一起会显得非常杂乱，用户很难快速定位自己需要操作的区域。分隔线就是把界面"分区"的最轻量手段。

今天的内容围绕四个实战场景展开。先看最基础的 QFrame::HLine 加 QFrame::Sunken 组合出一条经典水平凹线，再来看 QFrame::VLine 在工具栏中分隔按钮组的用法，然后研究 QFrame 不当分隔线而当有边框容器时的配置方式，最后把 QFrame 分隔线和布局管理器的 addSpacing 放在一起比较——搞清楚什么时候该用哪一种。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。全部内容都在 QtWidgets 模块内完成，链接 Qt6::Widgets 即可。示例代码涉及 QFrame、QLabel、QToolBar、QMainWindow、QVBoxLayout、QHBoxLayout、QGridLayout 和 QSpinBox。

## 3. 核心概念讲解

### 3.1 水平分隔线 HLine + Sunken

QFrame 当水平分隔线是最简方案，只需要三行代码。setFrameShape(QFrame::HLine) 把 QFrame 变成一条水平线，setFrameShadow(QFrame::Sunken) 给这条线加上微妙的凹入效果——上沿颜色深、下沿颜色浅，视觉上像是刻进界面里的一道细槽。最后用 setFixedHeight 把高度锁在 2 像素，避免布局管理器给它分配多余的空间。

```cpp
auto *separator = new QFrame;
separator->setFrameShape(QFrame::HLine);
separator->setFrameShadow(QFrame::Sunken);
separator->setFixedHeight(2);
```

这条线在垂直布局中尤其好用。设想你有一个设置面板，上半部分是"基本设置"分组，下半部分是"高级设置"分组，中间放一条 HLine 分隔线，用户一眼就能看出这是两个不同的功能区域。分隔线之所以选择 Sunken 而不是 Plain，原因在于 Sunken 的双色调效果比 Plain 的单色调线条更有层次感。Plain 看起来像一根平面线贴在界面上，而 Sunken 看起来像界面本身被刻了一道——更融入背景。

实际使用时有一个细节值得注意：分隔线的颜色你没法通过 QFrame 的 API 直接指定，它由当前应用程序的 QStyle 决定。在 Fusion 风格下 Sunken 线条的颜色是偏灰的，在不同系统主题下可能略有差异。如果你需要精确控制分隔线颜色，可以用 QSS 覆盖——但一旦用了 QSS 的 border 或 background-color，QFrame 原生的 Sunken 效果就被替换掉了，你需要自己在 QSS 中模拟凹入效果。所以在不需要自定义颜色的情况下，直接用原生 Sunken 是最省事的选择。

另外一个常见的需求是在分隔线旁边加文字标签——比如"基本信息"、"高级选项"这种带标题的分隔线。QFrame 本身不支持在分隔线上绘制文字，但我们可以用 QHBoxLayout 把一个 QLabel 和两条 QFrame 拼起来：左边一条水平线、中间一个 QLabel、右边一条水平线。两边各用 addStretch(1) 分配弹性空间，让线条自动延伸占满剩余宽度，文字标签居中或左对齐。

```cpp
auto *headerLayout = new QHBoxLayout;
headerLayout->setContentsMargins(0, 4, 0, 4);

auto *leftLine = new QFrame;
leftLine->setFrameShape(QFrame::HLine);
leftLine->setFrameShadow(QFrame::Sunken);
leftLine->setFixedHeight(2);

auto *titleLabel = new QLabel("基本信息");
titleLabel->setStyleSheet("color: #666; font-weight: bold;");

auto *rightLine = new QFrame;
rightLine->setFrameShape(QFrame::HLine);
rightLine->setFrameShadow(QFrame::Sunken);
rightLine->setFixedHeight(2);

headerLayout->addWidget(leftLine, 1);
headerLayout->addWidget(titleLabel);
headerLayout->addWidget(rightLine, 1);
```

这种"线 - 文字 - 线"的结构在现代桌面应用中非常常见，很多设置页面和属性面板都用这种布局来组织内容区块。

### 3.2 垂直分隔线在工具栏中的使用

垂直分隔线的用法和水平线完全对称——setFrameShape(QFrame::VLine) 把 QFrame 变成一条竖线，同样配合 Sunken 阴影，宽度锁在 2 像素。VLine 在工具栏中是最典型的应用场景。

QMainWindow 的 QToolBar 有一个 addSeparator() 方法，它底层就是往工具栏的水平布局里塞一个 VLine 的 QFrame。但 addSeparator() 创建的分隔线你没法精细控制它的外观——样式完全由 QStyle 决定。如果你需要对工具栏分隔线的宽度、颜色做调整，可以手动创建 QFrame 设为 VLine 再用 addWidget 加进工具栏。

```cpp
auto *toolbar = addToolBar("Main");

auto *newAction = toolbar->addAction("New");
auto *openAction = toolbar->addAction("Open");
auto *saveAction = toolbar->addAction("Save");

// 手动添加垂直分隔线
auto *separator = new QFrame;
separator->setFrameShape(QFrame::VLine);
separator->setFrameShadow(QFrame::Sunken);
separator->setFixedWidth(2);
toolbar->addWidget(separator);

auto *cutAction = toolbar->addAction("Cut");
auto *copyAction = toolbar->addAction("Copy");
auto *pasteAction = toolbar->addAction("Paste");
```

这里用 addWidget 而不是 addSeparator，区别在于 addWidget 给你完全的控制权——你可以自定义 QFrame 的 shadow、lineWidth 甚至用 QSS 美化它，而 addSeparator 创建的分隔线是 Qt 内部管理的，你拿不到它的指针。

VLine 除了工具栏之外，在水平布局中分隔左右区域也很好用。比如一个典型的两栏界面——左侧是导航列表，右侧是内容面板——中间可以用一条 VLine 把两个区域隔开，视觉上比突然的空白分隔清晰得多。

```cpp
auto *mainLayout = new QHBoxLayout;

auto *navPanel = new QWidget;
// ... 导航面板的子控件 ...

auto *vSeparator = new QFrame;
vSeparator->setFrameShape(QFrame::VLine);
vSeparator->setFrameShadow(QFrame::Sunken);
vSeparator->setFixedWidth(2);

auto *contentPanel = new QWidget;
// ... 内容面板的子控件 ...

mainLayout->addWidget(navPanel);
mainLayout->addWidget(vSeparator);
mainLayout->addWidget(contentPanel, 1);
```

有一点要注意：VLine 必须放在水平方向的布局里才能正确显示。如果你把一条 VLine 塞进 QVBoxLayout，它的宽度被固定但高度会自动拉伸——结果就是一条很窄的竖线变成了一个细长的矩形，看起来不对劲。反过来同理，HLine 必须放在垂直方向的布局里。方向搞反是最常见的低级失误。

### 3.3 QFrame 作为有边框容器的配置

QFrame 除了当分隔线，还有一个经常被忽略的用途——给一组控件套一个带边框的容器。你可能觉得 QGroupBox 不是专门干这个的吗？没错，QGroupBox 确实是带标题的分组容器，但它的标题是硬编码的——你必须设置一个标题文字，而且标题样式由 QStyle 控制，自定义空间有限。QFrame 作为容器就没有这些限制：你可以自由选择边框样式、用 QLabel 在任意位置放标题文字，或者干脆不要标题只要一个干净的边框。

把 QFrame 配置成有边框容器的标准做法是 setFrameShape(QFrame::StyledPanel) 加 setFrameShadow(QFrame::Raised)。StyledPanel 会根据当前应用风格自动选择最合适的面板样式——Fusion 风格下是一个简洁的灰色边框，Windows 风格下是一个微妙的立体边框。Raised 让这个面板看起来微微凸起，和背景之间形成层次感。

```cpp
auto *container = new QFrame;
container->setFrameShape(QFrame::StyledPanel);
container->setFrameShadow(QFrame::Raised);
container->setLineWidth(1);

auto *containerLayout = new QVBoxLayout(container);
containerLayout->addWidget(new QLabel("第一项"));
containerLayout->addWidget(new QLabel("第二项"));
containerLayout->addWidget(new QLabel("第三项"));
```

如果你更喜欢扁平化的现代风格，可以把 Shadow 换成 Plain——没有立体效果，就是一条干净的平面边框。然后再用 QSS 精确控制边框的颜色和圆角。

```cpp
auto *flatContainer = new QFrame;
flatContainer->setFrameShape(QFrame::StyledPanel);
flatContainer->setFrameShadow(QFrame::Plain);
flatContainer->setLineWidth(1);
flatContainer->setStyleSheet(
    "QFrame {"
    "  border: 1px solid #DDD;"
    "  border-radius: 6px;"
    "  background-color: #FAFAFA;"
    "  padding: 8px;"
    "}");
```

这里有一个容易踩的坑：给 QFrame 设了 QSS 的 border 之后，QFrame 原生的 setFrameShape / setFrameShadow 就失效了。QSS 的边框渲染和 QFrame 的原生边框渲染是两套独立的系统，它们互斥，不能叠加。所以上面的代码里虽然还保留了 setFrameShape(QFrame::StyledPanel) 和 setFrameShadow(QFrame::Plain)，但真正决定边框外观的是 QSS 中的 `border: 1px solid #DDD`。这两个系统同时存在不会报错，只是 QSS 的优先级更高——原生设置被静默覆盖。

所以实际开发中，选择策略是这样的：如果你只需要简单的标准边框，用 QFrame 原生的 Shape + Shadow 就够了，代码简洁且跨平台一致；如果你需要自定义颜色、圆角、渐变等效果，用 QSS 完全接管边框样式，不要混用两套系统。

### 3.4 QFrame vs addSpacing 的区别

QLayout 提供了 addSpacing(int size) 方法，它往布局中插入一段固定大小的空白——不放任何控件，纯粹是为了在控件之间制造间距。这和用一条 QFrame 分隔线看起来效果有些相似——都是在两个控件之间插入一段视觉间隔。但它们本质上完全不同。

addSpacing 插入的是透明空白。它不绘制任何东西，只是在布局中占据一段固定大小的空间，背景色和父控件完全一致。从视觉上看，就是两段控件之间多了一点空白——没有线条，没有颜色变化，没有视觉分隔的暗示。它适合用在"控件之间需要一点呼吸空间但不需要明确的分隔"的场景，比如一个按钮组内部各按钮之间的间距。

QFrame 分隔线插入的是一条可见的线。它有颜色（由 Shadow 和 QStyle 决定），有明确的视觉存在感，用户一看就知道"这里是两个不同的区域"。它适合用在"需要在视觉上明确划分区域"的场景，比如设置面板中"基本设置"和"高级设置"之间的分隔。

从另一个角度看，addSpacing 是布局层面的操作——它影响的是布局管理器对空间的分配，不会创建任何新的 QObject。QFrame 分隔线是控件层面的操作——它创建了一个完整的 QWidget 子对象，参与了 Qt 的对象树管理、事件分发和绘制流程。所以在性能上 addSpacing 比 QFrame 分隔线更轻量——如果只是需要一点间距，完全没必要为此创建一个 QFrame。

```cpp
auto *layout = new QVBoxLayout;

layout->addWidget(new QLabel("基本信息"));

// 方式一：用 addSpacing 制造纯空白间距
layout->addSpacing(16);

// 方式二：用 QFrame 制造可见分隔线
auto *separator = new QFrame;
separator->setFrameShape(QFrame::HLine);
separator->setFrameShadow(QFrame::Sunken);
separator->setFixedHeight(2);
layout->addWidget(separator);

layout->addWidget(new QLabel("高级信息"));
```

综合来说，当你需要在界面中做"区域分隔"——让用户明确感知到两部分内容是不同的功能区块——用 QFrame 分隔线。当你只是需要调整控件之间的间距——让界面不那么紧凑但不强调区域划分——用 addSpacing。两者不冲突，在同一个布局中可以混合使用。

## 4. 踩坑预防

第一个坑是 HLine 和 VLine 的布局方向搞反。HLine 必须放在 QVBoxLayout（或任何在垂直方向排列的布局）里才能正确拉伸宽度，VLine 必须放在 QHBoxLayout 里才能正确拉伸高度。方向搞反了分隔线要么缩成一团要么变得非常诡异。一个简单的判断方法是：HLine 是水平的，它应该在垂直方向的布局中延伸宽度；VLine 是垂直的，它应该在水平方向的布局中延伸高度。

第二个坑是 setFixedHeight 和 setFixedHeight 只锁了一个方向。水平分隔线设了 setFixedHeight(2) 之后，高度被固定为 2 像素，但宽度取决于布局——如果放在一个没有水平拉伸的布局里，宽度可能是 0。确保分隔线所在的方向上有足够的拉伸空间，或者在布局中给它设一个 stretch factor 大于 0 的值。

第三个坑是 QSS 和 QFrame 原生边框互斥的问题。如果你的全局 QSS 或者父控件的 QSS 中包含了 border 属性，它会覆盖 QFrame 的 Sunken 效果。排查的方法是检查应用级别的 setStyleSheet 以及父控件链上是否有影响 QFrame 的样式规则。如果发现分隔线看起来不是 Sunken 效果而是普通的灰线，大概率是 QSS 在捣鬼。

第四个坑是容器式 QFrame 内部子控件的边距。QFrame 的边框是绘制在客户区内部的，如果你给 QFrame 设了 lineWidth(3)，那每侧有 3 像素被边框占据。布局管理器会自动把子控件约束在 contentsRect() 内，所以子控件通常不会和边框重叠。但如果你同时在 QSS 中设了 padding，那 QSS 的 padding 和 QFrame 的 lineWidth 是叠加关系——内容区域会被进一步压缩。解决方案是用 contentsRect() 来确认实际可用的内容区域大小。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow，包含一个工具栏和中央控件。工具栏上放三组按钮——"新建"、"打开"、"保存"为文件操作组，"剪切"、"复制"、"粘贴"为编辑操作组，"帮助"、"关于"为其他组——每组之间用 VLine 分隔。中央控件使用 QVBoxLayout，包含"个人信息"和"工作信息"两个区域，每个区域上方有一条"线 - 文字标签 - 线"结构的带标题分隔线。两个区域之间再额外加一条纯 HLine 分隔。在每个区域内放几个 QLabel 和 QLineEdit 表示表单项，整体用 QFrame(StyledPanel + Raised) 包裹为一个带边框容器。底部有一行 QLabel 显示当前分隔线数量。窗口右侧用 VLine 分隔中央内容和右侧的一个窄面板（QLabel 显示"状态面板"）。

## 6. 官方文档参考链接

[Qt 文档 -- QFrame](https://doc.qt.io/qt-6/qframe.html) -- QFrame 可视框架基类

[Qt 文档 -- QFrame::Shape](https://doc.qt.io/qt-6/qframe.html#Shape-enum) -- HLine 和 VLine 在此枚举中

[Qt 文档 -- QLayout](https://doc.qt.io/qt-6/qlayout.html) -- addSpacing 方法参考

[Qt 文档 -- QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- addSeparator 和 addWidget 方法

---

到这里，QFrame 作为分隔线的所有用法就讲完了。HLine 加 Sunken 是水平分隔的标准方案，VLine 在工具栏和水平布局中分隔按钮组和左右区域，QFrame 当容器用 StyledPanel 加 Raised 或者干脆 QSS 接管，最后记住 addSpacing 是布局层面的纯空白、QFrame 分隔线是控件层面的可见分隔——场景不同选择不同。下次在界面上需要划分区域的时候，直接用 QFrame 就对了。
