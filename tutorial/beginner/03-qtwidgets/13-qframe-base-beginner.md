# 现代Qt开发教程（新手篇）3.13——QFrame：可视框架基类

## 1. 前言 / 为什么你需要搞懂 QFrame

QFrame 在 Qt 控件体系中的定位很特殊——它既是 QWidget 的直接子类，也是 QLabel、QListView、QTableWidget、QTextEdit 等一大批常用控件的祖先类。QFrame 存在的意义，是在 QWidget 的基础上增加了一层"可视边框"能力：它可以在控件周围绘制各种风格的边框（盒形、面板、阴影效果），也可以直接作为水平线或垂直线分隔线来使用。我们在界面上看到的那些带凸起/凹入效果的分组框、带立体边框的文本框、以及菜单栏和工具栏之间的细线分隔符，底层都是 QFrame 的边框绘制机制在工作。

很多初学者对 QFrame 的印象停留在"用来画分隔线"或者"给控件套个框"上，这没有错但远远不够。QFrame 的边框系统由 Shape（形状）和 Shadow（阴影）两个维度构成，配合 lineWidth 和 midLineWidth 两个宽度参数，可以组合出非常丰富的视觉效果。更重要的是，由于 QFrame 是很多控件的基类，你在 QLabel、QGroupBox、QStackedWidget 等控件上调用的 `setFrameShape()` 和 `setFrameShadow()` 方法，全部来自 QFrame 的继承。搞懂 QFrame 的边框系统，就等于搞懂了 Qt 中一大类控件的边框控制方式。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QFrame 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QFrame 的边框绘制在不同平台上的表现基本一致——它使用 QPainter 在 `paintEvent` 中绘制边框，而不是依赖系统的原生边框绘制，所以你不用担心跨平台的视觉差异。不过有一点需要了解：当你给 QFrame 设置了 QSS（Qt 样式表）的 border 属性时，QSS 的边框会覆盖 QFrame 自身的边框绘制。换句话说，QSS 和 QFrame 的原生边框系统是互斥的，不能同时生效。

## 3. 核心概念讲解

### 3.1 QFrame::Shape——边框形状

QFrame::Shape 枚举定义了边框的几何形状，它决定了边框"长什么样"。我们来逐个梳理常用的 Shape 值。

`QFrame::NoFrame` 表示不绘制任何边框。这是 QFrame 子控件的默认值——比如 QLabel 默认就是无边框的，你在界面上看到的 QLabel 没有任何边线包围，就是因为它默认用了 `NoFrame`。如果你需要在代码中动态切换边框的显示与隐藏，把 shape 设为 `NoFrame` 等价于"关掉边框"。

`QFrame::Box` 绘制一个完整的矩形边框——四条边都有边线。边框的视觉效果由 Shadow（阴影）属性决定：配合 `Raised` 就是凸起的矩形框，配合 `Sunken` 就是凹入的矩形框，配合 `Plain` 就是平面的矩形框。Box 是最"完整"的边框形式，适合需要明确包围一个区域的场景，比如围绕一组控件画一个容器框。

`QFrame::Panel` 绘制一个面板风格的边框。和 Box 的区别在于视觉表现不同——Panel 的边框渲染更"薄"，通常只在一个方向上产生立体感（凸起或凹入），而 Box 的四条边都有立体的边线。Panel 适合做内容区域的容器边框，比如一个图片查看器的边框、一个状态面板的边框。

`QFrame::StyledPanel` 让 QFrame 使用当前应用程序风格的默认面板样式来绘制边框。这是最"省心"的选择——它会在 Windows 上自动使用 Windows 风格的面板，在 macOS 上使用 macOS 风格的面板，在 Linux 上使用当前主题的面板。如果你不确定该用哪种 Shape，`StyledPanel` 是最安全的选择。

`QFrame::HLine` 绘制一条水平线，没有任何边框——就只是一条横线。这个 Shape 把 QFrame 变成了一个纯粹的水平分隔线控件，常用于菜单中分隔不同功能组，或者在界面上分隔不同的区域。水平线的高度通常设为 2-3 像素就够了，宽度由布局管理器自动拉伸。

`QFrame::VLine` 绘制一条垂直线，和 `HLine` 类似但方向不同。常用于工具栏中分隔不同的按钮组，或者在水平布局中分隔不同的区域。垂直线的宽度通常也设为 2-3 像素。

