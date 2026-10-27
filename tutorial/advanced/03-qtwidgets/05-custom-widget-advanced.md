---
title: "3.5 自定义控件进阶"
description: "同一段 paintEvent 在 Windows 上看着还算正常，拿到 Linux 上配色和字体一起穿帮。sizePolicy().setHeightForWidth(true) 一行链下去，宽高比纹丝不动。Designer 插件改完代码重新编译，设计器里的行为一点没变。这些现象的根子分别在硬编码的绘制参数、QSizePolicy 的拷贝语义和 Designer 插件的加载时机里，本篇用 QStylePainter、QStyleOption、subControlRect、heightForWidth 和 Designer 插件接口逐个讲清。配套示例在 examples/advanced/03-qtwidgets/05-custom-widget-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.5——自定义控件进阶

[入门篇自定义控件绘制](../../beginner/03-qtwidgets/05-custom-widget-paint-beginner.md) 把重写 paintEvent、拿 QPainter 画图形过了一遍，双缓冲防闪烁、sizeHint 配合布局这些也都讲了。咱们画个圆形进度条、画个示意图表，那套手艺确实够用，前提是软件只跑在一种平台上，而且您不介意它跟系统画风各走各的。来看一段再普通不过的 paintEvent：

```cpp
void MyWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#F0F0F0"));
    painter.setPen(QPen(QColor("#0078D7"), 2));
    painter.drawRoundedRect(rect().adjusted(5, 5, -5, -5), 4, 4);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Segoe UI", 10));
    painter.drawText(rect(), Qt::AlignCenter, "OK");
}
```

这段代码在 Windows 上多半还能看，背景色 #F0F0F0 碰巧贴近浅色主题的窗口底色，Segoe UI 也是系统现成的字体。同一份代码拿到 Linux 或 macOS 上编译运行，观感立刻不对了，背景跟系统配色对不上，字体退回兜底字体，圆角和边距也显得突兀。您在开发机上用 Fusion 风格调得再顺眼，换一套系统风格就全变了，控件看上去像从别的操作系统搬来的。

病根在硬编码，颜色、字体、圆角、间距全写死在代码里，QPainter 又是个照单全收的引擎，咱们给它什么它画什么。出路也有现成的，让控件跟着 QStyle 走，用 QStylePainter 代替 QPainter，用 QStyleOption 描述绘制参数，最终的视觉效果交给当前平台的风格引擎决定。本篇把机制看明白：绘制怎么委托给风格引擎，复合控件的区域怎么让风格替咱们量，heightForWidth 怎么让布局尊重宽高比约束，最后用 Designer 插件把控件送进设计器的控件面板，让它从“能画出来”走到“工程级可发布”。

## 把“长什么样”交给风格引擎：QStylePainter 与 QStyleOption

QPainter 是个纯粹的绘制引擎，咱们给它什么颜色、什么画笔、什么坐标，它就原封不动地画什么，完全不关心当前平台长什么样。QStylePainter 是 QPainter 的子类，内部持有一个 QStyle 指针，提供 `drawPrimitive()`、`drawControl()`、`drawComplexControl()` 这些方法，把绘制请求委托给当前平台的 QStyle 实现。咱们要做的只是准备一个 QStyleOption 对象，描述“要画什么”和“当前状态”，剩下的交给风格：

```cpp
void MyButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    QStyleOptionButton option;
    option.initFrom(this);
    option.text = this->text();
    option.state |= QStyle::State_Sunken;
    painter.drawControl(QStyle::CE_PushButton, option);
}
```

`option.initFrom(this)` 从控件上自动提取的信息不少，enabled 状态、窗口激活状态、布局方向、调色板、字体，全都填进 option。之后 QStylePainter 调用 drawControl()，当前平台的 QStyle 实现就根据这些信息，挑出合适的颜色、边框、动画效果。同样的代码，在 Windows 上画出 Windows 风格的按钮，在 macOS 上画出 macOS 风格的按钮，咱们一行平台判断都不用写。QStyleOption 还有大量子类对应不同的控件类型，咱们按控件类型挑合适的子类来填参数：

| QStyleOption 子类    | 对应的控件类型 |
| -------------------- | -------------- |
| QStyleOptionButton   | 按钮           |
| QStyleOptionMenuItem | 菜单项         |
| QStyleOptionTab      | 标签页         |

忘了调 `initFrom(this)` 的话，option 里留下的就是默认值，state 是零值的 State_None，rect 是空 QRect，palette、fontMetrics 只落到应用级的共享默认，控件自己设置的字体和调色板进不来，QStyle 绘制时拿不到正确的控件状态。rect 一空，内容就直接画进一块空矩形里。State_Enabled 一缺失，禁用状态的控件画出来跟启用状态一模一样。各平台风格消费这些字段的方式又不相同，绘制结果跟着不一致。咱们不妨把动作固定下来，每次创建 QStyleOption 对象，第一件事就是调 initFrom(this)。

