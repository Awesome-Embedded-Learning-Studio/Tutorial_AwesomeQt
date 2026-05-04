# 现代Qt开发教程（新手篇）3.64——QColorDialog：颜色选择对话框

## 1. 前言 / 颜色选择是绘图和编辑类应用的标配

任何涉及视觉呈现的应用，几乎都会在某个时刻需要让用户选择颜色。文本编辑器里改字体颜色，绘图工具里选画笔颜色，主题定制器里改强调色，地图应用里选标注颜色，甚至游戏里的角色捏脸也需要选肤色发色。这些场景背后都是同一个需求：弹出一个颜色选择对话框，让用户从中挑选一种颜色。

Qt 提供的 QColorDialog 是一个功能完善的颜色选择对话框，它内建了颜色轮盘（在支持的平台）、HSV/RGB 分量调节滑块、预设色板、HTML 色号输入框，以及一个"从屏幕上取色"的吸管工具。对于绝大多数颜色选择场景，一行 QColorDialog::getColor() 就够了——用户选完点确定，返回一个 QColor，你拿去用就是了。

但 QColorDialog 能做的事情远不止"弹个框选个色"。它支持透明度通道（Alpha），让你可以选择带半透明效果的 RGBA 颜色；它提供 currentColorChanged 信号，让你在用户还没点确定之前就能实时预览颜色变化的效果；它支持自定义预设色板，让你在对话框中放入业务相关的常用颜色。

今天我们从四个方面展开。先看 getColor 静态方法的基本用法和返回值处理，然后研究 setOption(ShowAlphaChannel) 启用透明度通道的方式，接着讨论通过 currentColorChanged 信号实现实时预览，最后看看如何自定义对话框中的预设色板和颜色历史。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QColorDialog 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QColorDialog、QColor、QApplication、QMainWindow、QPushButton、QLabel、QVBoxLayout、QHBoxLayout、QFrame、QPaintEvent、QPainter 和 QResizeEvent。

## 3. 核心概念讲解

### 3.1 getColor 静态方法：弹一个颜色选择器

QColorDialog::getColor() 是最常用的静态方法。它弹出模态的颜色选择对话框，用户选择颜色后点确定返回选中的 QColor，取消则返回一个无效的 QColor。你需要通过 QColor::isValid() 来区分"用户取消了"和"用户选了一个纯黑色的 RGB(0,0,0)"这两种情况。

```cpp
QColor color = QColorDialog::getColor(
    Qt::white,     // 初始颜色
    this,          // 父窗口
    "选择背景颜色"); // 对话框标题

if (color.isValid()) {
    setBackgroundColor(color);
}
```

getColor 的第一个参数是初始颜色——对话框弹出时会预选这个颜色，用户可以在它的基础上调整。如果你的场景是"改颜色"，传入当前颜色作为初始值很合理；如果是"选新颜色"，传一个默认色（比如白色或者上一个选中的颜色）就行。

getColor 有一个重载版本，多了 Options 参数：

```cpp
QColor color = QColorDialog::getColor(
    Qt::red,
    this,
    "选择颜色",
    QColorDialog::ShowAlphaChannel);
```

Options 参数是一个位掩码，支持以下值：

QColorDialog::ShowAlphaChannel — 在对话框中显示透明度（Alpha）滑块，允许用户选择带透明度的颜色。默认不显示 Alpha 通道，所有颜色的 Alpha 值都是 255（完全不透明）。

QColorDialog::NoButtons — 不显示 OK 和 Cancel 按钮。这个选项通常和 currentColorChanged 信号配合使用——在实时预览场景中，用户不需要"确认"操作，颜色变化立即反映到目标控件上，对话框通过别的方式关闭（比如点击对话框外部）。

QColorDialog::DontUseNativeDialog — 不使用系统原生的颜色选择对话框，强制使用 Qt 自己的跨平台颜色选择器。这个选项在你需要 QColorDialog 的所有特性（比如自定义色板、信号连接）时很有用，因为原生对话框可能不支持全部功能。不同平台原生对话框的行为差异比较大——Windows 的颜色选择器没有 Alpha 通道支持，macOS 的颜色面板是独立浮动的，Linux 上取决于桌面环境。