`QFrame::WinPanel` 绘制 Windows 风格的面板边框。这个值在 Qt 6 中已经不推荐使用了——它的视觉效果是为 Windows 95/98 时代设计的，在现代操作系统上看起来会很过时。建议使用 `StyledPanel` 代替。

```cpp
// 各种 Shape 的示例
auto *boxFrame = new QFrame();
boxFrame->setFrameShape(QFrame::Box);
boxFrame->setFrameShadow(QFrame::Raised);
boxFrame->setLineWidth(2);

auto *panelFrame = new QFrame();
panelFrame->setFrameShape(QFrame::Panel);
panelFrame->setFrameShadow(QFrame::Sunken);
panelFrame->setLineWidth(2);

auto *styledFrame = new QFrame();
styledFrame->setFrameShape(QFrame::StyledPanel);

// 水平分隔线
auto *hSeparator = new QFrame();
hSeparator->setFrameShape(QFrame::HLine);
hSeparator->setFrameShadow(QFrame::Sunken);
hSeparator->setFixedHeight(2);
```

### 3.2 QFrame::Shadow——边框阴影效果

QFrame::Shadow 枚举定义了边框的立体感效果，它和 Shape 配合使用，共同决定边框的最终视觉表现。Shadow 有三个值。

`QFrame::Raised` 让边框呈现凸起效果。凸起的边框看起来像是控件从背景表面"抬起来"了一样——顶部和左侧的边线颜色较浅（模拟光照），底部和右侧的边线颜色较深（模拟阴影）。这种效果在传统的桌面应用界面中大量使用，特别是在 Windows 的经典主题下，按钮和面板的凸起效果就是靠这种机制实现的。

`QFrame::Sunken` 让边框呈现凹入效果。和凸起正好相反——顶部和左侧的边线颜色较深，底部和右侧的边线颜色较浅，看起来像是控件被"按进"了背景表面。凹入效果通常用于需要"嵌入感"的控件，比如文本输入框、只读的显示区域。

`QFrame::Plain` 绘制平面边框，没有任何立体效果。四条边线都是相同颜色的实线。这种风格在现代扁平化界面设计中最为常用——它简洁、干净，没有多余的装饰。配合 QSS 的 border 属性，Plain 风格可以轻松实现各种自定义颜色的平面边框。

```cpp
auto *raised = new QFrame();
raised->setFrameShape(QFrame::Panel);
raised->setFrameShadow(QFrame::Raised);
raised->setLineWidth(2);
// 看起来像凸起的面板

auto *sunken = new QFrame();
sunken->setFrameShape(QFrame::Panel);
sunken->setFrameShadow(QFrame::Sunken);
sunken->setLineWidth(2);
// 看起来像凹入的面板

auto *plain = new QFrame();
plain->setFrameShape(QFrame::Panel);
plain->setFrameShadow(QFrame::Plain);
plain->setLineWidth(2);
// 平面的面板，没有立体效果
```

Shape 和 Shadow 的组合效果可以总结为：Shape 决定边框的几何结构（矩形/面板/线条），Shadow 决定边框的光影效果（凸起/凹入/平面）。两者的组合构成了 QFrame 边框系统的全部视觉表达。在实际开发中，`StyledPanel` + `Raised` 或者 `Panel` + `Sunken` 是最常用的两种组合。

### 3.3 水平线与垂直线：HLine / VLine

QFrame 作为分隔线使用是它最简单也最常见的用法之一。你只需要设置 `setFrameShape(QFrame::HLine)` 或 `setFrameShape(QFrame::VLine)`，QFrame 就会把自己变成一条水平或垂直的线条。通常配合 `Sunken` 阴影效果来使用，这样线条看起来有微妙的凹入感，比平面线条更有层次。

水平分隔线在垂直布局中特别常用。比如你有一个 QVBoxLayout 管理的界面，上方是导航区域，下方是内容区域，中间可以用一条 HLine 来做视觉分隔。在 QMenu 中，`addSeparator()` 底层也是创建一个 HLine 的 QFrame。在工具栏（QToolBar）中，`addSeparator()` 则创建一个 VLine 的 QFrame。