配套示例在 `examples/advanced/03-qtwidgets/05-custom-widget-advanced/`，`cmake -B build && cmake --build build` 构建后运行。示例是叫 StyledGauge 的环形仪表盘控件，paintEvent 里用的就是 QStylePainter，配一个基类 QStyleOption，调 initFrom(this) 之后，背景轨道的颜色取 opt.palette.mid()，进度弧取 opt.palette.highlight()，深色浅色主题下自动适配。中央文本也是风格感知的画法，用 QStylePainter::drawItemText 代替 QPainter::drawText，它自动带上系统 palette 和启用状态。环形仪表盘没有现成的 QStyle 元素可委托，示例就自己用 drawArc 画弧线，颜色全部取自系统 palette。咱们拖动窗口下方的滑块，进度弧跟着变长，中央的百分比文本一起刷新，连窗口标题都会同步显示当前进度。

## 参数不写死：颜色问控件要，尺寸问风格要

开篇那段 paintEvent 现在可以判了。背景色 #F0F0F0 在 Windows 浅色主题下碰巧接近系统背景，放到 Linux 的 Adwaita 或 macOS 的 Aqua 配色下，它就跟系统对不上了。字体写死了 Segoe UI，Linux 上根本没有这个字体。圆角半径 4px 在某些风格下太窄，某些风格下又太宽。修复也直白，背景色用 `palette().window().color()` 从控件自己的调色板取，字体用 `font()` 取系统字体，间距用 `style()->pixelMetric(QStyle::PM_ButtonMargin)` 问当前风格要。这些参数全部由控件和 QStyle 自动提供，咱们一个都不用猜。

间距之外，复合控件的区域划分更是重灾区。做一个带下拉箭头和文本的 ComboLabel，咱们得把控件矩形划分成文本区域、箭头区域、边框区域，分别绘制。要是硬编码“箭头区域是右边 20 像素”，Fusion 风格下可能没问题，换一套风格，箭头的标准尺寸也许是 16 或 24 像素，硬编码就跟系统风格对不上了。QStyle 提供了 `subControlRect()` 和 `subElementRect()` 帮咱们算这些区域，传入控件的整体矩形和一个 QStyleOptionComplex 对象，QStyle 就按当前风格的度量标准，返回每个子控件的精确矩形：

```cpp
void MyComboLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    QStyleOptionComboBox option;
    option.initFrom(this);
    option.rect = this->rect();
    option.editable = true;
    QRect text_rect = style()->subControlRect(
        QStyle::CC_ComboBox, &option,
        QStyle::SC_ComboBoxEditField, this);
    QRect arrow_rect = style()->subControlRect(
        QStyle::CC_ComboBox, &option,
        QStyle::SC_ComboBoxArrow, this);
    painter.drawComplexControl(QStyle::CC_ComboBox, option);
    painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, m_label);
}
```

切换系统风格时，文本区域和箭头区域的大小自动跟着调整，咱们画的自定义内容始终准确地落在系统风格预留的位置上。`subElementRect()` 用法类似，处理的是更基础的 UI 元素，比如 QStyle::SE_PushButtonContents，它返回按钮内容区域。

## 高度跟着宽度走：heightForWidth 与 sizePolicy 的拷贝语义

有些控件天然是“宽决定高”的。咱们做一个正方形的色块控件，宽度给 200，高度就该是 200，被压缩到 100，高度也得跟着变 100。可布局系统的默认行为是水平和垂直两个方向独立分配空间，它不知道咱们的高度依赖于宽度。

Qt 为此提供了 `heightForWidth()` 机制：重写 `QWidget::heightForWidth(int width) const`，返回给定宽度下的理想高度，同时通过 sizePolicy 把机制启用起来。把它用起来之前，有一处咱们非常容易写错的地方，`sizePolicy().setHeightForWidth(true)` 链式调用不生效。`sizePolicy()` 返回的是值的拷贝，修改拷贝影响不到控件实际持有的 policy，必须取出来、改好、再设回去：

```cpp
explicit SquareColorBlock(QWidget *parent = nullptr) : QWidget(parent)
{
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);   // 必须先取再设
    setSizePolicy(sp);
}
int heightForWidth(int w) const override { return w; }  // 宽高比 1:1
```

只写对函数还不够，另一个失误更隐蔽：heightForWidth() 实现了，QSizePolicy 的 hasHeightForWidth 标志，却没设为 true。heightForWidth() 是 QWidget 的虚函数，重写了它就能返回正确的高度值，可布局系统分配空间时根本不会调用这个函数，除非标志为 true。后果是精心计算的高度值整个被忽略，布局按默认逻辑分配垂直空间，宽高比约束形同虚设。再叠上前面的链式调用问题，两处一起中招，咱们调试半天也找不到原因。

还有个前提值得您确认：标志设对了，控件也得待在尊重这个标志的布局里。QBoxLayout 和 QGridLayout 都会检查它，手动 setGeometry() 管理的布局不会。