返回值的 isValid() 检查非常关键。getColor 在用户点击 Cancel 或关闭对话框时返回的 QColor 的 valid 状态是 false。如果你不检查直接使用这个颜色，Qt 不会崩溃——无效颜色会被当作黑色处理——但你的业务逻辑可能出错。比如用户"取消"了颜色选择，你的代码却把背景改成了黑色。

### 3.2 ShowAlphaChannel：透明度通道

默认情况下，QColorDialog::getColor() 返回的颜色是完全不透明的（Alpha = 255）。但很多现代应用需要半透明效果——比如浮动通知的背景色、水印文字的颜色、半透明的覆盖层等。要启用透明度选择，在 getColor 的 Options 参数中加上 ShowAlphaChannel：

```cpp
QColor color = QColorDialog::getColor(
    QColor(255, 0, 0, 128),   // 初始颜色: 半透明红色
    this,
    "选择半透明颜色",
    QColorDialog::ShowAlphaChannel);

if (color.isValid()) {
    // color.alpha() 返回 0~255 的透明度值
    qDebug() << "RGBA:" << color.red()
             << color.green() << color.blue()
             << color.alpha();
}
```

启用 ShowAlphaChannel 后，颜色选择对话框中会多出一个 Alpha 滑块，范围 0（完全透明）到 255（完全不透明）。用户可以拖动这个滑块来调整透明度。

需要注意一点：如果你的初始颜色的 Alpha 值不是 255，但不启用 ShowAlphaChannel，对话框会忽略 Alpha 值，把颜色当作完全不透明来处理。反过来，如果你启用了 ShowAlphaChannel 但初始颜色的 Alpha 值是 255，Alpha 滑块会停在右端（完全不透明），用户需要手动拖动才会产生透明效果。

在后续使用带透明度的 QColor 时，你需要确保目标绘制操作支持 Alpha 混合。比如 QPainter 默认支持 Alpha 混合，你可以直接用半透明的 QColor 来填充矩形或者绘制文字。但如果你把颜色传给 QWidget::setStyleSheet() 的 background-color 属性，某些样式可能不支持 RGBA 格式——这时候你需要用 QString("rgba(%1,%2,%3,%4)").arg(r).arg(g).arg(b).arg(a / 255.0) 来手动构建样式表字符串。

### 3.3 currentColorChanged 信号：实时预览

getColor 是模态的——对话框弹出后，用户必须点击 OK 或 Cancel 才能关闭对话框，然后你的代码才能拿到返回值。这种"选完再确认"的交互模式在大多数场景下没问题，但在某些场景下体验不够好。比如你在开发一个绘图工具，用户需要选择画笔颜色。如果每次选颜色都要"打开对话框 -> 选一个颜色 -> 点确定 -> 看效果 -> 不满意再打开对话框"，这个流程太笨重了。

更好的体验是：用户在颜色选择器中拖动滑块或者点击色板的时候，画笔颜色就实时变化，用户可以一边调整一边看到效果。这需要利用 QColorDialog 的 currentColorChanged 信号。

currentColorChanged 在用户修改对话框中的颜色时不断发射——每次拖动滑块、每次点击色板、每次修改 RGB 输入框，都会触发这个信号。你可以连接这个信号来实现实时预览。

```cpp
class ColorPickerWindow : public QMainWindow
{
    Q_OBJECT
public:
    ColorPickerWindow()
    {
        // 创建非模态的 QColorDialog
        m_colorDialog = new QColorDialog(this);
        m_colorDialog->setOption(
            QColorDialog::NoButtons);
        m_colorDialog->setWindowTitle("选择颜色");

        connect(m_colorDialog,
                &QColorDialog::currentColorChanged,
                this,
                &ColorPickerWindow::onColorChanged);
    }

private:
    void onColorChanged(const QColor &color)
    {
        // 实时更新预览
        m_previewWidget->setStyleSheet(
            QString("background-color: %1;")
                .arg(color.name()));
    }

    QColorDialog *m_colorDialog = nullptr;
};
```

这里有几个关键点值得展开。

第一，我们不再用 getColor 静态方法，而是创建了 QColorDialog 的实例。这样才能连接信号。静态方法是阻塞的，exec() 返回之前你做不了任何事情。