```cpp
auto *layout = new QVBoxLayout();

// 上方区域
layout->addWidget(new QLabel("导航区域"));

// 水平分隔线
auto *separator = new QFrame();
separator->setFrameShape(QFrame::HLine);
separator->setFrameShadow(QFrame::Sunken);
separator->setFixedHeight(2);
layout->addWidget(separator);

// 下方区域
layout->addWidget(new QLabel("内容区域"));
```

分隔线的粗细由 `lineWidth` 控制，通常 1-2 像素就够了。如果你希望更粗的分隔线，可以增加到 3-4 像素，但超过 4 像素在视觉上就会显得太粗重了。分隔线的颜色不可直接通过 QFrame 的 API 设置——它由当前的应用风格（QStyle）决定。如果你需要自定义分隔线的颜色，可以使用 QSS：`separator->setStyleSheet("background-color: #CCCCCC;");`，或者直接不用 QFrame 而是用一个固定高度的 QWidget 配合背景色来实现。

### 3.4 lineWidth / midLineWidth：边框宽度控制

QFrame 提供了两个宽度属性来精细控制边框的粗细：`lineWidth` 和 `midLineWidth`。

`setLineWidth(int)` 设置外边框线条的宽度，单位是像素，默认值为 1。这个值直接影响边框看起来有多"粗"。在凸起和凹入模式下，lineWidth 控制的是浅色边线和深色边线各自的宽度——如果 lineWidth 是 2，那么浅色边线是 2 像素，深色边线也是 2 像素，整个边框的总宽度就是 4 像素。

`setMidLineWidth(int)` 设置中间线的宽度，默认值为 0（不绘制中间线）。中间线只在凸起（Raised）和凹入（Sunken）模式下有效——它在浅色边线和深色边线之间绘制一条额外的线条，颜色介于浅色和深色之间。这个属性主要用于创建更"厚实"的立体效果，比如经典的 Windows 工具栏按钮那种中间有一条细线的凸起效果。

```cpp
// 标准的 1 像素凸起边框
auto *thin = new QFrame();
thin->setFrameShape(QFrame::Panel);
thin->setFrameShadow(QFrame::Raised);
thin->setLineWidth(1);       // 外边框 1 像素
thin->setMidLineWidth(0);    // 无中间线

// 厚实的 2+1+2 凸起边框
auto *thick = new QFrame();
thick->setFrameShape(QFrame::Panel);
thick->setFrameShadow(QFrame::Raised);
thick->setLineWidth(2);      // 外边框 2 像素（上下各 2）
thick->setMidLineWidth(1);   // 中间线 1 像素
// 总边框宽度：2 + 1 + 2 = 5 像素（每侧）
```

你可能会问：这些宽度值在实际开发中怎么选择？答案是大部分情况下你只需要用默认的 lineWidth = 1 就够了。如果你在做传统风格的界面（仿 Windows 经典风格），lineWidth = 2 配合 midLineWidth = 1 可以做出经典的厚实凸起效果。如果你在做现代扁平化界面，直接用 `StyledPanel` + `Plain` + lineWidth = 1，或者干脆不用 QFrame 的边框而用 QSS 来控制。

最后说一个容易忽略的细节：QFrame 的边框是绘制在控件的客户区内部的，不是向外扩展的。也就是说，如果你给一个 200x100 的 QFrame 设了 lineWidth = 5 的边框，控件的总大小还是 200x100，但内容区域会被边框挤占，变成 190x90（每侧被挤掉 5 像素）。这和 CSS 的 `box-sizing: border-box` 行为一致。如果你需要计算内容区域的大小，可以用 `contentsRect()` 方法获取。

## 4. 踩坑预防

第一个坑是 QSS 和 QFrame 原生边框系统的冲突。当你给一个 QFrame（或其子类）设置了 QSS 的 `border` 属性时，QSS 会完全接管边框的绘制，QFrame 自身的 `setFrameShape()`、`setFrameShadow()`、`setLineWidth()` 设置全部失效。这两个系统是互斥的，不能叠加使用。如果你发现调了 `setFrameShape(QFrame::Box)` 但边框没出来，先检查一下是不是 QSS 覆盖了。

