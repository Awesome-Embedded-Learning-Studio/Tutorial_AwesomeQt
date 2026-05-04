# 现代Qt开发教程（新手篇）3.11——QWidget 基类：所有控件的根

## 1. 前言 / 为什么你需要搞懂 QWidget 基类

如果你翻开 Qt 的类继承图，你会发现几乎所有的界面控件——QPushButton、QLabel、QLineEdit、QTextEdit、QComboBox——全部直接或间接继承自 QWidget。这意味着 QWidget 提供的那些属性和方法，是所有控件共有的"基础能力"。你在 QPushButton 上能调 `resize()` 和 `move()`，在 QLabel 上也能调，原因不是它们各自实现了一遍，而是它们都从 QWidget 继承了这个能力。

说实话，很多初学者（包括我刚开始学 Qt 的时候）对 QWidget 的认识停留在"一个空白窗口"上——`new QWidget`、`show()`、完事。但 QWidget 作为整个控件体系的根节点，它提供的远不止一个空白画布。窗口的位置和大小控制、显示与隐藏的精确控制、尺寸策略（控件希望多大、能被压缩到多小）、窗口标志（无边框、置顶、工具窗口等）——这些全部是 QWidget 层面的功能，而且你每天都在用，只是可能没意识到它们来自 QWidget。

这篇文章我们深入 QWidget 基类本身，把窗口属性、显示控制、尺寸策略和窗口标志这四大块基础能力彻底搞清楚。这些知识不仅对"直接使用 QWidget 做主窗口"有用，更重要的是它们适用于所有继承自 QWidget 的控件。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QWidget 是 QtWidgets 模块中最基础的类，链接 Qt6::Widgets 即可。QWidget 的窗口属性和尺寸策略在所有桌面平台上的行为基本一致，但窗口标志（WindowFlags）在不同操作系统上的表现会有差异——比如 `Qt::WindowStaysOnTopHint` 在 X11 下需要窗口管理器配合才能生效，Windows 和 macOS 上则完全由系统保证。文章中会在涉及平台差异的地方特别说明。

## 3. 核心概念讲解

### 3.1 窗口属性：resize / move / setWindowTitle / setWindowIcon

这四个方法控制的是窗口作为一个整体的基本属性——大小、位置、标题和图标。它们对顶级窗口（没有 parent 的 QWidget）的效果最直观，但对嵌入在布局中的子控件同样适用，只是子控件的位置和大小通常由布局管理器控制，手动调用 `move()` 或 `resize()` 可能会被布局覆盖。

`resize(int width, int height)` 设置窗口的客户区大小——注意是客户区，不包含窗口边框和标题栏。如果你设置 `resize(800, 600)`，实际窗口在屏幕上占据的面积会比 800x600 大一些（多出来的部分是标题栏和边框）。如果你需要精确控制窗口的总大小（包含边框），应该使用 `setGeometry()`，但这个方法会把位置和大小的设置合在一起，用起来不够直观，大多数情况下 `resize()` 配合 `move()` 是更好的选择。

```cpp
auto *window = new QWidget();
window->resize(800, 600);
window->show();
```

`move(int x, int y)` 设置窗口在屏幕上的位置——注意这里的坐标是窗口左上角（包含边框）相对于屏幕左上角的偏移。在多显示器环境下，坐标空间是所有显示器拼接起来的一个大矩形，副屏幕的坐标可能是负值（在主屏幕左侧）或者很大的正值。

```cpp
// 把窗口移动到屏幕左上角偏右 100、偏下 100 的位置
window->move(100, 100);
```

`setWindowTitle(const QString &)` 设置窗口标题栏上显示的文字。对于顶级窗口，这个文字也会显示在任务栏上。如果窗口是子控件（有 parent），标题不会显示在界面上，但你可以通过 `windowTitle()` 获取它——有些代码会利用这个特性给控件打"标签"。

```cpp
window->setWindowTitle("我的第一个 Qt 应用");
```

`setWindowIcon(const QIcon &)` 设置窗口的图标。图标会显示在标题栏的左侧（Windows 和 Linux 上），也会显示在任务栏上。在 macOS 上窗口标题栏通常不显示自定义图标，但图标会用在 Dock 栏和窗口切换器中。

```cpp
window->setWindowIcon(QIcon(":/icons/app.png"));
```

这四个方法经常在 `main()` 函数或者主窗口的构造函数中一起调用。需要注意的是，对于顶级窗口，你可以在 `show()` 之前设置所有属性，`show()` 之后属性会立即生效。

### 3.2 show() / hide() / setVisible() / raise() / lower()

这五个方法控制的是控件的可见性和层级。看起来简单，但它们之间的微妙关系值得仔细梳理。

