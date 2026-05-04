# 现代Qt开发教程（新手篇）3.1——布局系统基础

## 1. 前言 / 为什么需要布局管理器

最最开始的时候，我是真的会以为GUI编程中，控件的排列是靠手动算每个控件的 x、y 坐标，然后用 `move()` 和 `resize()` 把它们摆到指定位置（艹，想想就抽象是不是？）。窗口大小写死 800x600（你别说，你真别说我真接受过Qt项目一瞅窗口大小写死的，但是实际上真的只是写代码的不知道Layout这个东西。。。），看起来还行。然后用户把窗口一拉大——所有控件还是挤在左上角，右边一大片空白，丑到没法看。更要命的是不同系统的字体大小不一样，Windows 上排版正常，到了 Linux 上文字就溢出了。

后来才知道，Qt 有一整套布局管理器专门解决这些问题。布局管理器的核心思想很简单：你告诉它控件之间的相对关系——"这个按钮在左边的标签旁边"、"这些控件从上到下排列"——然后布局管理器会根据窗口的实际大小自动计算每个控件的位置和尺寸。窗口拉大，控件跟着变大；窗口缩小，控件自动收缩。不同平台、不同字体、不同 DPI 之下，界面都能保持合理的比例。

Qt 提供了四种常用的布局管理器：QHBoxLayout 做水平排列、QVBoxLayout 做垂直排列、QGridLayout 做网格排列、QFormLayout 做表单排列。还有一个 QStackedLayout 用于多页面切换。这五个加上 stretch 和 spacing 的调节手段，基本覆盖了你能见到的所有界面排版需求。

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本，CMake 3.26+，C++17 或更高标准。所有布局类都在 QtWidgets 模块中，所以示例代码只需链接 Widgets 和 Gui 两个模块。示例可以在任何支持 Qt6 的桌面平台上编译运行，布局管理器会自动处理不同平台的字体和 DPI 差异。

## 3. 核心概念讲解

### 3.1 QHBoxLayout 和 QVBoxLayout：一维排列的基础

QHBoxLayout 和 QVBoxLayout 是最基础的两个布局管理器——前者让控件从左到右水平排列，后者让控件从上到下垂直排列。它们俩的用法完全一样，只是方向不同。

我们先看一个最简单的例子：三个按钮水平排列。

```cpp
auto *layout = new QHBoxLayout;
layout->addWidget(new QPushButton("按钮1"));
layout->addWidget(new QPushButton("按钮2"));
layout->addWidget(new QPushButton("按钮3"));
setLayout(layout);
```

就这样，三个按钮会自动平分水平空间，从左到右排成一行。当你拉伸窗口时，三个按钮会等比例变宽。

垂直排列同理，把 QHBoxLayout 换成 QVBoxLayout 即可。控件会从上到下依次排列，每个控件默认占据其 `sizeHint()` 推荐的高度，剩余空间由设置了 stretch 的控件分配。

这里有一个初学者经常搞混的概念：布局不是 Widget，它没有自己的视觉表示。布局是附加在 Widget 上的，负责管理这个 Widget 的子控件的排列方式。所以使用布局的标准流程是：创建一个容器 Widget，创建一个布局，把子控件加到布局里，然后用 `setLayout()` 把布局绑定到容器 Widget 上。

```cpp
// 标准流程
auto *container = new QWidget;          // 容器
auto *layout = new QVBoxLayout(container);  // 创建布局，直接传入容器
// auto *layout = new QVBoxLayout;       // 也可以先创建再绑定
// container->setLayout(layout);

layout->addWidget(new QLabel("标题"));
layout->addWidget(new QTextEdit);
container->show();
```

你会发现 `new QVBoxLayout(container)` 这种写法同时完成了创建布局和绑定到容器两步，比先 new 再 setLayout 更简洁。两种写法效果完全一样，选哪种看个人习惯。

### 3.2 布局嵌套：组合出复杂界面

真正的界面很少只有一个层级的布局。比如一个典型的对话框：顶部是一个文本框（垂直方向占大部分空间），底部是一排水平排列的按钮。这需要 QVBoxLayout 套一个 QHBoxLayout：