第二个坑是 HLine / VLine 分隔线在布局中的尺寸策略。水平分隔线（HLine）的高度被 `setFixedHeight(2)` 固定后，它在垂直布局中的宽度会自动拉伸到占满可用空间——这是正常行为。但如果你把 HLine 放进一个 QHBoxLayout 而不是 QVBoxLayout，它的高度被固定但宽度只有默认的很小值，看起来可能不是你期望的效果。确保分隔线放在对应方向的布局中：HLine 放 QVBoxLayout，VLine 放 QHBoxLayout。

第三个坑是 `QFrame::WinPanel` 的过时问题。在 Qt 6 的文档中，`WinPanel` 被标记为"提供是为了兼容旧代码"。它的视觉效果在现代操作系统上非常突兀——那个厚实的 Windows 95 风格的凹入面板在现代扁平化界面中格格不入。建议始终使用 `StyledPanel` 来替代 `WinPanel`，`StyledPanel` 会自动适配当前平台的风格。

第四个坑是边框宽度对内容区域的影响容易被忽略。由于 QFrame 的边框是绘制在客户区内部的，lineWidth = 3 意味着每侧有 3 像素被边框占据，总共 6 像素。如果你在 QFrame 内部用布局管理器放置子控件，布局管理器会自动把内容约束在 `contentsRect()` 内，所以子控件不会和边框重叠。但如果你手动用 `setGeometry()` 定位子控件，就需要自己计算 `contentsRect()` 的偏移。

第五个坑是在 Qt Designer / Qt Creator 的设计器中设置 QFrame 属性后，代码中又设置了不同的值。设计器生成的 `setupUi()` 代码会设置 QFrame 的 frameShape、frameShadow、lineWidth 等属性，如果你在构造函数中再次设置，后面的设置会覆盖前面的。建议二选一——要么全在代码中设置，要么全在设计器中设置。

## 5. 练习项目

我们来做一个综合练习：创建一个窗口，展示 QFrame 各种边框效果的对比。窗口使用 QGridLayout，在网格中放置 9 个 QFrame，分别展示三种 Shape（Box / Panel / StyledPanel）和三种 Shadow（Raised / Sunken / Plain）的组合。每个 QFrame 内部放一个 QLabel 说明当前组合。在网格上方放置一条 HLine 作为标题和内容的分隔线。在网格下方放置一个控制面板，包含两个 QSpinBox 用于动态调整 lineWidth（范围 0-10）和 midLineWidth（范围 0-5），修改后所有 9 个 QFrame 的边框宽度同步更新。窗口右下角放一个 VLine 作为垂直分隔，分隔控制面板和其他信息。

几个提示：QGridLayout 用 `addWidget(widget, row, col)` 来定位，9 个 QFrame 放在 3x3 网格中；QSpinBox 的 `valueChanged` 信号连接到一个 lambda，遍历所有演示 QFrame 并调用 `setLineWidth()` / `setMidLineWidth()`；HLine 分隔线用 `setFrameShape(QFrame::HLine)` + `setFixedHeight(2)`。

## 6. 官方文档参考链接

[Qt 文档 · QFrame](https://doc.qt.io/qt-6/qframe.html) -- QFrame 可视框架基类

[Qt 文档 · QFrame::Shape](https://doc.qt.io/qt-6/qframe.html#Shape-enum) -- 边框形状枚举

[Qt 文档 · QFrame::Shadow](https://doc.qt.io/qt-6/qframe.html#Shadow-enum) -- 边框阴影枚举

[Qt 文档 · QLabel](https://doc.qt.io/qt-6/qlabel.html) -- QLabel 继承自 QFrame

[Qt 文档 · QGroupBox](https://doc.qt.io/qt-6/qgroupbox.html) -- QGroupBox 也继承自 QFrame

---

到这里，QFrame 的边框系统你就搞清楚了。Shape 决定边框的几何形态（矩形框/面板/分隔线），Shadow 决定边框的立体效果（凸起/凹入/平面），lineWidth 和 midLineWidth 精细控制边框粗细，HLine/VLine 直接把 QFrame 变成分隔线。这些能力不仅属于 QFrame 自身——通过继承，它们同样适用于 QLabel、QGroupBox、QStackedWidget 等一大批控件。下次你在界面上需要画边框或分隔线的时候，不用再犹豫用什么控件了，QFrame 就是为此而生的。
