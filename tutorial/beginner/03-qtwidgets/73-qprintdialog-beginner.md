# 现代Qt开发教程（新手篇）3.73——QPrintDialog：打印对话框

## 1. 前言 / 让用户自己选打印机比硬编码靠谱得多

上一篇我们讲了 QPrinter 的页面配置和绘制流程，但有一个关键环节被故意跳过了：QPrinter 默认使用系统默认打印机，用户没法在运行时选择用哪台打印机、打几份、打哪几页。如果你的应用直接调用 QPainter 开始画，用户连改个打印份数的机会都没有——这不是正常的桌面软件该有的体验。QPrintDialog 就是补上这一环的：它弹出操作系统原生的打印对话框，让用户选择打印机、设置份数、指定页面范围、切换单双面打印，然后你根据用户的选择来驱动后续的打印流程。

QPrintDialog 和 QColorDialog、QFontDialog 属于同一类——它们是 Qt 对操作系统原生对话框的封装。在 Windows 上你会看到标准的 Windows Print 对话框，在 macOS 上是标准的打印面板，在 Linux 上则是 CUPS 的打印配置界面。这意味着对话框的外观和功能因平台而异，但核心的交互模式是统一的：用户配置参数，点"打印"返回 Accepted，点"取消"返回 Rejected。

今天我们从四个方面来展开。先看 QPrintDialog::exec() 弹出对话框并获取用户选择的返回值，然后深入讨论如何读取用户配置的打印份数、页面范围、单双面等参数，接着把这些参数和 QPainter 的绘制流程串起来形成完整的打印管线，最后处理一个在实际部署中必然会遇到的问题——用户的机器上可能根本没有安装打印机，这时候需要提供一个 PDF 导出的回退方案。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QPrintDialog 和 QPrinter 一样属于 Qt6::PrintSupport 模块，CMake 配置需要 find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport) 并链接 Qt6::PrintSupport。示例代码涉及 QPrintDialog、QPrinter、QPainter、QMainWindow、QPushButton、QTextEdit、QLabel、QVBoxLayout 以及 QFileDialog 等类。在无打印机的环境下（比如 WSL、Docker 容器、CI 服务器），QPrintDialog::exec() 可能直接返回 Rejected，示例代码中包含了这种情况的 PDF 回退处理。

## 3. 核心概念讲解

### 3.1 exec() 弹出系统打印对话框

QPrintDialog 的核心用法和其他标准对话框（QFileDialog、QFontDialog）完全一致：创建实例，调用 exec()，根据返回值决定后续操作。exec() 是模态的——对话框弹出后主窗口被阻塞，用户必须处理对话框（点击打印或取消）才能回到主窗口。

```cpp
QPrinter printer;
QPrintDialog dialog(&printer, this);

if (dialog.exec() == QDialog::Accepted) {
    // 用户点击了"打印"，printer 已经被更新为用户的配置
    QPainter painter(&printer);
    painter.drawText(100, 100, "Hello from QPrintDialog");
} else {
    // 用户点击了"取消"
    qDebug() << "用户取消了打印";
}
```

这里有一个容易忽略的关键点：QPrintDialog 在构造时接受一个 QPrinter 指针。当用户在对话框中修改了打印机选择、份数、页面范围等配置后，这些修改会直接写入到你传入的那个 QPrinter 对象中。也就是说，dialog.exec() 返回 Accepted 之后，你的 QPrinter 对象已经携带了用户的所有配置——你不需要从 QPrintDialog 上读任何属性，直接用这个 QPrinter 开始绘制就行。这个设计非常干净：QPrintDialog 只是负责收集用户的打印参数，真正的打印行为完全由你和 QPrinter 之间的交互决定。

你可以通过 setWindowTitle() 自定义对话框的标题，也可以通过 setEnabledOptions() / addEnabledOption() 控制对话框上显示哪些配置选项。

```cpp
QPrintDialog dialog(&printer, this);
dialog.setWindowTitle("打印报表");
dialog.setOptions(
    QPrintDialog::PrintToFile |
    QPrintDialog::PrintSelection |
    QPrintDialog::PrintPageRange);
```

QPrintDialog::PrintToFile 选项允许用户选择"打印到文件"（PDF 或 PS），PrintSelection 让用户可以选择只打印选中的内容，PrintPageRange 让用户可以指定打印的页面范围（比如只打第 2-5 页）。这些选项是否可用取决于当前平台的打印系统是否支持——如果不支持，选项会在对话框中灰显或隐藏。