```cpp
auto *mainLayout = new QVBoxLayout;

// 上半部分：文本编辑区
auto *textEdit = new QTextEdit;
mainLayout->addWidget(textEdit, 1);  // 第二个参数是 stretch，1 表示占据剩余空间

// 下半部分：按钮栏（水平排列）
auto *buttonLayout = new QHBoxLayout;
buttonLayout->addWidget(new QPushButton("确定"));
buttonLayout->addWidget(new QPushButton("取消"));
buttonLayout->addStretch();  // 弹性空间，把按钮推到左边

mainLayout->addLayout(buttonLayout);  // 把子布局加到主布局中
setLayout(mainLayout);
```

`addLayout()` 是把一个布局加到另一个布局中的方法，这就是布局嵌套的关键。你可以无限层级地嵌套——QVBoxLayout 里套 QHBoxLayout，再套 QGridLayout——直到满足你的排版需求。实际开发中，一个中等复杂度的界面嵌套三四层布局是很正常的。

addWidget 的第二个参数 stretch 非常重要，它决定了控件在分配额外空间时的权重。stretch 为 0 表示控件不会自动拉伸，保持 sizeHint 推荐的大小；stretch 为 1 表示控件参与均分剩余空间；stretch 为 2 表示这个控件分到的额外空间是 stretch 为 1 的控件的两倍。

```cpp
// 三个控件，中间的占据所有额外空间
layout->addWidget(new QLabel("固定标签"), 0);  // stretch=0，不拉伸
layout->addWidget(new QTextEdit, 1);            // stretch=1，占据剩余空间
layout->addWidget(new QLabel("固定标签"), 0);  // stretch=0，不拉伸
```

### 3.3 QGridLayout：网格布局与行列 span

当你需要控件按照二维网格排列时，QGridLayout 就是最合适的工具。它把空间划分成行和列组成的网格，每个控件占据其中一个单元格。

```cpp
auto *grid = new QGridLayout;

// addWidget(widget, row, col) —— 指定行列位置
grid->addWidget(new QLabel("用户名:"), 0, 0);   // 第 0 行第 0 列
grid->addWidget(new QLineEdit, 0, 1);             // 第 0 行第 1 列
grid->addWidget(new QLabel("密码:"), 1, 0);       // 第 1 行第 0 列
grid->addWidget(new QLineEdit, 1, 1);             // 第 1 行第 1 列

setLayout(grid);
```

行列编号从 0 开始，这个没什么歧义。但有一个容易被忽略的细节：QGridLayout 会根据你添加的控件自动推断行数和列数。你不需要提前声明"这个网格有 3 行 2 列"，只要往里面加控件就行了。

更有用的是行列 span——让一个控件跨多行或多列。比如一个对话框的底部按钮栏，你可能希望它横跨整行：

```cpp
// addWidget(widget, row, col, rowSpan, colSpan)
grid->addWidget(new QTextEdit, 0, 0, 1, 2);  // 第 0 行，跨 1 行 2 列
grid->addWidget(new QPushButton("提交"), 1, 0, 1, 1);  // 正常占 1 格
grid->addWidget(new QPushButton("重置"), 1, 1, 1, 1);  // 正常占 1 格
```

span 参数的后两个数字分别表示跨越的行数和列数。`1, 2` 的意思是这个控件从当前位置开始占据 1 行 2 列。这在做复杂排版的时候非常好用，比如一个计算器界面：数字键都是 1x1，等号键跨两行，0 键跨两列。

你还可以通过 `setColumnStretch()` 和 `setRowStretch()` 设置各列各行的伸缩比例，效果和 BoxLayout 的 stretch 参数类似：

```cpp
grid->setColumnStretch(0, 1);  // 第 0 列 stretch=1
grid->setColumnStretch(1, 2);  // 第 1 列 stretch=2，宽度是第 0 列的两倍
```

### 3.4 QFormLayout：表单布局

QFormLayout 是专门为"标签+输入控件"这种表单式界面设计的布局。你可能觉得用 QGridLayout 也能实现同样的效果——确实能，但 QFormLayout 提供了更简洁的 API 和更好的平台适配行为。

```cpp
auto *form = new QFormLayout;
form->addRow("姓名:", new QLineEdit);
form->addRow("邮箱:", new QLineEdit);
form->addRow("电话:", new QLineEdit);
form->addRow("备注:", new QTextEdit);
setLayout(form);
```

`addRow(const QString &labelText, QWidget *field)` 这个签名非常方便，一行代码搞定标签和输入控件的配对。如果你需要对标签做更多定制，也可以传入一个 QLabel 对象：`addRow(new QLabel("姓名:"), new QLineEdit)`。

