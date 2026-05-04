# 现代Qt开发教程（新手篇）5.7——QtPrintSupport 打印体系概览

## 1. 前言：打印这件事，比你想象的要通用

在桌面应用开发里，打印功能经常被当成"最后再加"的需求。很多人的做法是——用 QTextDocument 或者 QTextEdit 自带的 print() 方法凑合一下，跑通了就算完事。但当你真正需要打印自定义报表、图文混排的页面、或者导出 PDF 的时候，才会发现 Qt 的打印体系远不止一个 print() 调用那么简单。

Qt 的 QtPrintSupport 模块提供了一套统一的打印抽象：不管你是要把内容送到物理打印机，还是导出成 PDF 文件，底层走的都是同一套 QPainter 渲染管线。这意味着我们之前学过的所有 QPainter 绘图技巧——画文字、画图片、画矢量图形——全部可以直接用在打印场景里，几乎不需要改任何渲染代码。这篇我们要做的是把 QPrinter、QPrintDialog、QPrintPreviewDialog 这三个核心类的协作关系彻底搞清楚，然后手写一个完整的打印示例，把"打印到纸张"和"导出到 PDF"两条路径都走通。

跨平台打印也有不少坑。Windows 上走的是 GDI/Win32 打印 API，macOS 上走的是 Cocoa 的 NSPrintOperation，Linux 上走的是 CUPS（Common UNIX Printing System）。不同平台对纸张大小、边距、分辨率的支持都有微妙的差异，这些我们在最后一节统一梳理。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 `Qt6::PrintSupport` 和 `Qt6::Widgets` 两个模块。CMake 配置很简单：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core PrintSupport Widgets)
```

QtPrintSupport 在 Qt 6 中没有经历大的 API 变动，核心类依然是 QPrinter、QPrintDialog、QPrintPreviewDialog 和 QPrintPreviewWidget。编译工具链方面，MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。

有一点需要提前说明：QtPrintSupport 依赖 Widgets 模块。如果你的项目是纯 QML 应用，打印功能需要通过 C++ 后端暴露给 QML 层——Qt 没有提供原生的 QML 打印 API。这一点在设计架构的时候要提前考虑好。

## 3. 核心概念讲解

### 3.1 QPrinter——打印体系的起点

QPrinter 是整个打印流程的核心对象，你可以把它理解成一个"虚拟的纸张"。它封装了打印机相关的所有配置：纸张大小（A4、Letter 等）、页面方向（纵向/横向）、页边距、分辨率（DPI）、输出格式（送到打印机还是输出到 PDF 文件）。不管后续使用 QPrintDialog 还是 QPrintPreviewDialog，最终渲染目标都是一个 QPrinter 对象。

创建 QPrinter 的方式取决于你的用途。如果是要送到物理打印机，使用默认构造就行：

```cpp
QPrinter printer(QPrinter::HighResolution);
```

`QPrinter::HighResolution` 这个参数告诉 QPrinter 使用打印机支持的最高分辨率。如果你选择 `QPrinter::ScreenResolution`，QPrinter 会使用屏幕分辨率（通常是 96 DPI），在纸上打印出来会很模糊。除非你有特殊理由，打印场景下始终使用 HighResolution。

如果你要导出 PDF，只需要设置输出格式和文件路径：

```cpp
QPrinter printer(QPrinter::HighResolution);
printer.setOutputFormat(QPrinter::PdfFormat);
printer.setOutputFileName("report.pdf");
```

设置完之后，这个 QPrinter 对象的行为就和"打印机"完全一致了——你用 QPainter 往上面画东西，画完之后内容会被写入 PDF 文件。这是 Qt 打印体系最优雅的地方：打印和导出 PDF 是完全统一的代码路径，区别仅仅是 QPrinter 的输出格式不同。

QPrinter 的常用配置项包括 setPageSize() 设置纸张大小、setPageOrientation() 设置页面方向、setPageMargins() 设置页边距。这些配置既可以通过代码硬编码，也可以让用户在 QPrintDialog 中自行选择。

### 3.2 QPrintDialog——让用户选择打印机的标准对话框

QPrintDialog 是 Qt 提供的标准打印对话框，功能类似于你在其他应用里按 Ctrl+P 弹出的那个对话框。它会列出系统中所有可用的打印机，允许用户选择打印机、设置纸张大小、页边距、打印份数、打印范围等。

使用方式非常直接：

```cpp
QPrinter printer(QPrinter::HighResolution);

