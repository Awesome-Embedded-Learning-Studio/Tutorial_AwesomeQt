---
title: "3.74 QPrintPreviewDialog 进阶"
description: "入门篇我们学会了 QPrintPreviewDialog 的基本用法：连接 paintRequested 信号、在回调中绘制内容、exec 打开预览。进阶篇要深挖的是 paintRequested 信号的绘制流程机制、预览页面的自定义操作（缩放、导航、方向），以及如何添加自定义工具栏按钮实现预览与实际打印的一致性保证。"
---

# 现代Qt开发教程（进阶篇）3.74——QPrintPreviewDialog 进阶

## 1. 前言 / 预览对话框里的门道比看起来多

入门篇我们学会了 QPrintPreviewDialog 的基本用法：连接 paintRequested 信号、在回调中绘制内容、exec 打开预览。说实话，做到这一步你已经能在应用里弹出打印预览了——用户能看到每一页的效果，点打印就能输出。但如果你真正用过 WPS、LibreOffice 这些应用的打印预览，你会发现它们的预览远不只是"看一眼"——工具栏上有缩放、页面导航、方向切换，甚至有"导出 PDF"和"页面设置"这样的自定义按钮。这些才是让预览从"能用"变成"专业"的关键。

这篇文章要深挖的是四个核心议题：paintRequested 信号的精确触发时机和绘制流程，预览页面操作（缩放、导航、方向切换）的行为细节，添加自定义工具栏按钮的方法，以及预览与实际打印之间的一致性保证。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QPrintPreviewDialog 属于 QtPrintSupport 模块，需要在 CMakeLists.txt 中添加 `find_package(Qt6 REQUIRED COMPONENTS PrintSupport)` 并链接 `Qt6::PrintSupport`。QPrintPreviewDialog 内部使用 QPrintPreviewWidget 实现预览功能。

## 3. 核心概念讲解

### 3.1 paintRequested 信号的绘制流程

paintRequested 是 QPrintPreviewDialog 最核心的信号——你需要连接它到一个槽函数，在槽函数中对 QPrinter 参数进行绘制。理解这个信号的触发时机是正确使用预览对话框的关键。

paintRequested 会在以下场景中被触发：对话框首次打开时触发一次，用于生成初始预览；用户在预览中修改了页面设置（比如切换纸张方向、调整边距）时触发，用于重新生成预览；用户点击了预览工具栏上的"刷新"按钮时触发（如果对话框有的话）。每次触发时，信号参数是一个指向 QPrinter 对象的指针，这个 QPrinter 就是你在构造 QPrintPreviewDialog 时传入的那个。

```cpp
QPrinter printer;
printer.setOutputFormat(QPrinter::PdfFormat);

QPrintPreviewDialog preview(&printer, this);
connect(&preview, &QPrintPreviewDialog::paintRequested,
        this, &MyClass::on_paint_requested);

preview.exec();
```

这里有一个非常重要的行为：paintRequested 可能在对话框的生命周期内被触发多次。也就是说你的绘制槽函数可能会被调用好几次——每次用户修改设置后都会重新调用。所以你的绘制逻辑必须是幂等的——多次调用不应该产生副作用（比如累加全局变量、追加文件内容等）。

```cpp
void MyClass::on_paint_requested(QPrinter* printer)
{
    // 每次调用都从零开始绘制——不要依赖上一次调用的状态
    QPainter painter(printer);

    QRectF page = printer->pageRect(QPrinter::DevicePixel);
    int page_num = 1;
    qreal y = page.top();

    // 绘制所有内容...
    for (const auto& item : m_items) {
        if (y + item.height() > page.bottom()) {
            printer->newPage();
            ++page_num;
            y = page.top();
        }
        draw_item(&painter, item, y);
        y += item.height();
    }
}
```

另一个需要注意的点是：paintRequested 的参数 QPrinter* 指向的 QPrinter 对象的输出格式被 QPrintPreviewDialog 内部设置为 PdfFormat，输出目标是一个内部的 QBuffer。也就是说，你的绘制内容先被写入内存中的 PDF，然后 QPrintPreviewDialog 再把这个 PDF 渲染成预览图像。这意味着你在 paintRequested 回调中对 printer 调用 setOutputFileName 是无效的——预览对话框会覆盖这个设置。

### 3.2 自定义预览页面：缩放、导航、方向

QPrintPreviewDialog 内部使用 QPrintPreviewWidget 来渲染预览页面。QPrintPreviewWidget 提供了缩放、页面导航、视图模式等控制方法。虽然 QPrintPreviewDialog 的工具栏已经暴露了基本的缩放和导航按钮，但你可以通过访问内部的 QPrintPreviewWidget 来做更精细的控制。

```cpp
// 获取内部的预览 widget
auto *preview_widget = preview.findChild<QPrintPreviewWidget*>();
if (preview_widget) {
    // 设置缩放比例
    preview_widget->setZoomFactor(1.5);

    // 设置视图模式：单页 / 双页 / 所有页面
    preview_widget->setViewMode(QPrintPreviewWidget::AllPagesView);

    // 跳转到指定页
    preview_widget->setCurrentPage(3);
}
```