### 3.2 获取用户配置：份数、打印范围、单双面

QPrintDialog 把用户的配置写入了 QPrinter 对象，所以我们通过 QPrinter 的接口来读取这些参数。

打印份数通过 copyCount() 获取。这个值就是用户在对话框中设置的"打印份数"——如果用户设了 3 份，copyCount() 返回 3。默认值是 1。需要注意的是，有些打印驱动自身支持多份打印（硬件级多份），这时 QPrinter::supportsMultipleCopies() 返回 true，驱动会负责打印多份；如果不支持，你的应用需要自己循环多次绘制。不过在大多数场景下，你不需要关心这个区别——QPainter 内部会处理 copyCount 的逻辑。

```cpp
int copies = printer.copyCount();
qDebug() << "打印份数:" << copies;
```

打印范围通过 printRange() 获取，它返回一个 QPrinter::PrintRange 枚举值。QPrinter::AllPages 表示打印全部页面，QPrinter::PageRange 表示用户指定了页面范围，QPrinter::Selection 表示只打印选中的内容。当 printRange() 返回 PageRange 时，你可以通过 fromPage() 和 toPage() 获取具体的范围。

```cpp
if (printer.printRange() == QPrinter::PageRange) {
    int from = printer.fromPage();
    int to = printer.toPage();
    qDebug() << "打印范围: 第" << from << "页 到 第"
             << to << "页";
}
```

这里有一个实现细节需要注意：fromPage() 和 toPage() 返回的是 1-based 的页码——第一页是 1，不是 0。如果你的绘制逻辑中用 0-based 的页码索引，需要做转换。另外，如果 printRange() 是 AllPages，fromPage() 和 toPage() 返回 0，表示没有范围限制。

单双面打印通过 duplex() 获取，返回 QPrinter::DuplexMode 枚举。QPrinter::DuplexNone 是单面打印，QPrinter::DuplexLongSide 是长边翻转（像书本一样翻页），QPrinter::DuplexShortSide 是短边翻转（像记事本一样翻页）。不过 duplex 设置是否生效完全取决于打印机硬件是否支持——如果你把 duplex 设为 DuplexLongSide 但打印机只支持单面打印，这个设置会被静默忽略。

```cpp
auto duplexMode = printer.duplex();
switch (duplexMode) {
    case QPrinter::DuplexNone:
        qDebug() << "单面打印"; break;
    case QPrinter::DuplexLongSide:
        qDebug() << "双面打印（长边翻转）"; break;
    case QPrinter::DuplexShortSide:
        qDebug() << "双面打印（短边翻转）"; break;
    default:
        break;
}
```

还有一个实用的属性：printerName()。它返回用户选择的那台打印机的名称。如果你需要在打印之前显示"正在发送到 HP LaserJet Pro..."这样的状态信息，就用这个。

```cpp
QString printerName = printer.printerName();
m_statusLabel->setText(
    QString("正在打印到 %1...").arg(printerName));
```

### 3.3 与 QPainter 联动的完整打印流程

现在我们把 QPrintDialog 的用户配置和 QPainter 的绘制流程串起来，形成一条完整的打印管线：用户点击打印按钮 -> 弹出 QPrintDialog -> 用户配置参数 -> 读取参数并决定打印内容 -> QPainter 开始绘制 -> 完成。

最典型的场景是"用户指定了页面范围，只打印其中一部分"。比如文档总共有 10 页，用户只想打印第 3-5 页。你的绘制函数需要知道当前正在绘制的是第几页，然后判断这一页是否在用户指定的范围内——如果在范围内就正常绘制内容，如果不在范围内就跳过（但仍需调用 newPage() 来推进页面计数）。

```cpp
void paintDocument(QPrinter *printer)
{
    QPainter painter(printer);
    if (!painter.isActive()) return;

    const int totalPages = calculateTotalPages(printer);
    const int fromPage =
        printer->printRange() == QPrinter::PageRange
            ? printer->fromPage() : 1;
    const int toPage =
        printer->printRange() == QPrinter::PageRange
            ? printer->toPage() : totalPages;

    int currentPage = 1;

    for (int page = 1; page <= totalPages; ++page) {
        if (page >= fromPage && page <= toPage) {
            // 这一页在用户指定的范围内，正常绘制
            paintPage(&painter, printer, page);
        }
        // 不管是否在范围内，都需要推进到下一页
        if (page < totalPages) {
            printer->newPage();
        }
    }
}
```

