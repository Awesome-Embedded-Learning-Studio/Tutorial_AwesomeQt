# 现代Qt开发教程（新手篇）3.72——QPrinter：打印机抽象类

## 1. 前言 / 把内容输出到纸张和 PDF 是桌面应用绕不开的一关

不管你做的是财务报表系统、工程绘图工具还是简单的文档编辑器，用户早晚有一天会指着屏幕上的内容问你：这个东西能打印出来吗？这个需求听起来平淡无奇，但如果你从零开始处理打印机驱动、页面尺寸、DPI 缩放、分页逻辑这些事情，工作量会迅速失控。Qt 的 QPrinter 就是为了让我们避开这些底层细节而存在的——它把"物理打印机"和"PDF 文件"统一抽象成一个 QPainterDevice，你只需要像在屏幕上绘图一样用 QPainter 往上面画内容，剩下的交给 Qt 去处理。

QPrinter 属于 Qt6::PrintSupport 模块，而不是 Qt6::Widgets。这意味着你的 CMakeLists.txt 里需要额外 find_package Qt6 PrintSupport 并链接 Qt6::PrintSupport。很多人第一次写打印功能的时候忘了加这个模块，编译阶段就收获了满屏的 undefined reference，笔者自己也踩过这个坑。

今天我们分四个部分展开。先从 QPrinter 的基础页面配置讲起——设置纸张尺寸、方向、页边距这些参数；然后用 QPainter + QPrinter 的组合实现自定义内容的打印，把文本、表格、图形画到打印页面上；接着引入 QPrintPreviewDialog 做打印预览，让用户在正式打印之前看到效果；最后单独讲一下导出 PDF 的用法，因为 QPrinter 的 PdfFormat 输出模式本质上和打印到纸张走的是同一条代码路径。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QPrinter 在 Qt6::PrintSupport 模块中，CMake 配置需要 find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport) 并链接 Qt6::PrintSupport。示例代码涉及 QPrinter、QPainter、QPrintPreviewDialog、QMainWindow、QPushButton、QTextEdit、QVBoxLayout 以及 QFontMetrics 等类的配合使用。打印功能在 Linux 上依赖 CUPS（Common UNIX Printing System），macOS 和 Windows 各自使用原生的打印子系统。没有物理打印机也不影响代码编写和测试——我们可以把输出模式设为 PDF 来验证所有绘制逻辑。

## 3. 核心概念讲解

### 3.1 页面配置：setPageSize / setOrientation / setPageMargins

QPrinter 在创建时就会读取系统默认打印机的配置作为初始状态。如果你不做任何设置就直接开始打印，拿到的是系统默认的纸张尺寸（比如中国大陆和欧洲的 A4，北美的 Letter）和纵向方向。但在实际项目中我们经常需要显式控制这些参数。

首先是纸张尺寸。QPrinter::setPageSize() 接受一个 QPageSize 对象，你可以用预定义的尺寸常量来构造它，也可以指定自定义的宽高。最常见的用法是直接传入 QPageSize(QPageSize::A4) 或者 QPageSize(QPageSize::Letter)。

```cpp
QPrinter printer;
printer.setPageSize(QPageSize(QPageSize::A4));
```

如果你需要自定义尺寸——比如打印小票或者不干胶标签——可以传入毫米或英寸为单位的尺寸：

```cpp
// 热敏小票打印机：宽 80mm，长度不限（由内容决定）
printer.setPageSize(QPageSize(
    QSizeF(80, 297), QPageSize::Millimeter));
```

然后是页面方向。默认是纵向（Portrait），也就是高度大于宽度。如果你要打印横向的表格或者宽幅图表，需要调用 setOrientation 把它改成横向：

```cpp
printer.setOrientation(QPrinter::Landscape);
```

