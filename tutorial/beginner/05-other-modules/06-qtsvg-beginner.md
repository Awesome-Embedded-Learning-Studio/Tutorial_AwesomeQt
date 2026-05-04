# 现代Qt开发教程（新手篇）5.6——QtSvg 矢量图形基础

## 1. 前言：位图缩放的那道坎，SVG 帮你跨过去

做 GUI 开发迟早会遇到图标模糊的问题。你画了一个 32x32 的 PNG 图标，在 1080p 屏幕上看着挺清晰，放到 4K 屏幕上或者高 DPI 缩放下就糊成马赛克了。换一张 128x128 的？在普通屏幕上又显得过大，还得写一堆缩放逻辑。这个问题的根本原因是位图的像素是固定的——放大就是插值，插值就是模糊，没有例外。

SVG（Scalable Vector Graphics）从根本上解决了这个问题。矢量图存储的是绘制路径而不是像素点，不管放大到什么尺寸，渲染引擎都会重新计算路径，永远不会模糊。Qt 的 QtSvg 模块提供了对 SVG 的完整支持——从最简单的"显示一个 SVG 文件"到在 QPainter 中灵活渲染 SVG、动态修改 SVG 颜色、把 SVG 用作高 DPI 图标。这篇我们要做的是把 QtSvg 的核心 API 全部摸一遍，从 QSvgWidget 直显到 QSvgRenderer 深度渲染，一条线走到底。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 `Qt6::Svg`、`Qt6::SvgWidgets` 和 `Qt6::Widgets` 三个模块。CMake 配置需要 `find_package(Qt6 REQUIRED COMPONENTS Core Svg SvgWidgets Widgets)`。QtSvg 在 Qt 6 中被拆分成了两个模块：`Qt6::Svg` 提供核心的 `QSvgRenderer` 类（不依赖 Widgets），`Qt6::SvgWidgets` 提供 `QSvgWidget` 控件（依赖 Widgets）。如果你的项目只需要渲染 SVG 到自定义画布而不需要专门的 SVG 控件，只引入 `Qt6::Svg` 就够了。

Qt 6 的 SVG 支持基于一个内置的 SVG 解析器，支持 SVG 1.1 规范的绝大部分特性，包括路径、渐变、裁剪、遮罩、滤镜等。对于 CSS 动画和 JavaScript 交互这类 Web 特性，QtSvg 不支持——它毕竟不是一个浏览器引擎。编译工具链方面，MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 QSvgWidget——最简单的 SVG 显示方式

如果你只需要在界面上显示一个 SVG 文件，什么都不用做，`QSvgWidget` 直接搞定。它是一个 QWidget 子类，内部封装了 `QSvgRenderer`，提供了加载和显示 SVG 的快捷接口：

```cpp
#include <QSvgWidget>

// 从文件加载
QSvgWidget *svgWidget = new QSvgWidget(":/icons/logo.svg", this);
svgWidget->setFixedSize(200, 200);

// 或者运行时加载
svgWidget->load(QString(":/icons/arrow.svg"));
svgWidget->load(QByteArray(svgData));  // 也可以从 QByteArray 加载
```

`QSvgWidget` 会自动缩放 SVG 内容以填满控件区域，保持宽高比。你不需要手动处理缩放逻辑——不管控件多大，SVG 都会重新渲染，不会模糊。`setFixedSize()` 固定控件尺寸，如果你希望 SVG 跟随窗口缩放，把它放到布局里就行。

`QSvgWidget` 还提供了 `renderer()` 方法，返回内部的 `QSvgRenderer` 指针。这意味着你可以先用 `QSvgWidget` 快速显示 SVG，然后通过 `renderer()` 获取底层渲染器来做更高级的操作（比如访问特定元素、查询尺寸等），不需要重新创建渲染器。

有一点需要注意：`QSvgWidget` 默认的 sizeHint 是 SVG 文件的原始尺寸（viewBox 大小）。如果你不设置固定尺寸，它可能会撑得很大。在布局中使用时建议设置 sizePolicy 为 `Ignored` 或者手动设置最大尺寸，避免 SVG 把整个布局撑坏。

### 3.2 QSvgRenderer——在 QPainter 中灵活渲染 SVG

`QSvgWidget` 适合简单的"显示"场景，但如果你需要在自定义的绘图逻辑中使用 SVG——比如在 `QPainter` 画布上渲染 SVG、把 SVG 画到 `QPixmap` 或 `QImage` 上、或者在 `QPaintDevice` 的某个特定区域渲染 SVG——就需要直接使用 `QSvgRenderer`。