QPrintDialog dialog(&printer, parentWidget);
dialog.setWindowTitle("打印文档");

if (dialog.exec() == QDialog::Accepted) {
    // 用户点了"打印"，printer 已被用户配置更新
    // 现在用 QPainter 在 printer 上渲染内容
    QPainter painter(&printer);
    painter.drawText(100, 100, "Hello from QtPrintSupport");
    painter.end();
}
```

exec() 返回 Accepted 表示用户点击了打印按钮，此时 printer 对象已经被对话框更新了——纸张大小、页边距、选中的打印机等都已设置好。你只需要创建 QPainter 开始画就行了。如果 exec() 返回 Rejected，表示用户取消了打印，什么都不用做。

QPrintDialog 在不同平台上的外观是不一样的。Windows 上它会调用系统的原生打印对话框，macOS 上会弹出 Cocoa 风格的打印面板，Linux 上会显示基于 CUPS 的 Qt 对话框。功能上也略有差异——比如 Linux 上的对话框可能不支持某些高级特性（如双面打印设置），这取决于 CUPS 的配置和打印驱动。

### 3.3 QPrintPreviewDialog——所见即所得的打印预览

实际开发中，直接把内容送到打印机往往会遇到排版问题——文字位置不对、图片被裁切、页边距不对。QPrintPreviewDialog 提供了一个预览界面，让你在真正打印之前看到每一页的渲染效果，并且可以直接在预览界面中调整缩放、翻页查看。

QPrintPreviewDialog 的使用模式和 QPrintDialog 有所不同，它不是通过 exec() 一次性获取结果，而是通过信号槽机制驱动渲染：

```cpp
QPrinter printer(QPrinter::HighResolution);

QPrintPreviewDialog preview(&printer, parentWidget);
preview.setWindowTitle("打印预览");

// 连接 paintRequested 信号——每当预览需要刷新时触发
QObject::connect(&preview, &QPrintPreviewDialog::paintRequested,
                 [](QPrinter *printer) {
    QPainter painter(printer);
    // 在这里渲染你的内容
    painter.drawText(100, 100, "预览内容");
    painter.end();
});

preview.exec();
```

关键在于 paintRequested 信号。当预览对话框打开时，它会发出 paintRequested 信号，参数是一个 QPrinter 指针。我们需要在槽函数里用 QPainter 往这个 QPrinter 上画内容。预览对话框内部会把 QPainter 的输出截获并渲染到预览窗口中。用户在预览中翻页、缩放时，对话框会重新发出 paintRequested，让你重新渲染。

这意味着你的渲染逻辑需要放在一个独立的函数里，既能被打印对话框调用，也能被预览对话框调用。这个设计其实很合理——渲染逻辑只写一份，打印和预览复用同一份代码。

### 3.4 打印与导出 PDF 的统一流程

把前面的知识串起来，一个完整的"打印或导出 PDF"流程是这样的：

```cpp
void renderDocument(QPrinter *printer)
{
    QPainter painter(printer);

    // 页面矩形（考虑了页边距）
    QRectF pageRect = printer->pageRect(QPrinter::DevicePixel);

    // 第一页
    painter.drawText(pageRect, Qt::AlignTop | Qt::AlignLeft,
                     "第一页标题");
    // ... 更多渲染代码 ...

    // 如果需要打印多页，调用 newPage() 开始新的一页
    printer->newPage();
    painter.drawText(pageRect, Qt::AlignTop | Qt::AlignLeft,
                     "第二页标题");
    // ... 更多渲染代码 ...

    painter.end();
}
```

然后根据用户选择调用不同的入口：

```cpp
// 路径一：打印到物理打印机
void printToPrinter(QWidget *parent)
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, parent);
    if (dialog.exec() == QDialog::Accepted) {
        renderDocument(&printer);
    }
}