第二，我们设置了 QColorDialog::NoButtons 选项——去掉 OK 和 Cancel 按钮。在实时预览场景中，用户不需要"确认"操作，因为颜色变化已经实时反映了。没有按钮的对话框看起来可能有点奇怪，但配合非模态显示（show() 而不是 exec()），它更像一个浮动面板——用户可以一边操作主窗口一边调整颜色。

第三，currentColorChanged 的频率非常高——用户拖动 HSV 色轮时，这个信号可能每秒触发几十次。如果你的预览操作比较耗时（比如需要重新渲染一张大图），考虑加一层防抖——用 QTimer 延迟 50~100 毫秒再更新预览，避免拖动时卡顿。

还有一个信号是 colorSelected，它只在用户最终点击 OK 时触发一次。如果你同时需要"实时预览"和"最终确认"两种逻辑，可以同时连接 currentColorChanged 和 colorSelected。

### 3.4 自定义调色板与颜色历史

QColorDialog 底部有一排预设的颜色格子——这是 Qt 内置的标准色板，包含了常用的基本颜色。但有些应用场景需要自定义的预设色板。比如一个绘图工具可能需要"皮肤色板"（各种肤色）、"自然色板"（天空蓝、草地绿、泥土棕等），或者一个品牌设计工具需要"公司标准色板"（品牌主色、辅助色、中性色）。

QColorDialog 提供了 setCustomColor() 和 customColor() 来管理自定义颜色格子。颜色选择器底部有两组颜色格子：上面一行是标准色（不可修改），下面一行是自定义色（用户最近使用的颜色会保存在这里）。你可以通过 setCustomColor() 来预设这些自定义颜色。

```cpp
// 设置自定义预设色（在弹出对话框之前调用）
// 索引 0~15 对应底部的 16 个自定义颜色格子
QColorDialog::setCustomColor(0, QColor("#FF6B6B"));
QColorDialog::setCustomColor(1, QColor("#4ECDC4"));
QColorDialog::setCustomColor(2, QColor("#45B7D1"));
QColorDialog::setCustomColor(3, QColor("#96CEB4"));
QColorDialog::setCustomColor(4, QColor("#FFEAA7"));
QColorDialog::setCustomColor(5, QColor("#DDA0DD"));
QColorDialog::setCustomColor(6, QColor("#98D8C8"));
QColorDialog::setCustomColor(7, QColor("#F7DC6F"));
```

setCustomColor 是一个静态方法，它修改的是全局的自定义颜色设置——也就是说，修改会影响整个应用中所有后续弹出的 QColorDialog。这些自定义颜色会被 Qt 保存到平台的颜色设置中（在 Windows 上写入注册表，在 Linux 上写入配置文件），应用重启后仍然保留。

自定义颜色格子在对话框底部显示为两行——第一行是标准色（Qt 内置，不可修改），第二行是自定义色。用户可以在使用过程中通过拖拽或者右键操作将当前颜色保存到自定义色格子中。

有一点需要留意：setCustomColor 的索引范围是 0 到 customCount() - 1。customCount() 默认返回 16，也就是说最多可以有 16 个自定义颜色格子。如果你传入越界的索引，setCustomColor 不会报错但也不会生效——这个行为没有任何警告或者断言，所以调试起来可能会让人抓狂。

另一个和颜色历史相关的方法是 QColorDialog::standardColor()，它返回 Qt 内置的标准色列表。这个列表是只读的，你不能修改标准色。

如果你的颜色选择需求超出了 QColorDialog 的能力——比如需要分组色板（皮肤色、自然色、金属色）、需要颜色命名（"品牌红"、"辅助蓝"）、需要从图片中提取调色板——那就需要自己写一个颜色选择面板了。QColorDialog 在设计上追求的是通用性，而不是深度定制。但作为起点，getColor 配合自定义颜色已经能覆盖大部分场景了。

## 4. 踩坑预防

第一个坑是不检查 getColor 返回值的 isValid()。当用户点击 Cancel 或者关闭对话框时，返回的 QColor 是无效的。如果你不检查直接使用，不会崩溃但颜色会被当作黑色处理。这在视觉上可能不明显——特别是在深色主题的应用中——导致你以为是"颜色没变"，实际上是"取消被当成了黑色"。务必用 if (color.isValid()) 来判断。