`QSvgRenderer` 是 QtSvg 的核心引擎。它负责解析 SVG 文件并提供渲染接口：

```cpp
#include <QSvgRenderer>
#include <QPainter>

// 加载 SVG
QSvgRenderer renderer(QString(":/icons/gear.svg"));

if (!renderer.isValid()) {
    qWarning() << "Failed to load SVG";
    return;
}

// 在 QImage 上渲染（用于生成高分辨率位图）
QImage image(256, 256, QImage::Format_ARGB32);
image.fill(Qt::transparent);

QPainter painter(&image);
renderer.render(&painter);  // 渲染到整个画布
painter.end();

image.save("gear_256.png");
```

`render()` 有两个重载版本。无参版本渲染到整个 `QPaintDevice`，带 `QRectF` 参数的版本渲染到指定矩形区域：

```cpp
// 渲染到指定区域（自动缩放）
renderer.render(&painter, QRectF(10, 10, 100, 100));
```

这种在 `QPainter` 中渲染 SVG 的方式非常灵活。你可以在同一个 `QPainter` 上画多个 SVG，可以和 `drawText()`、`drawRect()` 等其他绘图操作混用，还可以利用 QPainter 的变换矩阵（translate、rotate、scale）对 SVG 做旋转、缩放等操作。

一个常见的用途是把 SVG 渲染到 `QPixmap` 上用作图标。这种方式结合了 SVG 的矢量优势和 `QPixmap` 的显示效率——渲染一次后，后续的显示操作使用缓存的位图，不需要每次都重新解析 SVG：

```cpp
QPixmap renderSvgToPixmap(const QString &svgPath, int size)
{
    QSvgRenderer renderer(svgPath);
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}

// 使用
QPixmap icon = renderSvgToPixmap(":/icons/close.svg", 32);
button->setIcon(icon);
```

### 3.3 SVG 动态着色——修改 SVG 的颜色

SVG 文件本质上是 XML 文本，里面的颜色是通过 `fill`、`stroke` 等 CSS 属性指定的。这意味着你可以通过修改 SVG 文本来实现动态着色——比如根据主题色动态改变图标的颜色。

最直接的方式是字符串替换。假设你的 SVG 文件中使用了特定的占位色（比如 `#FF0000` 或者一个 CSS class），你可以加载 SVG 文本后替换颜色：

```cpp
QString recolorSvg(const QString &svgPath, const QColor &newColor)
{
    QFile file(svgPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QString svgContent = QString::fromUtf8(file.readAll());

    // 替换所有 fill="#FF0000" 为新颜色
    QString colorHex = newColor.name();  // 如 "#00FF00"
    svgContent.replace("fill=\"#FF0000\"", QString("fill=\"%1\"").arg(colorHex));

    return svgContent;
}

// 使用
QString coloredSvg = recolorSvg(":/icons/heart.svg", QColor("#E74C3C"));
QSvgRenderer renderer(coloredSvg.toUtf8());
```

这种方式简单粗暴，但对简单的 SVG 够用了。如果你的 SVG 结构比较复杂（用了 CSS class、渐变、多个颜色），推荐使用 `QSvgRenderer` 的 `setViewBox()` 配合 `QPainter` 的 `setOpacity()` 或者使用 CSS class 来区分不同可着色区域。

另一个更优雅的方案是在 SVG 中使用 `currentColor` 关键字。`currentColor` 是 CSS 规范中定义的特殊颜色值，它会继承父元素的 `color` 属性。QtSvg 支持通过 `QSvgRenderer` 设置默认的 CSS 样式。如果你的 SVG 图标设计时就使用了 `fill="currentColor"`，那么着色就变得非常简单：

```xml
<!-- icon.svg -->
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
  <path fill="currentColor" d="M12 2L2 7l10 5 10-5-10-5z"/>
</svg>
```

不过 QtSvg 对 CSS 样式继承的支持有限，实际项目中更可靠的做法还是直接替换 SVG 文本中的颜色值。

### 3.4 SVG 在图标系统中的使用——高 DPI 适配

在高 DPI 屏幕上，位图图标需要提供多套分辨率（1x、2x、3x），维护成本很高。SVG 天然支持任意缩放，一套 SVG 文件就能适配所有分辨率。