// 路径二：导出 PDF
void exportToPdf(const QString &filePath)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    renderDocument(&printer);
}

// 路径三：带预览的打印
void printWithPreview(QWidget *parent)
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, parent);
    QObject::connect(&preview, &QPrintPreviewDialog::paintRequested,
                     renderDocument);
    preview.exec();
    // 用户在预览中点"打印"后，内容会发送到 printer 指向的设备
}
```

三条路径共享同一个 renderDocument() 函数，唯一的区别是 QPrinter 的配置方式。这个统一的渲染接口是 QtPrintSupport 最实用的设计模式。

### 3.5 跨平台打印差异

QtPrintSupport 在三个桌面平台上的行为基本一致，但有一些值得注意的差异。

Windows 平台上，QPrintDialog 默认使用原生 Win32 打印对话框，外观和系统一致，支持所有 Windows 打印特性（双面打印、装订、颜色/灰度切换等）。QPrinter 底层使用 GDI 或 GDI+ 进行渲染，分辨率通常取决于打印机驱动报告的值。

macOS 平台上同样使用原生 Cocoa 打印对话框，QPrinter 底层通过 NSPrintOperation 提交打印任务。macOS 的打印系统对 PDF 有原生支持，所以导出 PDF 的质量非常好。

Linux 平台上，QtPrintSupport 依赖 CUPS。如果系统没有安装 CUPS，QPrintDialog 可能无法列出任何打印机。大部分桌面发行版默认安装了 CUPS，但在嵌入式 Linux 或最小化安装的环境中需要手动安装。Linux 上的打印分辨率由 CUPS 驱动决定，某些打印机驱动可能不支持 HighResolution 模式。

一个常见的跨平台坑是纸张大小的默认值。不同地区的系统默认纸张不一样——美国默认 Letter（8.5x11 英寸），中国和欧洲默认 A4（210x297 毫米）。如果你的应用需要精确控制排版，最好在代码中显式设置纸张大小，而不是依赖系统默认值：

```cpp
printer.setPageSize(QPageSize(QPageSize::A4));
```

## 4. 综合示例：图文报表打印与 PDF 导出工具

把前面学的串起来，我们写一个简单的报表打印工具。程序提供一个文本编辑区和一个图片加载区，支持三种输出方式：直接打印、导出 PDF、打印预览。渲染逻辑统一放在一个函数里，三条路径共享。

完整代码见 `examples/beginner/05-other-modules/07-qtprintsupport-beginner/`，下面是关键部分的讲解。

CMake 配置只需要 PrintSupport 和 Widgets：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core PrintSupport Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::PrintSupport Qt6::Widgets)
```

程序的核心是 renderDocument() 函数，它负责将文本和图片渲染到 QPrinter 上。这个函数既要处理单页内容，也要处理内容超出单页时自动分页的逻辑：

```cpp
void renderDocument(QPrinter *printer, const QString &title,
                    const QString &bodyText, const QPixmap &image)
{
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF page = printer->pageRect(QPrinter::DevicePixel);
    qreal y = 0;

    // ---- 标题 ----
    QFont titleFont("Sans", 18, QFont::Bold);
    painter.setFont(titleFont);
    QRectF titleRect = QRectF(page.left(), y, page.width(), 40);
    painter.drawText(titleRect, Qt::AlignCenter, title);
    y += 50;

    // ---- 分隔线 ----
    painter.drawLine(page.left(), y, page.right(), y);
    y += 10;

    // ---- 正文（自动换行） ----
    QFont bodyFont("Sans", 10);
    painter.setFont(bodyFont);
    QRectF textRect(page.left(), y, page.width(), page.bottom() - y);

    // 如果有图片，预留下半部分给图片
    if (!image.isNull()) {
        textRect.setHeight((page.bottom() - y) * 0.6);
    }

    QRectF usedRect;
    painter.drawText(textRect,
                     Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                     bodyText, &usedRect);
    y = usedRect.bottom() + 20;

    // ---- 图片 ----
    if (!image.isNull() && y < page.bottom() - 100) {
        qreal imgW = qMin(image.width(), static_cast<int>(page.width()));
        qreal imgH = imgW * image.height() / image.width();
        if (y + imgH > page.bottom()) {
            imgH = page.bottom() - y;
            imgW = imgH * image.width() / image.height();
        }
        painter.drawPixmap(
            QRectF(page.left(), y, imgW, imgH), image,
            QRectF(0, 0, image.width(), image.height()));
    }

    painter.end();
}
```

