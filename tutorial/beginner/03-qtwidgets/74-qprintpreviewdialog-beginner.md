# 现代Qt开发教程（新手篇）3.74——QPrintPreviewDialog：打印预览

## 1. 前言 / 打印预览不是锦上添花，是刚性需求

上一篇我们用 QPrintDialog 让用户选择打印机和配置参数，但用户在正式打印之前最想看到的是什么？是效果。打印消耗纸张和墨粉，如果打印出来的内容歪了、表格被截断、字体太大或太小、页码位置不对，用户只能扔掉重打。对于一个正式的桌面应用来说，打印预览不是一个可选功能——它是打印流程中不可缺少的环节。QPrintPreviewDialog 就是 Qt 为此提供的标准预览对话框：它把你的绘制内容渲染在屏幕上，支持翻页、缩放、单页/双页/四页布局切换，用户确认效果无误后再一键发送到打印机。

QPrintPreviewDialog 的设计思路我们在第 72 篇已经接触过了——通过 paintRequested 信号把绘制逻辑委托给调用方。本篇我们要深入到它的交互细节中：如何在 paintRequested 槽函数中正确处理多页绘制，预览对话框自带的翻页和缩放操作是怎么和你的绘制代码联动的，如何通过自定义工具栏添加额外功能（比如页面跳转、打印参数调整），以及如何集成 QPageSetupDialog 让用户在预览过程中随时修改页面参数。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QPrintPreviewDialog 和 QPageSetupDialog 都属于 Qt6::PrintSupport 模块，CMake 配置需要 find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport) 并链接 Qt6::PrintSupport。示例代码涉及 QPrintPreviewDialog、QPageSetupDialog、QPrinter、QPainter、QMainWindow、QToolBar、QAction、QTextEdit、QVBoxLayout 以及 QFontMetricsF 等类。QPrintPreviewDialog 在没有物理打印机的环境下依然可以正常工作——预览渲染不依赖打印硬件。

## 3. 核心概念讲解

### 3.1 paintRequested 信号与多页绘制

QPrintPreviewDialog 的核心机制是 paintRequested(QPrinter*) 信号。每当对话框需要刷新预览内容时——无论是初次打开、用户翻页、用户改变缩放比例，还是用户修改了页面设置——它都会发射这个信号，把你传入的 QPrinter 指针作为参数传递。你在槽函数中用这个 QPrinter 创建 QPainter 并绘制内容，对话框就会把你的绘制结果显示在预览区域。

这里有一个关键的认知：你的 paintDocument() 槽函数不需要关心用户当前在看哪一页——你必须把所有页面的内容都绘制出来。QPrintPreviewDialog 内部会把你的多页绘制结果缓存起来，然后在预览区域中展示对应的页面。用户翻到第 3 页时，对话框不需要重新调用 paintDocument()——它直接从缓存中取出第 3 页的渲染结果。只有当预览需要完全刷新时（比如用户修改了页面设置），才会重新发射 paintRequested。

```cpp
void MainWindow::onPrintPreview()
{
    QPrinter printer;
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(
        QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    QPrintPreviewDialog dialog(&printer, this);
    dialog.setMinimumSize(900, 700);

    connect(&dialog, &QPrintPreviewDialog::paintRequested,
            this, [this](QPrinter *p) {
                paintDocument(p);
            });

    dialog.exec();
}
```

paintDocument() 的实现和直接打印到纸张时完全一样——用 QPainter 逐行绘制文本，遇到页面底部时调用 newPage() 换页。区别在于 QPainter 的底层输出目标不同：在预览模式下，QPainter 把内容渲染到屏幕上的预览缓冲区；在打印模式下（用户点击了预览对话框上的"打印"按钮），QPainter 把内容发送到物理打印机或 PDF 文件。你的绘制代码完全不需要知道这个区别——这正是 QPaintDevice 抽象层的威力。

```cpp
void MainWindow::paintDocument(QPrinter *printer)
{
    QPainter painter(printer);
    if (!painter.isActive()) return;

    QRectF page = printer->pageRect(QPrinter::Point);
    QFont bodyFont("sans-serif", 11);
    painter.setFont(bodyFont);

    const QFontMetricsF fm(bodyFont);
    const qreal lineH = fm.lineSpacing();
    const qreal pageBottom = page.height() - 30;

    qreal y = 30;
    QStringList lines =
        m_textEdit->toPlainText().split('\n');

    for (const auto &line : lines) {
        if (line.isEmpty()) {
            y += lineH * 0.6;
            continue;
        }

        QRectF bounding = fm.boundingRect(
            QRectF(30, y, page.width() - 60, 0),
            Qt::TextWordWrap | Qt::AlignLeft, line);

        // 需要换页
        if (y + bounding.height() > pageBottom) {
            printer->newPage();
            y = 30;
        }

        painter.drawText(
            QRectF(30, y, page.width() - 60,
                   bounding.height()),
            Qt::TextWordWrap | Qt::AlignLeft, line);

        y += bounding.height() + lineH * 0.2;
    }
}
```