在 Qt 的图标系统中使用 SVG 有两种方式。第一种是直接把 SVG 文件加入 Qt 资源系统（.qrc），然后通过文件路径加载。第二种是把 SVG 渲染成 `QPixmap` 后设置到 `QIcon` 上。

推荐的做法是为每个需要的尺寸渲染一次 SVG 并缓存：

```cpp
#include <QIcon>
#include <QPixmapCache>

QIcon createSvgIcon(const QString &svgPath)
{
    QIcon icon;

    // 为常见 DPI 缩放因子渲染多个尺寸
    const QList<int> sizes = {16, 24, 32, 48, 64};

    QSvgRenderer renderer(svgPath);
    if (!renderer.isValid()) {
        return icon;
    }

    for (int size : sizes) {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        renderer.render(&painter);
        painter.end();

        icon.addPixmap(pixmap);
    }

    return icon;
}
```

Qt 的 `QIcon` 会根据当前控件的设备像素比（`devicePixelRatio`）自动选择最合适尺寸的 pixmap。在高 DPI 屏幕上，一个 32px 的控件实际需要 64px 的像素数据（2x 缩放），`QIcon` 会自动选择 64px 的那个 pixmap。如果你使用 SVG 渲染，所有的尺寸都是矢量计算出来的，不会有模糊的问题。

更高级的方案是结合 `QPixmapCache` 做全局缓存，避免重复渲染：

```cpp
QPixmap renderCachedSvg(const QString &svgPath, int size)
{
    QString cacheKey = svgPath + QString("_%1").arg(size);

    QPixmap cached;
    if (QPixmapCache::find(cacheKey, &cached)) {
        return cached;
    }

    QSvgRenderer renderer(svgPath);
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(1.0);  // 不让 Qt 二次缩放
    QPixmapCache::insert(cacheKey, pixmap);

    return pixmap;
}
```

这种缓存策略在图标密集的界面（工具栏、列表视图）中非常有效——首次渲染后，后续的图标加载基本是零开销的。

### 3.5 访问 SVG 内部元素

`QSvgRenderer` 提供了访问 SVG 内部 DOM 元素的能力。通过 `elementId`，你可以获取 SVG 中特定元素的边界矩形，或者只渲染 SVG 的某一部分：

```cpp
QSvgRenderer renderer(QString(":/icons/complex.svg"));

// 检查元素是否存在
if (renderer.elementExists("arrow-left")) {
    // 获取元素的边界矩形（SVG 坐标系）
    QRectF bounds = renderer.boundsOnElement("arrow-left");

    // 只渲染指定元素
    QImage image(64, 64, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter, "arrow-left");
    painter.end();
}
```

这个功能在 SVG sprite sheet 中特别有用。你可以把多个图标放在同一个 SVG 文件中，每个图标用 `<g id="icon-name">` 分组，然后通过 `elementId` 单独渲染每一个。这样做的好处是只需要加载和解析一次 SVG 文件，就能获取里面所有图标，减少了文件 I/O 和解析开销。

SVG 文件的结构大概是这样：

```xml
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128">
  <g id="icon-home">
    <path d="M10 30 L30 10 L50 30 L50 50 L10 50 Z"/>
  </g>
  <g id="icon-settings">
    <circle cx="30" cy="80" r="15"/>
  </g>
  <g id="icon-close">
    <line x1="80" y1="10" x2="110" y2="40"/>
    <line x1="110" y1="10" x2="80" y2="40"/>
  </g>
</svg>
```

加载后用 `renderer.render(&painter, "icon-home")` 就只渲染 home 图标。这种 sprite 技术在 Web 前端里很常见，在 Qt 里同样适用。

## 4. 综合示例：SVG 查看器与着色工具

把前面学的串起来，我们写一个 SVG 查看器，支持加载显示、缩放、动态着色和元素提取。程序使用 `QSvgWidget` 显示 SVG，通过 `QSvgRenderer` 实现着色和元素访问，提供缩放控制和颜色修改功能。

完整代码见 `examples/beginner/05-other-modules/06-qtsvg-beginner/`，下面是关键部分的讲解。