`show()` 让控件可见。对于顶级窗口（没有 parent 的 widget），`show()` 不仅让窗口可见，还会触发窗口的第一次显示——创建系统窗口资源、设置窗口标志、发送 `QShowEvent`。对于子控件（有 parent 的 widget），`show()` 只是把它标记为可见，前提是它的父控件也必须是可见的。

`hide()` 让控件不可见。控件被隐藏后不会接收鼠标和键盘事件，也不会参与布局计算。但控件的内存不会被释放——它仍然存在，只是看不见了。你可以随时再调 `show()` 把它显示出来，之前的状态（文本内容、选中状态等）都会保留。

`setVisible(bool visible)` 是 `show()` 和 `hide()` 的统一接口。`setVisible(true)` 等价于 `show()`，`setVisible(false)` 等价于 `hide()`。在通用代码中（比如你需要根据条件决定显示还是隐藏），用 `setVisible()` 比写 `if-else` 分支调 `show()`/`hide()` 更简洁。

```cpp
// 根据复选框状态显示或隐藏某个面板
connect(checkbox, &QCheckBox::toggled, panel, &QWidget::setVisible);
```

`raise()` 把控件提升到同级控件的最顶层。当多个子控件重叠时，`raise()` 会让这个控件盖住其他同级控件。这个方法只影响 Z 序（绘制顺序），不影响键盘焦点或激活状态。

`lower()` 把控件降到同级控件的最底层。被 `lower()` 的控件会被所有同级控件盖住。

```cpp
// 把浮动工具栏提升到最顶层
toolbar->raise();

// 把背景面板降到最底层
backgroundPanel->lower();
```

这里有一个容易混淆的点：`raise()` 和 `lower()` 操作的是"同级"控件之间的层级关系。如果你有一个 QLabel 盖在一个 QPushButton 上面，对 QPushButton 调 `raise()` 会让按钮出现在标签之上。但如果 QLabel 是 QPushButton 的子控件（这在正常开发中不应该发生），`raise()` 就无效了——子控件永远绘制在父控件之上。

另一个需要了解的是 `isHidden()` 和 `isVisible()` 的区别。`isVisible()` 返回 true 要求控件自身可见且所有祖先控件都可见，而 `isHidden()` 只检查控件自身是否被 `hide()` 过。一个自身 `show()` 了但父控件 `hide()` 了的 widget，`isVisible()` 返回 false，`isHidden()` 返回 false。这个区别在处理复杂的界面嵌套时很重要。

### 3.3 尺寸策略：setSizePolicy / setFixedSize / setMinimumSize

尺寸策略（Size Policy）是 Qt 布局系统的核心概念之一。当你把控件放进布局管理器（QHBoxLayout、QVBoxLayout 等）时，布局管理器会根据每个控件的尺寸策略来决定如何分配空间。理解尺寸策略是避免"控件被拉伸变形"或"控件大小不符合预期"的关键。

`setSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical)` 设置控件在水平和垂直方向上的尺寸策略。最常用的几种策略如下。

`QSizePolicy::Fixed` 表示控件的大小是固定的，不会随布局的变化而变化。控件的实际大小由 `sizeHint()` 决定。如果你通过 `setFixedSize()` 设置了固定大小，sizePolicy 会自动变为 Fixed。

`QSizePolicy::Preferred` 表示控件有一个首选大小（`sizeHint()`），但可以缩小到 `minimumSizeHint()`，也可以放大。布局管理器会尽量给控件它的首选大小，但如果空间不够会压缩它，空间多余也会给它分配更多。大多数控件的默认策略是 Preferred。

`QSizePolicy::Expanding` 表示控件希望尽可能多地占据可用空间。和 Preferred 类似，控件有首选大小和最小大小，但当布局中有剩余空间时，Expanding 控件会优先获得这些空间。QTextEdit 的默认策略就是 Expanding——它希望占据尽可能多的可用空间。

`QSizePolicy::Minimum` 表示控件的 `minimumSizeHint()` 就是它想要的大小，它可以被放大但不希望被压缩到比 `minimumSizeHint()` 更小。适合"内容确定、大小固定"的控件，比如 QLabel。

`QSizePolicy::Maximum` 和 Fixed 类似，但允许控件缩小。控件不会变得比 `sizeHint()` 更大。

```cpp
// 标签：有固定大小，不希望被拉伸
label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

// 文本编辑器：希望占据所有可用空间
textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

// 按钮：水平方向可以拉伸，垂直方向保持固定高度
button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
```