这段代码的逻辑需要仔细理解。循环遍历所有页面（从 1 到 totalPages），对于每一页：如果在用户指定的范围内，就调用 paintPage() 绘制实际内容；如果不在范围内，这一页会被跳过但仍然调用了 newPage()——不对，这里有个问题。实际上如果页面不在范围内，我们不应该调用 newPage()，因为我们根本不需要产生这个空白页。

正确的做法是：先跳过不在范围内的页面，只对范围内的页面执行绘制和分页。

```cpp
void paintDocument(QPrinter *printer)
{
    QPainter painter(printer);
    if (!painter.isActive()) return;

    const int totalPages = calculateTotalPages(printer);
    const int fromPage =
        printer->printRange() == QPrinter::PageRange
            ? printer->fromPage() : 1;
    const int toPage =
        printer->printRange() == QPrinter::PageRange
            ? printer->toPage() : totalPages;

    bool firstPage = true;

    for (int page = fromPage; page <= toPage; ++page) {
        if (!firstPage) {
            printer->newPage();
        }
        firstPage = false;

        paintPage(&painter, printer, page);
    }
}
```

这样就对了。循环直接从 fromPage 遍历到 toPage，只绘制用户需要的那几页。firstPage 标志确保第一页不需要 newPage()（因为 QPrinter 创建时默认就在第一页上），后续每一页在绘制前先 newPage()。

copyCount（份数）通常不需要你在代码中处理——QPainter + QPrinter 的内部实现会自动处理多份打印。但如果你需要自己控制（比如每份的页眉上标注"第 X 份"），可以手动循环：

```cpp
const int copies = printer->copyCount();
for (int copy = 0; copy < copies; ++copy) {
    // 在页眉上标注份数
    // 然后绘制所有页面...
}
```

### 3.4 无打印机时的 PDF 回退方案

这是一个在真实部署中必然会遇到的场景。你的应用可能在各种环境下运行——开发机上通常有虚拟打印机，但客户的办公电脑可能没装打印机驱动，Linux 服务器上可能连 CUPS 都没启动，WSL 环境下根本就没有打印子系统。如果 QPrintDialog::exec() 直接返回 Rejected，你不知道是用户主动取消了还是因为系统根本没有可用的打印机。

QPrinter 提供了几个有用的查询函数来判断当前打印环境的状态。printerName() 返回空字符串表示没有可用打印机，isValid() 在 QPrinter 被正确初始化后返回 true。但最可靠的方式是检查 QPrinterInfo::availablePrinters() 是否为空——它能告诉你系统上到底有没有可用的打印设备。

```cpp
#include <QPrinterInfo>

QList<QPrinterInfo> printers =
    QPrinterInfo::availablePrinters();

if (printers.isEmpty()) {
    // 没有打印机，直接走 PDF 回退
    int ret = QMessageBox::question(
        this, "未检测到打印机",
        "当前系统没有可用的打印机。\n"
        "是否将内容导出为 PDF 文件？",
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        exportToPdf();
    }
    return;
}
```

在 PDF 回退方案中，我们把 QPrinter 的输出格式切换为 PdfFormat，通过 QFileDialog 让用户选择保存路径，然后用和打印完全相同的绘制函数输出内容。这样既处理了无打印机的环境，又不需要维护两套独立的代码路径。

```cpp
void exportToPdf()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出 PDF", "output.pdf",
        "PDF 文件 (*.pdf)");
    if (filePath.isEmpty()) return;

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(
        QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    // 复用同一个绘制函数
    paintDocument(&printer);

    QMessageBox::information(
        this, "导出成功",
        QString("内容已保存到:\n%1").arg(filePath));
}
```

还有一点值得注意：在有些 Linux 桌面环境下，QPrintDialog 会依赖系统的打印服务（CUPS）。如果 CUPS 服务没有运行，QPrintDialog 可能直接弹不出来或者一闪而过。在这种情况下，如果你的应用目标用户可能包括 Linux 桌面用户，建议在弹出 QPrintDialog 之前先做一次打印机检测，无打印机就直接走 PDF 路径，避免弹出一个空对话框让用户摸不着头脑。