还有一个经常被忽略但非常影响打印效果的参数：页边距。setPageMargins() 接受一个 QMarginsF 对象，单位可以选择毫米、英寸、磅或者设备像素。如果你不设置，QPrinter 会使用打印机的默认页边距——这个值因打印机型号而异，有些打印机的最小页边距可能达到 5mm 以上，你如果把内容画到页边距之外就会被裁切掉。

```cpp
// 四边各留 15mm 页边距
printer.setPageMargins(
    QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
```

这三个参数——纸张尺寸、方向、页边距——共同决定了 QPainter 在 QPrinter 上的可用绘图区域。你可以通过 printer.pageRect(QPrinter::Millimeter) 获取实际的可用绘图矩形，在做复杂布局之前先打印一下这个矩形的尺寸是个好习惯，能帮你确认配置是否正确。

```cpp
QRectF page = printer.pageRect(QPrinter::Millimeter);
qDebug() << "可用区域:" << page.width() << "x"
         << page.height() << "mm";
```

需要注意的是，setPageSize 和 setOrientation 必须在 QPainter 开始绘制之前调用。一旦 QPainter 以 QPrinter 为 paint device 开始了 begin()，页面参数就被锁定了——中途修改不会生效。如果你需要每一页用不同的纸张尺寸或方向，需要在每页结束后调用 QPainter::end()，修改参数，再重新 begin()。

### 3.2 QPainter + QPrinter：自定义内容的打印流程

QPrinter 继承自 QPagedPaintDevice，而 QPagedPaintDevice 继承自 QPaintDevice。这个继承链告诉我们一个关键事实：QPrinter 可以作为 QPainter 的绘制目标。你在屏幕上用 QPainter 往 QWidget 上画矩形、文字、线条的那套代码，几乎可以原封不动地用于 QPrinter——区别只是坐标系从屏幕像素变成了打印机的点（point，1/72 英寸）。

打印的基本流程分为三步：创建 QPrinter 并配置参数、创建 QPainter 以 QPrinter 为目标开始绘制、绘制完成后结束。

```cpp
QPrinter printer;
printer.setPageSize(QPageSize(QPageSize::A4));
printer.setPageMargins(
    QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

if (!QPainter painter(&printer); painter.isActive()) {
    qWarning() << "无法启动打印绘制";
    return;
}

// 在第一页上绘制内容
painter.drawText(100, 100, "Hello, Printer!");
painter.drawLine(100, 120, 500, 120);

// painter 析构时自动调用 end()
```

这里的 if 语句用了 C++17 的初始化变量声明语法——在 if 条件中声明 painter 并检查它是否成功激活。如果 QPrinter 的状态不对（比如打印机不可用），QPainter::begin() 会失败，isActive() 返回 false。

打印文本的时候有一个常见的问题：QPainter::drawText() 不会自动换行，也不会自动分页。如果你有一大段文本要打印，需要自己处理换行和分页逻辑。换行可以通过 QFontMetrics 配合 QRectF 来实现——把文本交给 QFontMetrics 按给定宽度逐行切分，或者直接使用 QPainter::drawText(const QRectF &rect, int flags, const QString &text) 的矩形重载版本，它会自动在矩形区域内换行。

```cpp
QFont font("SimSun", 12);
painter.setFont(font);

QRectF textRect(50, 50, 500, 700);
QString longText = "这是一段很长的文本内容，..."
                   "需要打印在多行上。";

painter.drawText(textRect,
    Qt::TextWordWrap | Qt::AlignTop | Qt::AlignLeft,
    longText);
```

分页则需要手动调用 QPrinter::newPage()。这个函数的作用是结束当前页、开始新的一页——相当于往打印机发送一个"走纸"命令。你需要在绘制逻辑中判断当前页面的剩余空间是否足够放下下一块内容，不够就 newPage()。