QPrintPreviewWidget::ViewMode 有三个选项：SinglePageView（一次显示一页，默认）、FacingPagesView（一次显示两页，左右并排，模拟书本翻页效果）、AllPagesView（一次显示所有页面的缩略图）。

页面方向（纵向/横向）的切换会影响 QPrinter 的页面配置。当用户在预览对话框的工具栏上切换方向时，QPrintPreviewDialog 会调用 QPrinter::setPageOrientation，然后重新触发 paintRequested 让你重新绘制内容。这就是为什么你的绘制逻辑必须从 QPrinter 动态读取页面配置——不能用缓存的旧值。

```cpp
void MyClass::on_paint_requested(QPrinter* printer)
{
    // 每次都从 printer 读取当前配置——用户可能在预览中改了方向
    QPageLayout::Orientation orientation = printer->pageLayout().orientation();
    QRectF page = printer->pageRect(QPrinter::DevicePixel);

    if (orientation == QPageLayout::Landscape) {
        // 横向布局的绘制逻辑
    } else {
        // 纵向布局的绘制逻辑
    }
}
```

### 3.3 添加自定义工具栏按钮

QPrintPreviewDialog 的工具栏上默认有缩放、导航、页面设置、打印等按钮。在工程中我们经常需要添加自定义按钮——比如"导出 PDF"（不经过打印机直接保存）、"页面水印设置"、"页眉页脚配置"等。

方法是找到对话框内部的 QToolBar，然后添加自定义的 QAction。

```cpp
QPrintPreviewDialog preview(&printer, this);
connect(&preview, &QPrintPreviewDialog::paintRequested,
        this, &MyClass::on_paint_requested);

// 找到内部工具栏
auto *toolbar = preview.findChild<QToolBar*>();
if (toolbar) {
    // 添加分隔符
    toolbar->addSeparator();

    // 添加"导出 PDF"按钮
    auto *export_action = toolbar->addAction("Export PDF");
    connect(export_action, &QAction::triggered, this, [&printer, this]() {
        QString path = QFileDialog::getSaveFileName(
            this, "Export PDF", QString(), "PDF Files (*.pdf)");
        if (!path.isEmpty()) {
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(path);
            // 需要重新触发一次绘制
            do_export_pdf(&printer);
        }
    });
}

preview.exec();
```

这里有几个注意事项。第一，findChild<QToolBar*>() 依赖于 QPrintPreviewDialog 的内部实现，虽然 Qt 6 系列中这个方法是稳定的，但理论上未来版本可能改变内部控件结构。如果你的应用需要长期维护，建议做 nullptr 检查并提供降级方案。

第二，在自定义按钮的槽函数中，如果你修改了 QPrinter 的配置（比如上面代码中设置 outputFileName），这些修改会影响到后续的 paintRequested 调用。因为 paintRequested 使用的就是同一个 QPrinter 对象。所以如果你在"导出 PDF"按钮中改了 outputFileName，之后用户点预览对话框自带的"打印"按钮时，输出可能会被错误地导向你设置的文件而不是打印机。解决方案是在自定义操作完成后恢复 QPrinter 的配置，或者在导出 PDF 时使用一个独立的 QPrinter 对象。

```cpp
// 安全的导出方式——使用独立的 QPrinter
connect(export_action, &QAction::triggered, this, [this]() {
    QString path = QFileDialog::getSaveFileName(
        this, "Export PDF", QString(), "PDF Files (*.pdf)");
    if (!path.isEmpty()) {
        QPrinter export_printer;  // 独立的 QPrinter，不影响预览
        export_printer.setOutputFormat(QPrinter::PdfFormat);
        export_printer.setOutputFileName(path);
        export_printer.setPageSize(printer.pageLayout().pageSize());
        export_printer.setPageMargins(printer.pageLayout().margins());
        do_export_pdf(&export_printer);
    }
});
```

### 3.4 预览与实际打印的一致性保证

这是打印预览功能中最容易被忽视但影响最大的问题——用户在预览中看到的效果必须和最终打印出来的效果完全一致。如果不一致，预览就失去了意义。

一致性的核心原则是：预览和打印必须使用完全相同的绘制代码。不要为预览写一套绘制逻辑、为打印写另一套——哪怕差异很小（比如字体大小差了 1pt），用户也会发现并失去对预览功能的信任。

```cpp
// 统一的绘制入口
void MyClass::do_paint(QPrinter* printer)
{
    QPainter painter(printer);
    QRectF page = printer->pageRect(QPrinter::DevicePixel);

    // 所有配置都从 printer 参数动态读取
    QFont body_font("Sans", 10);
    painter.setFont(body_font);

    // ... 统一的绘制逻辑 ...
}

// 预览时调用
void MyClass::on_paint_requested(QPrinter* printer)
{
    do_paint(printer);
}

// 直接打印时也调用同一个函数
void MyClass::on_print()
{
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        do_paint(&printer);
    }
}
```