`setFixedSize(int width, int height)` 同时设置控件的固定大小和尺寸策略为 Fixed。调用 `setFixedSize(200, 100)` 之后，控件的大小将永远是 200x100，无论布局管理器怎么拉伸。这在你需要"这个控件就是这个大小，不要动它"的场景下非常方便。

```cpp
// 固定大小的按钮
auto *btn = new QPushButton("固定按钮");
btn->setFixedSize(120, 40);
```

`setMinimumSize(int width, int height)` 设置控件允许被压缩到的最小尺寸。布局管理器在分配空间时不会让控件小于这个值。如果你发现某个控件在窗口缩小的时候被压得面目全非，给它设一个 `setMinimumSize()` 通常就能解决。

```cpp
// 编辑器至少 300x200
textEdit->setMinimumSize(300, 200);
```

对应的还有 `setMaximumSize()`，它限制控件不会被放大到超过指定的尺寸。`setFixedSize(w, h)` 本质上就是 `setMinimumSize(w, h)` + `setMaximumSize(w, h)` 的语法糖。

这三个方法之间的关系可以总结为：`minimumSize` 和 `maximumSize` 定义了控件大小的允许范围，`sizePolicy` 定义了控件在这个范围内的"倾向"（希望大还是希望小），`sizeHint()` 定义了控件的"首选"大小。布局管理器在分配空间时会综合考虑所有这些因素。

### 3.4 窗口标志 Qt::WindowFlags

窗口标志是 Qt 对操作系统原生窗口行为的高级抽象。通过在窗口构造时或创建后传入不同的 `Qt::WindowFlags` 组合，你可以控制窗口的外观和行为——是否有边框、是否置顶、是否是工具窗口、是否是弹出窗口等。

窗口标志的设置时机很重要。对于顶级窗口，窗口标志在窗口创建（第一次 `show()`）时就确定了。如果你在 `show()` 之后才调用 `setWindowFlags()`，Qt 会销毁旧的系统窗口并用新的标志重新创建一个——这会导致窗口闪烁一下。所以最佳实践是在构造函数中就设置好窗口标志。

```cpp
class FramelessWindow : public QWidget
{
public:
    explicit FramelessWindow(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        // 无边框窗口
        setWindowFlags(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        resize(400, 300);
    }
};
```

下面是几种常用的窗口标志组合。

`Qt::FramelessWindowHint` 去掉窗口的标题栏和边框，创建一个无边框窗口。无边框窗口无法通过标题栏拖拽移动，也无法通过边框调整大小——这些功能需要你自己实现（通常通过重写 `mousePressEvent`/`mouseMoveEvent` 来模拟拖拽）。无边框窗口通常配合 `Qt::WA_TranslucentBackground` 使用来实现自定义形状的窗口。

`Qt::WindowStaysOnTopHint` 让窗口始终停留在所有其他窗口之上。适用于浮动工具栏、通知弹窗等需要"钉在最上层"的场景。这个标志在 Windows 和 macOS 上很可靠，在 Linux/X11 上需要窗口管理器支持。

`Qt::Tool` 创建一个工具窗口。工具窗口通常比普通窗口的标题栏更窄（取决于平台），在 Windows 任务栏上不会显示独立条目，最小化时是缩到父窗口而不是任务栏。适合做浮动面板、属性检查器等辅助窗口。

`Qt::Dialog` 创建一个对话框窗口。对话框窗口在 Windows 上默认没有最大化和最小化按钮，而且在某些窗口管理器中对话框会保持在父窗口之上。

`Qt::SplashScreen` 创建一个启动画面窗口。启动画面窗口默认无边框，通常在应用启动时显示 logo 和加载进度。

```cpp
// 置顶的浮动工具窗口
auto *toolWindow = new QWidget(parentWidget);
toolWindow->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
toolWindow->show();

// 启动画面
auto *splash = new QWidget;
splash->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
splash->show();
```

你可以在构造函数中传入窗口标志，也可以创建后通过 `setWindowFlags()` 设置。后者会导致窗口被销毁重建，所以如果窗口已经显示在屏幕上了，用 `setWindowFlags()` 会有可见的闪烁。

```cpp
// 构造时传入窗口标志
auto *window = new QWidget(nullptr, Qt::FramelessWindowHint);

// 或者在构造后设置（不推荐在 show() 之后使用）
window->setWindowFlags(Qt::FramelessWindowHint);
```

多个窗口标志用 `|` 运算符组合。比如一个无边框且置顶的窗口：`Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint`。