```cpp
QPrinter printer;
QPainter painter(&printer);

QFont font("SimSun", 11);
painter.setFont(font);

const QFontMetricsF fm(font);
const qreal lineHeight = fm.lineSpacing();
const qreal pageWidth = printer.pageRect(QPrinter::Point).width();
const qreal pageHeight = printer.pageRect(QPrinter::Point).height();

// 页面顶部留 50 磅的页边距
qreal y = 50;

for (const auto &line : allLines) {
    if (y + lineHeight > pageHeight - 50) {
        printer.newPage();  // 剩余空间不够，换页
        y = 50;
    }
    painter.drawText(50, y, line);
    y += lineHeight;
}
```

这段代码的逻辑很直接：跟踪当前 y 坐标，每次画一行前检查是否还有足够的空间。如果加上这行的高度会超出页面底部（再留 50 磅的底边距），就调用 newPage() 开新页并重置 y。这是最简单的分页策略——对于更复杂的布局（表格、图文混排），分页逻辑会复杂得多，但基本思路不变：跟踪剩余空间，不够就换页。

还有一个细节值得注意：打印机的坐标原点在页面左上角（页边距以内），x 轴向右，y 轴向下。这个坐标系和 QWidget 的坐标系完全一致。但打印机的分辨率通常是 72 DPI（PostScript 标准）或者更高——有些打印机的物理 DPI 可能达到 600 或 1200。QPainter 会自动处理 DPI 缩放，所以你用 point 为单位绘制的尺寸在不同分辨率的打印机上物理大小是一致的。但如果你用 setWindow / setViewport 做了自定义映射，就需要自己确保缩放正确。

### 3.3 QPrintPreviewDialog：打印前的预览

让用户直接点"打印"然后发现内容歪了、字体太大、表格被截断——这种体验非常糟糕。QPrintPreviewDialog 提供了一个现成的预览窗口，用户可以在里面翻页、缩放、查看每一页的实际效果，确认无误后再点击打印。

QPrintPreviewDialog 的使用方式有点特殊。你创建一个 QPrintPreviewDialog 实例，然后连接它的 paintRequested(QPrinter*) 信号——这个信号在对话框需要渲染预览内容时发射，参数是一个指向内部 QPrinter 对象的指针。你在槽函数中用这个 QPrinter 创建 QPainter 并绘制内容，对话框就会把你的绘制结果显示在预览区域中。

```cpp
void MainWindow::onPrintPreview()
{
    QPrinter printer;
    QPrintPreviewDialog dialog(&printer, this);
    dialog.setMinimumSize(800, 600);

    connect(&dialog, &QPrintPreviewDialog::paintRequested,
            this, &MainWindow::paintDocument);

    dialog.exec();
}

void MainWindow::paintDocument(QPrinter *printer)
{
    QPainter painter(printer);
    if (!painter.isActive()) return;

    // 和直接打印完全相同的绘制逻辑
    painter.setFont(QFont("SimSun", 14));
    painter.drawText(100, 100, "预览文档标题");

    QFontMetricsF fm(painter.font());
    qreal y = 150;
    const qreal lineH = fm.lineSpacing();

    for (const auto &text : m_documentLines) {
        if (y + lineH >
            printer->pageRect(QPrinter::Point).height() - 50) {
            printer->newPage();
            y = 50;
        }
        painter.drawText(50, y, text);
        y += lineH;
    }
}
```

这里有一个重要的设计模式：paintDocument() 这个槽函数同时服务于"打印预览"和"实际打印"两种场景。当 QPrintPreviewDialog 调用它时，传入的 QPrinter 指向的是预览渲染器，你的绘制内容会显示在预览窗口中。当用户在预览窗口点击"打印"按钮时，对话框会再次调用 paintDocument()，但这次传入的 QPrinter 指向的是真正的物理打印机或者 PDF 输出。这意味着你只需要写一份绘制代码，预览和打印就自动共享了——这比维护两套独立的渲染逻辑靠谱得多。

QPrintPreviewDialog 的工具栏上提供了翻页、缩放、页面切换等操作按钮。用户可以用鼠标滚轮或者按钮在页面间导航，也可以通过缩放查看细节。你不需要写任何额外代码来支持这些交互——Qt 全部帮你处理好了。你唯一要做的事情就是在 paintRequested 的槽函数里把内容画正确。