有一点值得强调：paintDocument() 可能被多次调用。每次对话框需要刷新时都会重新调用——如果绘制逻辑很重，建议在打开对话框之前把数据准备好，paintDocument() 只负责纯绘制操作。如果你的绘制涉及数据库查询或者文件读取，千万不要在 paintDocument() 中做这些——否则用户每次改变缩放都会触发一次数据库查询，性能会非常差。

### 3.2 翻页与缩放操作

QPrintPreviewDialog 内置了完整的翻页和缩放交互，你不需要写任何额外代码来支持这些功能。

翻页方面，对话框的工具栏上有"上一页"、"下一页"按钮和一个页码输入框。用户可以直接在输入框中键入页码跳转到指定页面，也可以用键盘方向键翻页。对话框底部的状态栏会显示"第 X 页 / 共 Y 页"的信息。这些功能全部是 QPrintPreviewDialog 内置的——它通过你调用 newPage() 的次数来计算总页数，不需要你额外告知。

缩放方面，工具栏上有"放大"、"缩小"、"适应宽度"、"适应页面"等按钮。用户也可以用 Ctrl+鼠标滚轮来缩放预览。缩放操作不会影响打印输出——它只是改变了预览的显示比例。你的绘制代码不应该对缩放做任何响应，因为缩放是预览对话框的 UI 行为，和实际打印无关。

预览布局方面，对话框支持单页、双页并排、四页网格等多种预览模式。工具栏上的页面布局按钮让用户在不同模式之间切换。这些模式只是改变了预览的排列方式——你的绘制代码仍然是一页一页地画，QPrintPreviewDialog 自动把渲染结果排列到对应的布局中。

```cpp
// 预览对话框支持的模式（在工具栏上切换）
// 单页模式: 一次显示一页
// 双页模式: 左右并排显示两页
// 四页模式: 2x2 网格显示四页
```

需要注意的是，当你的文档页数很多（比如几十页甚至上百页）时，预览对话框的首次渲染可能需要一些时间。这是因为 paintDocument() 需要把所有页面都绘制一遍。如果你的文档超过 50 页，建议在对话框打开时显示一个等待提示，或者在 paintDocument() 中做适当的性能优化——比如避免不必要的字体加载、减少复杂路径的绘制等。

### 3.3 自定义预览工具栏

QPrintPreviewDialog 的工具栏上已经包含了翻页、缩放、布局切换、打印等标准操作。但有些时候你需要添加自定义功能——比如"跳转到第一页/最后一页"按钮、"更改页面设置"按钮、"导出 PDF"按钮。QPrintPreviewDialog 继承自 QDialog，它内部有一个 QToolBar——你可以通过遍历子对象找到它，然后添加自定义的 QAction。

```cpp
void MainWindow::onPrintPreview()
{
    QPrinter printer;
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPrintPreviewDialog dialog(&printer, this);
    dialog.setMinimumSize(900, 700);

    // 查找对话框内的工具栏并添加自定义操作
    auto toolBars = dialog.findChildren<QToolBar*>();
    if (!toolBars.isEmpty()) {
        QToolBar *toolbar = toolBars.first();

        toolbar->addSeparator();

        // 添加"页面设置"按钮
        auto *pageSetupAction = toolbar->addAction(
            "页面设置");
        connect(pageSetupAction, &QAction::triggered,
                this, [&printer, this]() {
                    QPageSetupDialog pageSetup(&printer, this);
                    if (pageSetup.exec() == QDialog::Accepted) {
                        // 页面设置修改后预览会自动刷新
                    }
                });

        // 添加"导出 PDF"按钮
        auto *exportPdfAction = toolbar->addAction(
            "导出 PDF");
        connect(exportPdfAction, &QAction::triggered,
                this, [&printer]() {
                    QString path = QFileDialog::getSaveFileName(
                        nullptr, "导出 PDF", "preview.pdf",
                        "PDF 文件 (*.pdf)");
                    if (!path.isEmpty()) {
                        printer.setOutputFormat(
                            QPrinter::PdfFormat);
                        printer.setOutputFileName(path);
                    }
                });
    }

    connect(&dialog, &QPrintPreviewDialog::paintRequested,
            this, [this](QPrinter *p) {
                paintDocument(p);
            });

    dialog.exec();
}
```