CMake 配置需要引入 Svg 和 SvgWidgets：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Svg SvgWidgets Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Svg Qt6::SvgWidgets Qt6::Widgets)
```

程序结构：中央是 `QSvgWidget` 显示 SVG，右侧面板包含缩放滑块、颜色选择器、元素列表、打开文件按钮。

SVG 加载与显示的核心代码：

```cpp
// 打开文件
connect(openButton, &QPushButton::clicked, this, [this, svgWidget]() {
    QString file = QFileDialog::getOpenFileName(
        this, "选择 SVG 文件", {},
        "SVG Files (*.svg);;All Files (*)");
    if (!file.isEmpty()) {
        currentSvgPath_ = file;
        svgWidget->load(file);
        updateElementList();
    }
});
```

动态着色的实现——读取 SVG 文本，替换所有 `fill` 颜色：

```cpp
void applyColor(QSvgWidget *widget, const QString &svgPath, const QColor &color)
{
    QFile file(svgPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QString content = QString::fromUtf8(file.readAll());

    // 使用正则替换 fill="..." 属性
    QRegularExpression regex(R"(fill="[^"]*")");
    content.replace(regex, QString("fill=\"%1\"").arg(color.name()));

    widget->load(content.toUtf8());
}
```

元素列表通过 `QSvgRenderer` 获取：

```cpp
void updateElementList()
{
    QSvgRenderer *renderer = svgWidget_->renderer();
    // QSvgRenderer 没有直接列出所有元素 ID 的方法
    // 可以通过解析 SVG 的 XML 结构来获取
    QFile file(currentSvgPath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QXmlStreamReader xml(&file);
    elementList->clear();
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            QString id = xml.attributes().value("id").toString();
            if (!id.isEmpty()) {
                elementList->addItem(id);
            }
        }
    }
}
```

缩放控制通过调整 `QSvgWidget` 的固定尺寸实现：

```cpp
connect(zoomSlider, &QSlider::valueChanged, this,
    [this, svgWidget](int value) {
        double scale = value / 100.0;
        int w = static_cast<int>(svgWidget_->sizeHint().width() * scale);
        int h = static_cast<int>(svgWidget_->sizeHint().height() * scale);
        svgWidget_->setFixedSize(w, h);
    });
```

运行程序后，点击"打开文件"选择一个 SVG 文件，它会在界面中央显示。拖动缩放滑块可以放大缩小——你会发现不管怎么放大，SVG 的线条始终清晰锐利，不会出现任何锯齿或模糊。使用颜色选择器可以动态修改 SVG 的填充色，点击元素列表中的条目可以查看 SVG 中各元素的 ID。

## 5. 练习项目

练习项目：SVG 图标管理器。

我们要做一个 SVG 图标管理工具，支持加载一个包含多个图标的 SVG sprite 文件，自动提取所有带 `id` 的 `<g>` 元素，以网格形式展示所有图标，支持点击导出单个图标为 PNG。

完成标准是这样的：使用 `QXmlStreamReader` 解析 SVG 文件，提取所有带 `id` 属性的元素；使用 `QSvgRenderer::render(painter, elementId)` 逐个渲染图标到 `QPixmap`；图标以 `QGridLayout` 网格排列，每个图标下方显示元素 ID；双击图标弹出对话框，可选择导出尺寸（16/24/32/48/64/128/256），导出为 PNG。

几个实现提示：使用 `QSvgRenderer::boundsOnElement(elementId)` 获取每个图标的实际边界，渲染时根据边界计算正确的缩放比例；网格布局可以用 `QScrollArea` + `QGridLayout`，每格放一个 `QLabel`（显示图标）+ 一个 `QLabel`（显示名称）；导出时创建一个新的 `QImage`，用 `QSvgRenderer` 渲染指定元素到指定尺寸，然后 `QImage::save()` 保存为 PNG。

## 6. 官方文档参考

[Qt 文档 · QtSvg 模块](https://doc.qt.io/qt-6/qtsvg-index.html) -- SVG 模块总览

[Qt 文档 · QSvgRenderer](https://doc.qt.io/qt-6/qsvgrenderer.html) -- SVG 渲染引擎

[Qt 文档 · QSvgWidget](https://doc.qt.io/qt-6/qsvgwidget.html) -- SVG 显示控件

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了！QtSvg 模块虽然不大，但它的定位非常精准——解决矢量图形在 Qt 应用中的显示和渲染问题。QSvgWidget 负责简单场景的"打开就显示"，QSvgRenderer 负责深度场景的灵活渲染和元素访问。掌握了这两个类之后，在你的项目中使用 SVG 替代位图图标就不再是难事了。一套 SVG 走天下，不管屏幕分辨率怎么变，图标永远清晰锐利。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