需要注意的是，QPrintPreviewDialog 每次需要更新预览时都会发射 paintRequested。比如用户在预览窗口中改变缩放比例时，对话框会重新请求绘制。这意味着你的 paintDocument() 可能被调用多次。如果你的绘制逻辑很重（比如需要从数据库加载大量数据），建议在调用 dialog.exec() 之前把数据准备好存到成员变量中，paintDocument() 只负责绘制，不做数据加载。

### 3.4 导出 PDF：QPrinter 的 PdfFormat 模式

QPrinter 有一个特别实用的功能：把输出目标从物理打印机切换到 PDF 文件。你只需要设置输出格式为 QPrinter::PdfFormat 并指定文件名，之后的绘制代码和打印到纸张完全一样。这意味着你不需要引入任何额外的 PDF 生成库——QPainter + QPrinter 本身就是一个 PDF 生成器。

```cpp
QPrinter printer(QPrinter::HighResolution);
printer.setOutputFormat(QPrinter::PdfFormat);
printer.setOutputFileName("report.pdf");
printer.setPageSize(QPageSize(QPageSize::A4));
printer.setPageMargins(
    QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

QPainter painter(&printer);
painter.setFont(QFont("SimSun", 16));
painter.drawText(100, 100, "PDF 报告标题");
painter.setFont(QFont("SimSun", 11));
painter.drawText(100, 140, "这是 PDF 文档的内容。");
```

执行这段代码后，当前目录下会生成一个 report.pdf 文件，内容就是你在 QPainter 上绘制的所有内容。生成的 PDF 是矢量格式的——文字可以选中复制，线条放大不会模糊。这对于生成报表、导出凭证、归档文档等场景非常合适。

QPrinter::HighResolution 是一个值得了解的构造参数。默认情况下 QPrinter 使用 ScreenResolution（72 DPI），这对 PDF 输出来说精度足够。但如果你同时支持打印到纸张，HighResolution（通常 300-1200 DPI）会产生更高质量的输出——代价是处理时间稍长、文件体积稍大。对于 PDF 导出，ScreenResolution 在大多数场景下已经够用。

导出 PDF 的一个典型应用是"打印/导出"双模式：同一个绘制函数，根据用户的选择切换输出目标。用户点"打印"就输出到物理打印机，点"导出 PDF"就输出到文件，绘制代码一份都不用改。

```cpp
void MainWindow::exportOrPrint(bool toPdf)
{
    QPrinter printer;

    if (toPdf) {
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName("exported_document.pdf");
    }

    printer.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter(&printer);
    paintContent(&painter, &printer);
}

void MainWindow::paintContent(QPainter *painter, QPrinter *printer)
{
    // 统一的绘制逻辑——不关心输出目标是打印机还是 PDF
    painter->drawText(100, 100, "统一内容");
}
```

这种设计模式在真实项目中非常常见。你的核心绘制逻辑只管往 QPainter 上画，完全不需要知道底层输出到哪里。QPainter 负责把你的绘制指令翻译成打印机命令或者 PDF 页面描述——这就是抽象层的威力。

还有一点：QPrinter::setOutputFileName() 的文件名如果是相对路径，它相对于应用的当前工作目录。在正式项目中建议使用绝对路径或者让用户通过 QFileDialog 选择保存位置。如果你指定的文件路径不可写（权限不足、磁盘已满），QPainter::begin() 会失败，所以别忘记检查 isActive()。

## 4. 踩坑预防

第一个坑是忘了链接 Qt6::PrintSupport。QPrinter 不在 QtWidgets 里，它在独立的 PrintSupport 模块中。如果你的 CMakeLists.txt 只写了 find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)，编译时会遇到 QPrinter 相关符号全部 undefined reference。解决方法就是在 find_package 里加上 PrintSupport，然后在 target_link_libraries 里加上 Qt6::PrintSupport。