这段代码有几个要点。第一，我们通过 findChildren<QToolBar*>() 获取预览对话框的工具栏。QPrintPreviewDialog 内部使用了一个工具栏来放置标准的预览操作按钮，我们在它的末尾追加自定义按钮。第二，"页面设置"按钮弹出 QPageSetupDialog，用户修改页面参数后，预览对话框会自动检测到 QPrinter 参数变化并重新发射 paintRequested 来刷新预览——这是 QPrintPreviewDialog 和 QPageSetupDialog 之间内置的联动机制。第三，"导出 PDF"按钮把 QPrinter 的输出格式切换为 PdfFormat——但这里有一个微妙之处：这个操作不会立即触发 PDF 输出，它只是改变了 QPrinter 的配置，后续的打印操作才会使用这个配置。

使用 findChildren 来访问预览对话框的内部工具栏属于一种"非正式"的用法——它依赖 QPrintPreviewDialog 的内部实现细节，未来 Qt 版本可能会改变工具栏的组织方式。如果你的项目对 API 稳定性有严格要求，更安全的做法是提供一个独立的"页面设置"按钮放在预览对话框之外，或者在打开预览对话框之前先让用户通过 QPageSetupDialog 配置好页面参数。

### 3.4 QPageSetupDialog：页面参数配置

QPageSetupDialog 是一个独立的对话框，专门用于配置页面参数——纸张尺寸、页面方向、页边距。它和 QPrintDialog 的关系是互补的：QPrintDialog 负责打印机选择和打印参数（份数、范围、单双面），QPageSetupDialog 负责页面物理参数（纸张大小、方向、边距）。在某些平台上（比如 Windows），QPrintDialog 自带页面设置标签页，这时 QPageSetupDialog 就不需要单独使用了。但在 Linux 和 macOS 上，QPageSetupDialog 提供了独立的页面配置界面。

```cpp
QPrinter printer;
printer.setPageSize(QPageSize(QPageSize::A4));

QPageSetupDialog pageSetup(&printer, this);
if (pageSetup.exec() == QDialog::Accepted) {
    // 用户修改了页面参数，printer 已更新
    qDebug() << "纸张尺寸:"
             << printer.pageSize();
    qDebug() << "方向:"
             << (printer.orientation() == QPrinter::Portrait
                     ? "纵向" : "横向");
    qDebug() << "页边距:"
             << printer.pageLayout().margins();
}
```

QPageSetupDialog 和 QPrintDialog 一样，在用户点击"确定"后直接修改传入的 QPrinter 对象。你不需要从 QPageSetupDialog 上读任何属性——它只是一个参数收集器，所有修改都反映在你传入的 QPrinter 上。

在打印预览流程中集成 QPageSetupDialog 的推荐模式是：先让用户通过 QPageSetupDialog 配置页面参数，然后打开 QPrintPreviewDialog 预览效果。如果用户在预览中发现页面参数不对（比如纸张尺寸选错了），可以再次打开 QPageSetupDialog 修改——修改后预览会自动刷新。这种"配置 -> 预览 -> 调整 -> 再预览 -> 打印"的工作流是桌面应用中最常见的打印交互模式。

```cpp
void MainWindow::onPrintWithSetup()
{
    QPrinter printer;

    // 第一步：页面设置
    QPageSetupDialog pageSetup(&printer, this);
    if (pageSetup.exec() != QDialog::Accepted) {
        return;  // 用户取消了页面设置
    }

    // 第二步：打印预览（使用用户配置的页面参数）
    QPrintPreviewDialog preview(&printer, this);
    preview.setMinimumSize(900, 700);

    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, [this](QPrinter *p) {
                paintDocument(p);
            });

    if (preview.exec() == QDialog::Accepted) {
        // 用户在预览中点击了打印
        statusBar()->showMessage("打印已发送");
    }
}
```

这种组合式的交互流程给了用户充分的控制权：先设置纸张大小和方向，然后预览确认效果，最后才真正发送到打印机。每一步都有"取消"的机会，避免了盲目打印导致的纸张浪费。

QPageSetupDialog 在不同平台上的外观差异很大。在 Windows 上它是一个标准的页面设置对话框，包含纸张选择、方向切换和页边距输入。在 macOS 上它是原生的 Page Setup 面板。在 Linux 上它可能是 Qt 自己绘制的对话框（如果没有 CUPS 集成的话）。不管外观如何变化，核心功能是一致的：让用户配置纸张尺寸、方向和页边距，然后把配置写入 QPrinter。