配套示例走的就是这套机制。StyledGauge 的构造函数里，新建 QSizePolicy、设好标志、整体设回去三步全在，heightForWidth() 原样返回宽度，1:1 宽高比。咱们运行示例后拖宽窗口试试，布局给仪表盘多少宽度，它就请求多少高度，无论窗口怎么拉伸，它始终保持正方形。sizeHint() 返回 (120, 120)，minimumSizeHint() 返回 (60, 60)，布局系统拿得到完整的尺寸建议。progress 还是带 READ/WRITE/NOTIFY 的完整 Q_PROPERTY，QPropertyAnimation 可以直接驱动它做平滑动画，示例里咱们暂且拿滑块手动驱动。

## 控件进设计器：Designer 插件的接口、部署与缓存

自定义控件写好后，您要是希望它出现在 Qt Designer 的控件面板里，支持拖拽放置和属性编辑，就需要实现一个 Designer 插件。Qt 提供了 `QDesignerCustomWidgetInterface` 接口描述控件的元信息，各方法各管一摊：

| 方法           | 职责                       |
| -------------- | -------------------------- |
| name()         | 控件在设计器里的名字       |
| group()        | 控件面板里所属的分组       |
| icon()         | 面板里显示的图标           |
| toolTip()      | 鼠标悬停时的提示           |
| includeFile()  | 生成代码时要包含的头文件   |
| isContainer()  | 控件能不能当容器承载子控件 |
| createWidget() | 构造控件实例，最核心的一个 |

接口还要求实现 initialize() 和 isInitialized()，用来支持延迟初始化。插件类用 `Q_PLUGIN_METADATA` 宏声明 IID 为 `"org.qt-project.Qt.QDesignerCustomWidgetInterface"`，用 `Q_INTERFACES` 声明实现的接口，两个宏的写法有现成范例，就在 Qt 自家的 QtCharts Designer 插件源码里，文件是 qchartsplugin.h。CMake 配置上要链接 `Qt6::Designer` 和 `Qt6::Widgets`，编译产物必须是动态库，也就是 `.so`、`.dll` 或 `.dylib`，放到 Qt Designer 的插件搜索路径下，文件名以 `designer_` 前缀开头方便识别。搜索路径通过 `QT_PLUGIN_PATH` 环境变量指定，加载状态咱们可以在 Qt Creator 的“关于插件”对话框里确认。多个控件想共用一个插件库，用 `QDesignerCustomWidgetCollectionInterface` 打包。

插件部署之后还有一个加载时机要提前知道，不然调试的时候真能让人怀疑人生：插件注册成功后，Qt Creator 缓存的是旧版本。您修改了控件代码、重新编译、替换了插件文件，Designer 里的行为却一点没变，因为 Designer 插件只在 Creator 启动时加载一次，不会热更新。咱们每次重新编译插件后都要重启 Qt Creator，重启还不行，就清理 Qt Creator 的缓存目录再重启一次，Linux 下通常在 `~/.local/share/data/QtProject/qtcreator`，Windows 下通常在 `%LOCALAPPDATA%\QtProject`。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QStylePainter 和 QStyleOption 属于 QtWidgets 模块，Designer 插件开发需要额外链接 Qt6::Designer 模块，示例在 Windows / Linux / macOS 上均可编译运行，绘制效果随平台风格不同而有差异，这正是咱们追求的效果。

[Qt 文档 · QStylePainter](https://doc.qt.io/qt-6/qstylepainter.html) -- 基于 QStyle 的绘制便捷类，drawPrimitive / drawControl / drawComplexControl 接口

[Qt 文档 · QStyleOption](https://doc.qt.io/qt-6/qstyleoption.html) -- 绘制参数容器，包含 state、palette、rect 等所有 QStyle 需要的信息

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 抽象风格基类，subControlRect / subElementRect / pixelMetric 等核心方法

[Qt 文档 · QSizePolicy](https://doc.qt.io/qt-6/qsizepolicy.html) -- hasHeightForWidth 标志和六种空间策略的完整说明

[Qt 文档 · QDesignerCustomWidgetInterface](https://doc.qt.io/qt-6/qdesignercustomwidgetinterface.html) -- Qt Designer 自定义控件插件接口

[Qt 文档 · Styles and Style Aware Widgets](https://doc.qt.io/qt-6/style-reference.html) -- 自定义控件如何正确使用 QStyle 的完整指南

---

自定义控件的进阶就到这里。想再动手的，请您把 StyledGauge 完整玩一轮：拖动滑块，看进度弧和中央百分比怎么跟着走，再拉伸窗口，验证正方形约束是不是稳稳当当，最后把 paintEvent 里的 opt.palette.mid() 换成别的角色，重新编译运行，看仪表盘配色怎么跟着系统调色板变。上一篇 [3.4 QSS 进阶](./04-qss-advanced.md) 讲样式表的特异性边界和主题刷新，下一篇 [3.6 对话框进阶](./06-dialog-advanced.md) 讲模态范围、输入验证和位置记忆，控件之外，Qt 还有一整套现成的对话框体系等着咱们。