QFormLayout 有一个 QGridLayout 做不到的事：它会根据当前桌面的风格自动决定标签放在输入框的左边还是上面。在大多数桌面环境下，标签默认在左边；但在某些特殊平台或者 Qt 的特定配置下，标签可能会被放到输入框上方。这种自适应行为在你做跨平台应用时很有价值。

你还可以通过 `setRowWrapPolicy()` 和 `setFieldGrowthPolicy()` 来控制具体的排版策略。比如 `QFormLayout::ExpandingFieldsGrow` 会让所有设置了 expending size policy 的输入框自动拉伸，`QFormLayout::WrapAllRows` 会让所有标签都显示在输入框上方。这些细节在你需要精细调整表单外观的时候再查文档就行。

### 3.5 QStackedLayout：页面切换

QStackedLayout 的用途跟前面几个不太一样——它不是用来排版的，而是用来做页面切换的。它管理一组 Widget，但同一时间只显示其中一个。你通过 `setCurrentIndex()` 或 `setCurrentWidget()` 来切换当前显示的页面。

这个布局最常见的应用场景是"选项卡式"的界面——左侧一个 QListWidget 做导航菜单，右侧一个 QStackedLayout 根据菜单选择切换不同的设置页面。

```cpp
auto *stackedLayout = new QStackedLayout;

auto *page1 = new QWidget;
auto *page1Layout = new QVBoxLayout(page1);
page1Layout->addWidget(new QLabel("这是第一页"));
// ... 添加更多控件

auto *page2 = new QWidget;
auto *page2Layout = new QVBoxLayout(page2);
page2Layout->addWidget(new QLabel("这是第二页"));

stackedLayout->addWidget(page1);  // index 0
stackedLayout->addWidget(page2);  // index 1

// 切换到第二页
stackedLayout->setCurrentIndex(1);

// 或者根据 Widget 指针切换
// stackedLayout->setCurrentWidget(page2);
```

QStackedLayout 和 QStackedWidget 的关系，就跟 QLayout 和 QWidget 的关系一样——QStackedWidget 是对 QStackedLayout 的一层封装，提供了一个更方便的 Widget 接口。如果你不需要精细控制布局行为，直接用 QStackedWidget 就行。

### 3.6 addStretch、setSpacing 和 setContentsMargins

这三个方法不属于任何一个特定的布局类，而是所有布局类共有的调节手段。

`addStretch()` 在 BoxLayout 中插入一段弹性空白。它的效果是"占据所有剩余空间"。最常见的用法是把按钮推到右边或底部：

```cpp
auto *layout = new QHBoxLayout;
layout->addWidget(new QPushButton("左边的按钮"));
layout->addStretch();  // 弹性空白，把后面的控件推到最右边
layout->addWidget(new QPushButton("右边的按钮"));
```

`addStretch()` 也可以传入一个 stretch 参数，在多个 stretch 之间按比例分配空间。但大多数情况下你只需要一个无参数的 `addStretch()` 就够了。

`setSpacing(int)` 设置控件之间的间距，单位是像素。默认值通常是取决于平台风格的某个值（一般在 5-10 像素之间）。如果你觉得控件之间太挤了或者太松了，调这个值就行。

`setContentsMargins(int left, int top, int right, int bottom)` 设置布局的外边距——也就是布局区域和 Widget 边缘之间的距离。如果你发现控件紧贴着窗口边缘不好看，把 margins 设大一点就好了。

```cpp
layout->setSpacing(10);                      // 控件之间间距 10px
layout->setContentsMargins(15, 15, 15, 15);  // 四周外边距 15px
```

还有一个简化版本 `setContentsMargins(int all)` 可以一次性设置四个方向相同的外边距，用起来更方便。

到这里你可以想一个问题：如果你要做一个包含标题、内容区域和底部按钮栏的界面，应该怎么组合使用这些布局？标题固定高度、内容区域占满剩余空间、按钮栏固定在底部——用 QVBoxLayout 嵌套 QHBoxLayout，配合 stretch 就能搞定。如果你能在脑子里把这个结构画出来，说明布局系统的核心你已经掌握了。

## 4. 踩坑预防