## 4. 踩坑预防

第一个坑是 paintDocument() 被频繁调用导致性能问题。QPrintPreviewDialog 在初始化时会调用一次 paintDocument() 来渲染所有页面，之后每次用户修改页面设置或者点击"打印"都会再调用一次。如果你的绘制逻辑中包含耗时操作（数据库查询、文件读取、复杂计算），建议把数据预处理放在 paintRequested 之前完成，paintDocument() 中只做纯绘制。对于特别大的文档，可以考虑在 paintDocument() 中使用 qApp->processEvents() 来保持 UI 响应，但要小心不要导致重入问题。

第二个坑是在 paintDocument() 中修改 QPrinter 参数。paintRequested 的槽函数接收的是一个 const 意义上的 QPrinter 指针——你应该只从中读取参数（比如 pageSize），不应该修改它的输出目标或页面配置。如果你在 paintDocument() 中调用了 printer->setOutputFormat(QPrinter::PdfFormat)，后续的打印行为可能会出问题。参数配置应该在打开对话框之前或通过 QPageSetupDialog 完成，paintDocument() 只负责"画"。

第三个坑是 QPageSetupDialog 修改参数后预览不刷新。正常情况下 QPrintPreviewDialog 会监听 QPrinter 的参数变化并自动重新发射 paintRequested。但如果你在 paintDocument() 执行期间修改了 QPrinter 参数，可能导致刷新时序问题。确保所有页面参数修改都发生在 paintRequested 槽函数之外。

第四个坑是预览对话框和 QPrintDialog 的重复弹出。有些开发者先弹出 QPageSetupDialog，再弹出 QPrintPreviewDialog，最后又弹出 QPrintDialog——三个对话框弹一遍，用户操作非常繁琐。合理的流程是：QPageSetupDialog（可选）-> QPrintPreviewDialog（预览 + 打印按钮）。QPrintPreviewDialog 的工具栏上已经有"打印"按钮，用户在预览中确认效果后直接点打印就行，不需要再弹一个 QPrintDialog。

第五个坑是 findChildren<QToolBar*>() 的兼容性。通过 findChildren 访问 QPrintPreviewDialog 的内部工具栏依赖于 Qt 的内部实现。在 Qt 5.x 和 Qt 6.x 之间，工具栏的组织方式有过变化——未来版本可能进一步调整。如果你的代码需要在多个 Qt 版本上运行，建议对 findChildren 的结果做 nullptr 检查，并提供优雅的降级方案。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，模拟一个文档编辑器。主窗口中间放置一个 QTextEdit，预填多页的示例文本。工具栏上有三个按钮——"页面设置"按钮弹出 QPageSetupDialog 让用户配置纸张尺寸、方向和页边距；"打印预览"按钮打开 QPrintPreviewDialog 显示完整的文档预览（支持自动分页），预览对话框的工具栏上额外添加一个"页面设置"快捷按钮，让用户在预览过程中也能调整页面参数；"直接打印"按钮先弹出 QPrintDialog 让用户选择打印机和配置参数，确认后执行打印。三个功能共享同一个 paintDocument() 绘制函数，该函数读取 QPrinter 的页面参数进行布局，通过 QFontMetricsF 逐行测量并自动分页，每页底部绘制页码。

提示：paintDocument() 通过 printer->pageRect(QPrinter::Point) 获取可用区域。预览对话框的工具栏通过 findChildren<QToolBar*>() 获取，在上面添加 QAction 触发 QPageSetupDialog。

## 6. 官方文档参考链接

[Qt 文档 -- QPrintPreviewDialog](https://doc.qt.io/qt-6/qprintpreviewdialog.html) -- 打印预览对话框

[Qt 文档 -- QPageSetupDialog](https://doc.qt.io/qt-6/qpagesetupdialog.html) -- 页面设置对话框

[Qt 文档 -- QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机抽象类

[Qt 文档 -- QPrintDialog](https://doc.qt.io/qt-6/qprintdialog.html) -- 打印对话框

---

到这里，QPrintPreviewDialog 的核心用法就全部讲完了。paintRequested 信号和 QPagedPaintDevice 的多页绘制机制让我们用一份代码同时服务于屏幕预览和硬拷贝输出，内置的翻页和缩放交互免去了手动实现的成本，自定义工具栏扩展了预览对话框的功能边界，QPageSetupDialog 的集成使得"配置 -> 预览 -> 打印"的完整工作流成为可能。掌握了这些，你的应用的打印功能在用户体验上就和专业的文档编辑器站在了同一个水平线上。