最后说一下 `Qt::WindowType` 和 `Qt::WindowFlags` 的关系。`Qt::WindowFlags` 是 `QFlags<Qt::WindowType>` 的类型别名，它本质上是一个位掩码。`Qt::Window`、`Qt::Dialog`、`Qt::Tool`、`Qt::SplashScreen` 这些是窗口类型（决定窗口的基本行为），而 `Qt::FramelessWindowHint`、`Qt::WindowStaysOnTopHint` 这些是窗口提示（在窗口类型的基础上微调行为）。你可以同时指定一个窗口类型和多个窗口提示。

## 4. 踩坑预防

第一个坑是在 `show()` 之后调用 `setWindowFlags()`。这会导致窗口被销毁重建，产生可见的闪烁。如果你确实需要动态修改窗口标志，做好心理准备——或者在创建时就确定好所有标志。

第二个坑是 `setFixedSize()` 和布局管理器的冲突。如果你给一个控件设了 `setFixedSize()`，但布局管理器试图给它分配不同的大小，控件会保持固定大小，但布局可能出现意料之外的空白或重叠。通常你应该选择一种方式——要么用固定大小，要么交给布局管理器，不要两者混用。

第三个坑是 `Qt::FramelessWindowHint` 在不同平台上的渲染差异。Windows 上无边框窗口默认没有背景——你需要在 `paintEvent` 中自己画背景，或者设置 `setAutoFillBackground(true)`。macOS 上无边框窗口默认有白色背景。Linux/X11 上行为取决于窗口管理器。为了跨平台一致性，建议始终在 `paintEvent` 中手动绘制背景。

第四个坑是 `minimumSize` 和 `maximumSize` 的值被 `sizePolicy` 的隐式约束覆盖。比如你设了 `setMinimumSize(100, 50)` 但 `sizePolicy` 是 `QSizePolicy::Fixed`，实际的最小大小会被 `sizeHint()` 覆盖。理解这三个概念之间的优先级关系很重要：`minimumSize` / `maximumSize` 是硬约束，`sizePolicy` 是软偏好，`sizeHint()` 是建议值。

第五个坑是在子控件上调用 `move()` 被布局管理器覆盖。如果一个控件已经被加入了布局（通过 `addWidget()` 或 `setLayout()`），那么控件的位置由布局管理器控制，手动 `move()` 的效果会在下一次布局刷新时被覆盖。如果你需要手动控制子控件的位置，不要把它加入布局，或者使用没有布局管理的 QWidget 作为容器。

## 5. 练习项目

我们来做一个综合练习：创建一个 QWidget 主窗口，里面放置六个按钮来演示各种基础功能。第一个按钮"改变大小"每次点击在 600x400 和 400x300 之间切换窗口大小。第二个按钮"移动窗口"每次点击在两个预设位置之间移动。第三个按钮"显示/隐藏面板"切换底部一个 QLabel 面板的可见性。第四个按钮"置顶切换"切换窗口的 `WindowStaysOnTopHint` 标志。第五个按钮"无边框切换"切换 `FramelessWindowHint` 标志（注意会闪烁）。第六个按钮"最小尺寸切换"在设置和取消 minimumSize 之间切换，观察窗口缩小行为的变化。

几个提示：大小切换用 `resize()`，位置切换用 `move()`；面板的显示隐藏用 `setVisible()` 配合一个 bool 状态变量；置顶切换用 `setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint)` 异或切换标志位，然后需要 `show()` 重新显示窗口；无边框切换类似；最小尺寸切换用 `setMinimumSize()` 和 `setMinimumSize(0, 0)` 之间切换。

## 6. 官方文档参考链接

[Qt 文档 · QWidget](https://doc.qt.io/qt-6/qwidget.html) -- QWidget 基类，所有控件的根基

[Qt 文档 · Window Flags](https://doc.qt.io/qt-6/qt.html#WindowType-enum) -- 窗口类型和窗口标志的完整列表

[Qt 文档 · QSizePolicy](https://doc.qt.io/qt-6/qsizepolicy.html) -- 尺寸策略类，包含所有策略枚举的说明

[Qt 文档 · Window and Dialog Widgets](https://doc.qt.io/qt-6/application-windows.html) -- 窗口和对话框的架构说明

[Qt 文档 · Layout Management](https://doc.qt.io/qt-6/layout.html) -- 布局管理概述，解释 sizeHint / sizePolicy / minimumSize 之间的关系

---

到这里，QWidget 基类的核心能力你就掌握了。窗口属性控制位置大小标题图标，show/hide/setVisible 管理可见性，raise/lower 调整层级，sizePolicy/minimumSize/maximumSize 配合布局管理器控制尺寸，WindowFlags 定义窗口的基本行为模式。这些能力不是 QWidget 独有的——它们通过继承传递给了所有 Qt 控件。搞懂 QWidget 的这些基础接口，你在使用任何 Qt 控件时都能做到心中有数。