这里的排版逻辑是手动计算坐标的——先画标题，再画分隔线，然后画正文，最后画图片。如果正文太长或者图片太大超出了单页范围，在入门阶段我们可以先做简单的截断处理（不自动分页），更复杂的自动分页逻辑属于进阶内容。

用户交互部分，程序提供三个按钮分别对应三种输出方式，另外提供一个文本编辑器和图片选择按钮来准备打印内容：

```cpp
// 打印到打印机
connect(printButton, &QPushButton::clicked, this, [this]() {
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        renderDocument(&printer, titleEdit_->text(),
                       bodyEdit_->toPlainText(), currentImage_);
    }
});

// 导出 PDF
connect(exportPdfButton, &QPushButton::clicked, this, [this]() {
    QString path = QFileDialog::getSaveFileName(
        this, "导出 PDF", "report.pdf", "PDF Files (*.pdf)");
    if (!path.isEmpty()) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(path);
        renderDocument(&printer, titleEdit_->text(),
                       bodyEdit_->toPlainText(), currentImage_);
    }
});

// 打印预览
connect(previewButton, &QPushButton::clicked, this, [this]() {
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, [this](QPrinter *p) {
        renderDocument(p, titleEdit_->text(),
                       bodyEdit_->toPlainText(), currentImage_);
    });
    preview.exec();
});
```

运行程序后，在文本框中输入标题和正文，可选加载一张图片，然后分别点击三个按钮测试。你会发现导出的 PDF 和打印预览显示的效果完全一致——因为它们使用的是同一个 renderDocument() 函数。这就是 Qt 打印体系统一管线的好处：渲染逻辑写一次，输出目标随意切换。

## 5. 练习项目

练习项目：多页文档打印引擎。

我们要做一个支持自动分页的打印引擎。给定一段很长的文本（可能跨越多页），程序需要自动计算文本高度，当超出单页可打印区域时自动调用 QPrinter::newPage() 开始新的一页，并在每页底部添加页码。

完成标准是这样的：使用 QPainter::boundingRect() 计算文本占用的矩形高度，判断是否超出 pageRect 的底部；超出时调用 newPage() 后重新定位 y 坐标到页面顶部继续绘制；在每页的底部居中位置绘制 "第 X 页" 的页码文字；支持 A4 纵向和横向两种页面方向，页边距统一为 20mm。

几个实现提示：QPainter::drawText() 的最后一个参数 QRectF* usedRect 会返回实际绘制的矩形，用这个矩形的高度来判断剩余空间是否够用；newPage() 调用后 QPainter 的状态（字体、画笔等）会被重置，需要重新设置；页码可以通过计数器变量在循环中递增。

## 6. 官方文档参考

[Qt 文档 · QtPrintSupport 模块](https://doc.qt.io/qt-6/qtprintsupport-index.html) -- 打印支持模块总览

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机/输出设备抽象

[Qt 文档 · QPrintDialog](https://doc.qt.io/qt-6/qprintdialog.html) -- 标准打印对话框

[Qt 文档 · QPrintPreviewDialog](https://doc.qt.io/qt-6/qprintpreviewdialog.html) -- 打印预览对话框

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。QtPrintSupport 模块的 API 表面不算大——QPrinter 封装输出目标，QPrintDialog 提供用户选择入口，QPrintPreviewDialog 提供所见即所得的预览，三者配合覆盖了桌面打印的绝大部分场景。记住核心原则：渲染逻辑只写一份，通过 QPrinter 的输出格式切换来实现"打印到纸张"和"导出到 PDF"两条路径。掌握了这个统一管线之后，在你的项目中添加打印功能就不是什么难事了。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