更健壮的做法是把"打印"和"导出 PDF"合并在一个统一的入口中。比如工具栏上的按钮叫"输出"，点击后先检测打印机——有打印机就弹 QPrintDialog，对话框里也有"打印到文件"的选项；没有打印机就直接弹 PDF 导出。这样用户的体验是连贯的，不需要关心底层到底有没有打印硬件。

## 4. 踩坑预防

第一个坑是 exec() 返回 Rejected 时的误判。exec() 返回 Rejected 有两种原因：用户主动点了取消，或者系统没有可用的打印机导致对话框无法正常弹出。你的代码需要区分这两种情况。最简单的方式是在弹出 QPrintDialog 之前先检查 QPrinterInfo::availablePrinters()，如果没有打印机就跳过对话框直接走 PDF 回退——这样 exec() 返回 Rejected 就只意味着"用户取消了"。

第二个坑是页面范围的 1-based 索引。QPrinter::fromPage() 和 toPage() 返回的页码从 1 开始，而你的绘制循环中的页码索引可能从 0 开始（特别是用数组索引的时候）。如果你用 0-based 索引去和 1-based 的 fromPage/toPage 比较，会少打印一页或者多打印一页。建议在绘制函数中统一使用 1-based 页码。

第三个坑是 copyCount 在不同打印机上的行为差异。有些打印机驱动硬件级支持多份打印，这时候你只需要绘制一次内容，打印机会自动复制。有些打印机不支持硬件多份，需要应用层循环多次。QPainter 内部会根据打印机能力自动处理这个问题，所以通常你不需要自己循环——但如果你手动处理了 copyCount 循环，在某些打印机上可能会打出 copies^2 份。

第四个坑是 duplex（双面打印）设置无效。duplex 设置是否生效完全取决于打印机硬件是否支持双面打印。即使你在代码里设了 QPrinter::DuplexLongSide，如果打印机不支持，它照样单面输出。你可以通过 QPrinterInfo 查询打印机是否支持双面打印：QPrinterInfo::supportedDuplexModes() 返回该打印机支持的所有双面模式。如果你的应用有双面打印的需求，建议先检查打印机是否支持，不支持就在界面上禁用双面选项。

第五个坑是 QPrintDialog 在某些平台上的本地化问题。在中文环境下，QPrintDialog 的按钮文字和标签应该是中文的（因为它是原生对话框）。但如果你使用的是自定义的 QPrintDialog 子类或者手动添加了额外的控件，这些控件上的文字需要你自己翻译。建议在项目中启用 Qt 的翻译机制，确保 PrintSupport 模块的翻译文件被加载。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，模拟一个文档打印工具。主窗口中间放置一个 QTextEdit，预填多页的示例文本内容。工具栏上有"打印"和"导出 PDF"两个按钮。"打印"按钮先通过 QPrinterInfo::availablePrinters() 检测系统打印机——如果检测到打印机，弹出 QPrintDialog 让用户选择打印机和配置参数，确认后使用 QPainter 绘制 QTextEdit 的内容（自动分页，支持用户选择的页面范围）；如果没有检测到打印机，弹出一个 QMessageBox 询问用户是否导出为 PDF。"导出 PDF"按钮通过 QFileDialog 选择保存路径，直接输出 PDF。打印和导出共用同一个 paintDocument() 绘制函数，该函数读取 QPrinter 的 fromPage/toPage 参数来决定打印范围，在状态栏上显示打印进度。

提示：检测打印机使用 QPrinterInfo::availablePrinters()。打印范围判断使用 printer.printRange() == QPrinter::PageRange。绘制时注意 1-based 页码。

## 6. 官方文档参考链接

[Qt 文档 -- QPrintDialog](https://doc.qt.io/qt-6/qprintdialog.html) -- 打印对话框

[Qt 文档 -- QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机抽象类

[Qt 文档 -- QPrinterInfo](https://doc.qt.io/qt-6/qprinterinfo.html) -- 打印机信息查询

[Qt 文档 -- QPrinter::PrintRange](https://doc.qt.io/qt-6/qprinter.html#PrintRange-enum) -- 打印范围枚举

---

到这里，QPrintDialog 的核心用法就全部讲完了。exec() 弹出操作系统原生的打印对话框，用户的所有配置直接写入 QPrinter 对象，我们通过 copyCount / printRange / fromPage / toPage / duplex 读取这些参数并驱动 QPainter 的绘制逻辑，当系统没有打印机时通过 PdfFormat 模式提供 PDF 回退。整个流程从用户交互到硬拷贝输出形成了一条完整的管线。