第二个坑是在 QPainter 已经 begin() 之后修改页面参数。setPageSize、setOrientation、setPageMargins 这些函数只有在 QPainter 未开始绘制时才生效。一旦你调用了 QPainter painter(&printer)，打印机设备就进入了"正在打印"状态，此时再改纸张尺寸没有任何效果。如果确实需要每页不同的纸张参数（虽然这种需求很少见），必须在 newPage() 之前结束当前 QPainter，修改参数后重新创建 QPainter。

第三个坑是打印中文时字体缺失。QPainter 在 QPrinter 上绘制文本时使用的字体必须在系统上可用。如果你指定了 "SimSun" 但用户的 Linux 系统没安装宋体，Qt 会回退到默认字体，中文可能显示为方框。建议使用通用的字体族名称（比如 "sans-serif"）或者检测系统可用字体列表。对于 PDF 导出来说，中文字体的问题更加突出——因为 PDF 文件可能在不同系统的阅读器中打开。一个稳妥的做法是使用 QFontDatabase 提前检查目标字体是否存在，不存在就选择一个已知的中文回退字体。

第四个坑是忽略了页边距。打印机的可打印区域通常比纸张尺寸小一圈——因为打印机机械结构需要留出夹纸的边距。如果你把内容从 (0, 0) 开始画，边缘部分很可能会被裁掉。始终使用 pageRect() 获取实际可用区域，或者通过 setPageMargins() 显式设置合理的边距。

第五个坑是 newPage() 的调用时机。如果你只在最后一页绘制了内容但忘了调用 newPage()，那最后一页的内容也会正常输出——不需要手动 newPage() 来"结束"最后一页。但如果你在没有绘制任何内容的情况下调用了 newPage()，会输出一张空白页。在循环逻辑中要确保每次 newPage() 之后确实有内容要画。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，模拟一个简单的文档编辑器。主窗口中间放置一个 QTextEdit 作为编辑区域，预填一段包含多行内容的示例文本。工具栏上有三个按钮——"页面设置"按钮弹出对话框让用户选择纸张尺寸（A4 / Letter / B5）和方向（纵向 / 横向），"打印预览"按钮打开 QPrintPreviewDialog 显示 QTextEdit 中的内容（自动处理分页），"导出 PDF"按钮将内容导出为 PDF 文件。打印和导出共享同一个绘制函数，该函数接收 QPrinter 指针和待打印文本，通过 QFontMetrics 逐行测量并自动分页。

提示：绘制函数可以先调用 printer->pageRect(QPrinter::Point) 获取可用区域，然后用 QFontMetricsF 计算每行高度，逐行绘制。当 y 坐标超过页面底部边距时调用 printer->newPage() 开新页并重置 y 坐标。

## 6. 官方文档参考链接

[Qt 文档 -- QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机抽象类

[Qt 文档 -- QPageSize](https://doc.qt.io/qt-6/qpagesize.html) -- 页面尺寸管理

[Qt 文档 -- QPageLayout](https://doc.qt.io/qt-6/qpagelayout.html) -- 页面布局参数

[Qt 文档 -- QPrintPreviewDialog](https://doc.qt.io/qt-6/qprintpreviewdialog.html) -- 打印预览对话框

[Qt 文档 -- QPainter](https://doc.qt.io/qt-6/qpainter.html) -- 绘图引擎

---

到这里，QPrinter 的核心用法就全部讲完了。页面配置（纸张尺寸、方向、边距）决定了输出画布的大小，QPainter + QPrinter 的组合让我们用熟悉的绘图 API 在纸张上自由绘制内容，QPrintPreviewDialog 提供了零成本的打印预览能力，PdfFormat 模式则让同一份绘制代码同时支持打印和 PDF 导出。掌握了这些，你的应用就拥有了完整的硬拷贝输出能力。