第一个坑是不设布局就直接用 `move()` 和 `resize()` 定位控件。这种方式叫做"绝对定位"，不是说不能用，但它的缺点非常明显：窗口大小变了控件不会跟着变，不同平台的字体 DPI 不同排版也会乱。除非你做的是固定尺寸的嵌入式界面或者游戏 UI，否则都应该用布局管理器。绝对定位属于"知道有这个东西就好，日常别用"的范畴。

第二个坑是布局的父子关系搞混。当你写 `new QVBoxLayout(widget)` 的时候，布局会自动成为这个 widget 的子对象，并且自动管理添加到布局中的所有控件的父子关系。但如果你写 `new QVBoxLayout` 不传 widget，然后又忘了 `setLayout()`，这个布局就不会被任何 widget 管理，也不会生效。更糟糕的是你往里面加的控件也不会有正确的 parent，可能导致内存泄漏。养成习惯：创建布局的时候直接传容器 widget，或者创建完立刻 `setLayout()`。

第三个坑是混用同一个布局上的 stretch 和 sizePolicy。stretch 决定了布局给控件分配额外空间的比例，而 sizePolicy 决定了控件自己愿不愿意接受额外空间。如果你给一个控件设了 `setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)`，那不管你给它多大的 stretch，它都死活不变大。这两个概念是配合工作的：stretch 管"给多少"，sizePolicy 管"收不收"。

第四个坑是在已经设置了布局的 Widget 上再次调用 `setLayout()`。Qt 的一个 Widget 只能有一个顶层布局，如果你试图设第二个，会直接报 warning 并且行为不可预测。如果你需要动态改变布局结构，不要替换整个布局，而是用 `removeWidget()` 和 `addWidget()` 来调整内容。

## 5. 练习项目

我们来做一个综合练习：用五种布局组合实现一个类似"设置面板"的界面。左侧是一个垂直排列的导航菜单（三个按钮：个人信息、外观设置、关于），右侧是一个 QStackedLayout 切换三页内容。个人信息页用 QFormLayout 排列表单（姓名、邮箱、电话输入框），外观设置页用 QGridLayout 排列颜色选择按钮和字体大小调节，关于页用 QVBoxLayout 居中显示版本信息。

完成标准是：主窗口用 QHBoxLayout 分为左右两栏，左栏宽度固定（用 `setFixedWidth` 或 stretch 控制）、右栏自适应；三个导航按钮用 QVBoxLayout 排列，底部加一个 `addStretch()` 让按钮靠上；QStackedLayout 管理三页内容，点击导航按钮时调用 `setCurrentIndex()` 切换；所有布局设置合理的 spacing 和 margins，控件之间不拥挤不松散。

几个提示：左栏导航可以用一个独立的 QWidget 作为容器，对这个容器设置 QVBoxLayout 而不是对整个窗口；QStackedLayout 添加页面的顺序决定了 `currentIndex` 的对应关系，要跟导航按钮的信号对应上；QGridLayout 的 setColumnStretch 可以让输入框列自动拉伸而标签列保持固定宽度。

## 6. 官方文档参考链接

[Qt 文档 · Layout Management](https://doc.qt.io/qt-6/layout.html) -- Qt 布局系统的完整概述，包含布局管理器的工作原理和使用建议

[Qt 文档 · QHBoxLayout](https://doc.qt.io/qt-6/qhboxlayout.html) -- 水平布局的 API 文档，addWidget / addStretch / setSpacing 等方法详解

[Qt 文档 · QVBoxLayout](https://doc.qt.io/qt-6/qvboxlayout.html) -- 垂直布局的 API 文档，与 QHBoxLayout 接口一致

[Qt 文档 · QGridLayout](https://doc.qt.io/qt-6/qgridlayout.html) -- 网格布局的 API 文档，包含行列 span 和 stretch 设置

[Qt 文档 · QFormLayout](https://doc.qt.io/qt-6/qformlayout.html) -- 表单布局的 API 文档，addRow 的多种重载和排版策略

[Qt 文档 · QStackedLayout](https://doc.qt.io/qt-6/qstackedlayout.html) -- 页面切换布局的 API 文档，setCurrentIndex / currentChanged 信号

---

到这里，Qt 的布局系统你就入门了。五大布局管理器各有各的适用场景，掌握它们的关键不在于死记 API，而在于拿到一个界面设计稿的时候能快速判断"这里该用什么布局、嵌套关系是怎样的"。下一篇文章我们进入事件处理与传播的机制，那是理解 Qt 应用"怎么响应用户操作"的核心。