有几个常见的不一致来源需要注意。第一是字体渲染差异——QPrintPreviewDialog 使用屏幕渲染来显示预览，而打印机使用打印机自身的渲染引擎。字体在屏幕和打印机上的 hinting 和抗锯齿处理可能不同，导致文字在预览和打印输出中的视觉效果有微妙差异。这不是代码能解决的问题，但差异通常很小，用户不太会注意到。

第二是颜色空间差异——屏幕使用 RGB 颜色，打印机通常使用 CMYK。如果你的内容中包含鲜艳的蓝色或绿色，在打印输出中可能会变暗变灰。QPrintPreviewDialog 的预览使用屏幕 RGB 渲染，所以预览中的颜色比实际打印更鲜艳。如果你的应用对颜色准确性有要求（比如打印产品目录），需要提醒用户这个差异。

第三是分辨率差异——预览的分辨率是屏幕分辨率（通常 96 DPI），打印的分辨率可能是 300 或 600 DPI。这意味着打印输出中的细节（比如细线、小文字）比预览中更清晰。这不是 bug，是物理特性的差异。

## 4. 踩坑预防

第一个坑是 paintRequested 被多次调用导致的副作用。预览对话框在用户修改设置时会重新触发 paintRequested，如果你的绘制函数有副作用（比如往文件写数据、修改全局状态），多次调用会产生重复或错误的结果。解决方案是确保绘制函数是纯函数——只从 QPrinter 参数和输入数据读取，只向 QPainter 写入，不修改外部状态。

第二个坑是在自定义工具栏按钮中修改了共享 QPrinter 的配置。如 3.3 节所述，预览对话框和自定义按钮共享同一个 QPrinter 对象。如果你在自定义按钮中改了 outputFormat 或 outputFileName，后续的 paintRequested 调用会使用被修改后的配置，导致预览行为异常。解决方案是在需要修改 QPrinter 配置时使用独立的 QPrinter 对象，不要修改预览对话框使用的那个。

第三个坑是 findChild 获取内部控件的脆弱性。通过 findChild<QToolBar*>() 或 findChild<QPrintPreviewWidget*>() 获取内部控件依赖于 Qt 的内部实现，可能在版本升级后失效。解决方案是始终做 nullptr 检查，并且不要过度依赖内部控件的特定行为。如果 findChild 返回 nullptr，降级到不使用自定义功能的方式。

第四个坑是预览中页面数量与实际打印页面数量不一致。这通常发生在绘制逻辑使用了外部状态（比如当前时间、随机数）或者依赖了某些在预览上下文中不存在的资源。由于 paintRequested 在预览和打印时使用同一套代码，如果代码中依赖了只在某个上下文中有效的数据，就会导致页面数量不一致。解决方案是绘制逻辑不依赖上下文相关的状态，所有需要的数据都通过参数或成员变量传入。

## 5. 练习项目

练习项目：带自定义导出按钮的打印预览对话框。我们要实现一个报表预览功能，打开 QPrintPreviewDialog 时工具栏上多一个"Export PDF"按钮和一个"Add Watermark"勾选项。Export PDF 点击后弹出文件保存对话框，将报表保存为 PDF（使用独立的 QPrinter，不影响预览）。Add Watermark 勾选后，每一页的右下角绘制一个半透明的 "DRAFT" 水印文字。报表内容使用硬编码的数据（比如 50 行表格数据），分页输出。

完成标准是：预览正确显示所有页面；勾选 Add Watermark 后预览立即更新（重新触发 paintRequested）并显示水印；Export PDF 导出的 PDF 内容和预览完全一致（包括水印状态）；多次切换 Add Watermark 不产生重复或残留内容。提示几个关键点：用一个 bool 成员变量控制水印是否绘制，Add Watermark 的 toggled 信号连接到更新这个 bool 并触发预览刷新（通过 findChild<QPrintPreviewWidget*>()->updatePreview()）；Export PDF 使用独立的 QPrinter 对象；绘制函数中检查水印标志决定是否画水印。

## 6. 官方文档参考链接

[Qt 文档 · QPrintPreviewDialog](https://doc.qt.io/qt-6/qprintpreviewdialog.html) -- 打印预览对话框类，包含 paintRequested 信号和工具栏配置说明

[Qt 文档 · QPrintPreviewWidget](https://doc.qt.io/qt-6/qprintpreviewwidget.html) -- 预览控件类，包含缩放、导航、视图模式方法

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机类，页面配置和输出格式设置

[Qt 文档 · QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏类，用于向预览对话框添加自定义操作按钮

---

到这里，QPrintPreviewDialog 的进阶内容就过了一遍。paintRequested 的核心机制是每次配置变化时重新触发绘制——你的绘制函数必须是幂等的，不能有副作用。预览页面的缩放和导航可以通过内部的 QPrintPreviewWidget 控制，但 findChild 的方式需要注意版本兼容性。自定义工具栏按钮能显著提升预览对话框的实用性，但切记用独立的 QPrinter 做导出操作，不要修改共享的 QPrinter 配置。预览与打印的一致性保证依赖于使用完全相同的绘制代码，所有配置都从 QPrinter 参数动态读取。把这些搞清楚了，你的打印预览就能从"能看"变成"靠谱"了。