第二个坑是原生对话框的行为差异。默认情况下 Qt 会使用系统原生的颜色选择对话框——Windows 上是一个老式的颜色选择器，macOS 上是系统的颜色面板，Linux 上取决于桌面环境。原生对话框的 API 有限，可能不支持 ShowAlphaChannel 选项、NoButtons 选项、currentColorChanged 信号等特性。如果你需要这些特性，加上 DontUseNativeDialog 选项强制使用 Qt 自带的颜色选择器。

第三个坑是 currentColorChanged 的触发频率。用户拖动 HSV 色轮时这个信号会高频触发，如果你的槽函数里做了耗时的操作（比如重新渲染大图），会导致界面卡顿。建议在槽函数中使用 QTimer 做防抖，或者只在 colorSelected（最终确认）时才执行重渲染。

第四个坑是自定义颜色的全局影响。setCustomColor 是静态方法，修改的是进程级别的全局设置。如果你在应用的一个模块中设置了自定义颜色，另一个模块弹出的 QColorDialog 也会看到这些颜色。这在大多数情况下是期望的行为（保持一致性），但如果你需要不同的模块有不同的自定义色板，就需要在每次弹出对话框前重新设置。

第五个坑是样式表中的 RGBA 格式。QColor::name() 默认返回的是 "#RRGGBB" 格式，不包含 Alpha 通道。如果你在样式表中需要半透明颜色，需要手动拼接 rgba() 格式：QString("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alphaF())。直接用 color.name() 会丢失透明度信息。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，中央区域包含一个用于预览颜色的 QFrame（占大部分空间）和底部的一排控制按钮。"选择颜色"按钮调用 QColorDialog::getColor() 弹出模态颜色选择对话框，用户确认后更新预览区域的背景色，同时在预览区域上方显示颜色的 HEX、RGB 和 Alpha 值。"选择透明色"按钮在 getColor 中启用 ShowAlphaChannel 选项，初始颜色为半透明红色 rgba(255,0,0,128)，允许用户调整透明度。"实时预览"按钮创建一个非模态的 QColorDialog 实例，设置 NoButtons 选项，连接 currentColorChanged 信号到主窗口的预览更新方法——用户在颜色选择器中拖动滑块时，预览区域的背景色实时变化。

预览区域使用一个自定义的 ColorPreviewWidget（继承 QFrame），重写 paintEvent 在 paintEvent 中同时绘制一个棋盘格背景（用来可视化透明度效果）和用户选择的颜色层。这样当颜色有透明度时，用户可以透过颜色层看到棋盘格——这是所有绘图工具中展示透明度的标准方式。

提示：棋盘格绘制用 QPainter::fillRect 在 paintEvent 中交替绘制浅灰和白色的小方块，然后设置 QPainter 的 compositionMode 为 QPainter::CompositionMode_SourceOver，用选择的颜色（带透明度）覆盖在上面。

## 6. 官方文档参考链接

[Qt 文档 -- QColorDialog](https://doc.qt.io/qt-6/qcolordialog.html) -- 颜色选择对话框类

[Qt 文档 -- QColorDialog::getColor](https://doc.qt.io/qt-6/qcolordialog.html#getColor) -- 获取颜色静态方法

[Qt 文档 -- QColorDialog::setCustomColor](https://doc.qt.io/qt-6/qcolordialog.html#setCustomColor) -- 设置自定义颜色

[Qt 文档 -- QColor](https://doc.qt.io/qt-6/qcolor.html) -- 颜色类

[Qt 文档 -- QPainter](https://doc.qt.io/qt-6/qpainter.html) -- 绘图类

---

到这里，QColorDialog 的核心用法就全部讲完了。getColor 静态方法一行代码搞定最基本的颜色选择需求，ShowAlphaChannel 选项打开了半透明颜色的大门，currentColorChanged 信号让实时预览成为可能，自定义颜色格子让你可以把业务相关的常用色放进对话框。掌握了这些，你的应用就能以合适的方式让用户选择合适的颜色——无论是简单的"选个背景色"还是复杂的"实时调整半透明画笔"。
