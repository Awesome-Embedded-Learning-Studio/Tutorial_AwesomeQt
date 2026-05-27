---
title: "3.73 QPrintDialog 进阶"
description: "入门篇我们学会了 QPrintDialog 的基本用法：创建对话框、exec 执行、用户确认后获取 QPrinter 配置。进阶篇要深挖的是打印范围控制、选项配置的精细行为、非阻塞打印对话框与结果处理，以及与 QPrintPreviewDialog 的配合使用流程。"
---

# 现代Qt开发教程（进阶篇）3.73——QPrintDialog 进阶

## 1. 前言 / 打印对话框不只是"点一下确认"那么简单

入门篇我们学会了 QPrintDialog 的基本用法：创建对话框、exec 执行、用户确认后获取 QPrinter 配置。说实话，如果你的应用只打印全部内容，这一步就够了。但实际工程中用户的需求远不止于此——他们可能只想打印第 3 页到第 7 页，可能想双面打印，可能想打印 3 份并逐份排列。而且有些场景下你不能用 exec 阻塞整个应用——比如你的主界面有一个实时预览窗口，用户调整打印选项时需要实时看到效果变化。

这篇文章要深挖的是四个核心议题：setPrintRange / setFromTo 控制打印范围，setOption 配置对话框选项的精细行为，非阻塞方式打开打印对话框并处理结果，以及 QPrintDialog 和 QPrintPreviewDialog 的协作流程。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QPrintDialog 属于 QtPrintSupport 模块，需要在 CMakeLists.txt 中添加 `find_package(Qt6 REQUIRED COMPONENTS PrintSupport)` 并链接 `Qt6::PrintSupport`。在 Linux 上 QPrintDialog 可能使用 Qt 自定义实现或 CUPS 对话框，取决于平台插件配置。

## 3. 核心概念讲解

### 3.1 setPrintRange / setFromTo 打印范围控制

QPrintDialog 提供了三种打印范围模式，通过 setPrintRange 设置：QPrintDialog::AllPages（打印全部页面，默认值）、QPrintDialog::Selection（只打印用户选中的内容）、QPrintDialog::PageRange（打印指定的页码范围）。

当设置为 PageRange 模式时，需要用 setFromTo 指定起始页和结束页：

```cpp
QPrintDialog dialog(&printer, parent);

// 设置为页码范围模式
dialog.setPrintRange(QPrintDialog::PageRange);
dialog.setFromTo(3, 7);  // 只打印第 3 到第 7 页

if (dialog.exec() == QDialog::Accepted) {
    int from = printer.fromPage();  // 3
    int to = printer.toPage();      // 7（用户可能在对话框中修改了范围）
    // 根据 from/to 执行打印
}
```

这里有一个非常重要的行为需要搞清楚：setPrintRange 和 setFromTo 设置的是对话框的初始值，用户可以在对话框中修改这些设置。也就是说，你设置了 setFromTo(3, 7)，但用户在对话框里可能改成了 setFromTo(1, 10) 或者直接切换成了 AllPages。用户点击确认后，你应该从 QPrinter 上读取最终的 fromPage / toPage，而不是用你设置的初始值。

另一个需要注意的点是：Selection 模式的实际行为取决于你的应用。QPrintDialog 本身不知道"用户选中了什么内容"——它只是在对话框 UI 上显示一个"Selection"单选按钮。如果你的应用支持文本选择，需要自己检测当前是否有选中内容，然后在打印时只输出选中的部分。如果用户选择了 Selection 模式但你的应用没有提供选中内容，打印结果可能是空的或者不可预期的。

```cpp
if (dialog.exec() == QDialog::Accepted) {
    switch (dialog.printRange()) {
    case QPrintDialog::AllPages:
        print_all_pages(&printer);
        break;
    case QPrintDialog::Selection:
        print_selected_content(&printer);  // 你自己实现
        break;
    case QPrintDialog::PageRange:
        print_page_range(&printer, printer.fromPage(), printer.toPage());
        break;
    default:
        break;
    }
}
```

### 3.2 setOption 选项配置

QPrintDialog 提供了一组选项标志，通过 setOption 或 setOptions 配置。这些选项控制对话框中显示哪些 UI 元素。

QPrintDialog::PrintSelection 控制是否显示"Selection"单选按钮。QPrintDialog::PrintPageRange 控制是否显示"Pages"输入框和页码范围选项。QPrintDialog::PrintCollateCopies 控制是否显示"Collate"逐份打印选项。QPrintDialog::PrintShowPageSize 控制是否显示页面大小选择（Qt 6.x 部分平台支持）。QPrintDialog::DontUseSheet 在 macOS 上阻止将打印对话框显示为 sheet。

```cpp
QPrintDialog dialog(&printer, parent);
dialog.setOption(QPrintDialog::PrintSelection, true);
dialog.setOption(QPrintDialog::PrintPageRange, true);
dialog.setOption(QPrintDialog::PrintCollateCopies, true);
```

这里有几个行为边界需要知道。第一，PrintSelection 选项只是让对话框显示"Selection"按钮，它不会自动帮你处理选中内容的打印逻辑。如果你设置了 PrintSelection 但没有在打印时检测 printRange() == Selection，用户选了 Selection 也没用。

第二，setFromTo 设置的范围和 PrintPageRange 选项有交互关系。如果你没有启用 PrintPageRange 选项，setFromTo 设置的范围不会在对话框中显示（但内部仍然生效）。如果你启用了 PrintPageRange，用户在对话框中输入的页码范围会被验证——超出文档实际页码范围的值可能被忽略或截断，具体行为取决于平台实现。

第三，在 macOS 上，QPrintDialog 使用系统原生的打印对话框（NSPrintPanel），选项的可用性和行为由 macOS 决定。某些 setOption 在 macOS 上可能没有效果，因为原生对话框不支持隐藏某些控件。如果需要完全自定义打印对话框的外观，可以考虑使用 Qt 自己的实现（通过 AA_DontUseNativeDialogs 属性），但外观会不如原生对话框自然。

### 3.3 非阻塞打印对话框与结果处理

QPrintDialog::exec() 是阻塞调用——它会打开一个模态对话框，用户确认或取消后才返回。在大多数场景下这没问题，但如果你需要更灵活的控制（比如在对话框打开时更新其他 UI），可以用 open() 配合信号槽实现非阻塞模式。

```cpp
auto *dialog = new QPrintDialog(&printer, this);
dialog->setOption(QPrintDialog::PrintPageRange, true);

// 非阻塞打开
connect(dialog, &QDialog::accepted, this, [this, dialog]() {
    // 用户点击了确认
    do_print(&printer);
    dialog->deleteLater();
});
connect(dialog, &QDialog::rejected, dialog, &QDialog::deleteLater);

dialog->open();  // 非阻塞，立即返回
```

open() 会显示对话框但不会阻塞事件循环——对话框打开期间，你的应用仍然可以响应其他事件（比如定时器、网络回调等）。当用户点击确认或取消时，accepted 或 rejected 信号被发射，你在槽函数中处理结果。

不过说实话，非阻塞打印对话框的使用场景比较少。大多数应用的打印流程是线性的：用户点打印 -> 对话框 -> 确认 -> 打印。如果你确实需要非阻塞，open() 方式比 exec() 更灵活。但要注意 QPrintDialog 的生命周期管理——用 open() 时对话框不会自动删除，需要手动 deleteLater。

### 3.4 与 QPrintPreviewDialog 配合使用

在工程实践中，QPrintDialog 和 QPrintPreviewDialog 经常配合使用。典型的流程是：用户点击"打印预览"按钮 -> 打开 QPrintPreviewDialog 显示预览 -> 用户在预览中确认效果后点击"打印到打印机" -> QPrintPreviewDialog 内部使用 QPrintDialog 让用户选择打印机和选项。

```cpp
QPrinter printer;
printer.setOutputFormat(QPrinter::PdfFormat);  // 预览用 PDF 格式

QPrintPreviewDialog preview(&printer, this);
connect(&preview, &QPrintPreviewDialog::paintRequested,
        this, &MyClass::on_paint_requested);

preview.exec();  // 打开预览对话框
```

QPrintPreviewDialog 内部有一个"打印"按钮。当用户在预览对话框中点击"打印"时，QPrintPreviewDialog 会打开一个 QPrintDialog 让用户选择打印机和打印选项，然后将之前 paintRequested 回调中绘制的内容输出到用户选择的打印机。这意味着你只需要实现一次绘制逻辑（on_paint_requested），预览和打印都能用。

```cpp
void MyClass::on_paint_requested(QPrinter* printer)
{
    // 统一的绘制逻辑——预览和打印共用
    QPainter painter(printer);
    // ... 分页绘制内容 ...
}
```

这里有一个重要的协作细节：如果你在打开 QPrintPreviewDialog 之前对 QPrinter 做了配置（比如 setPageSize、setPageMargins），这些配置会传递到预览对话框内部。用户在预览对话框中修改的设置（比如缩放比例、页面方向）会反映到最终的打印输出中。但有一个例外：如果用户在预览对话框的"打印"按钮中通过 QPrintDialog 选择了不同的打印机，这个新打印机的默认配置可能覆盖你之前的设置。所以建议在 paintRequested 回调中始终从 printer 参数重新读取配置，而不是缓存之前的配置值。

现在有一个工程场景需要考虑。如果你的应用同时提供"打印"和"打印预览"两个入口，怎么处理？推荐的做法是："打印"按钮直接打开 QPrintDialog（exec 方式），在 accepted 回调中执行绘制逻辑。"打印预览"按钮打开 QPrintPreviewDialog，paintRequested 回调使用同一套绘制逻辑。这样避免了代码重复，同时两个入口的用户体验都是正确的。

## 4. 踩坑预防

第一个坑是 setFromTo 的页码范围与实际内容页数不一致。如果你设置了 setFromTo(1, 100) 但实际内容只有 5 页，QPrinter 不会报错——它只会输出你实际绘制的内容。也就是说 fromPage/toPage 只是一个"提示"，不会强制 QPrinter 生成指定数量的页面。解决方案是在打印逻辑中自己检查页码范围，只在 fromPage 到 toPage 之间绘制内容，超出范围的部分跳过。

第二个坑是 macOS 上 QPrintDialog 使用原生对话框导致 setOption 部分无效。macOS 的 NSPrintPanel 有自己的 UI 布局，不能像 Qt 自定义对话框那样精确控制每个控件的显隐。如果你需要完全一致的跨平台行为，可以设置 `QApplication::setAttribute(Qt::AA_DontUseNativeDialogs)` 强制使用 Qt 实现。但这样做会失去原生外观，需要权衡。建议的做法是在 macOS 上接受原生对话框的行为差异，在 Windows 和 Linux 上使用 Qt 实现并精确控制选项。

第三个坑是 QPrintDialog::open() 后对话框被意外删除。用 open() 打开非阻塞对话框时，对话框的生命周期需要你自己管理。如果对话框的 parent 被删除了，对话框也会跟着被删除（Qt 的对象树机制），你的信号槽连接就失效了。解决方案是确保对话框有合适的 parent，或者在对话框打开后断开与可能被删除的对象的关联。

第四个坑是 Selection 模式下没有选中内容时的空打印。如果你启用了 PrintSelection 选项但应用没有提供"选中内容"的概念（比如一个只读的报表），用户选择了 Selection 模式后打印结果可能是空的。解决方案是：如果不支持选中内容，不要启用 PrintSelection 选项；或者在选择 Selection 模式时回退到 AllPages 并提示用户。

## 5. 练习项目

练习项目：带打印范围选择的文档打印器。我们要实现一个简单的文本编辑器（基于 QPlainTextEdit），支持"打印"和"打印预览"两个功能。"打印"按钮打开 QPrintDialog，启用 PrintPageRange 选项，用户可以选择打印全部或指定页码范围。"打印预览"按钮打开 QPrintPreviewDialog，用户在预览中确认后可以点击打印。两个功能共用一套绘制逻辑，将 QPlainTextEdit 的内容按页分页绘制到 QPrinter 上。

完成标准是：打印对话框中页码范围输入框正常可用，用户指定范围后只打印对应页面；预览对话框正确显示所有页面内容；从预览对话框点击打印后能正确输出到打印机或 PDF；两个入口的打印结果完全一致。提示几个关键点：绘制逻辑统一放在一个函数中接受 QPrinter* 参数；分页时根据 fromPage/toPage 决定是否绘制当前页；QPlainTextEdit 的内容可以用 QTextDocument 的 print 方法或者自己分页绘制。

## 6. 官方文档参考链接

[Qt 文档 · QPrintDialog](https://doc.qt.io/qt-6/qprintdialog.html) -- 打印对话框类，包含 PrintRange、setOption、accepted 信号说明

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机类，fromPage/toPage 获取最终打印范围

[Qt 文档 · QPrintPreviewDialog](https://doc.qt.io/qt-6/qprintpreviewdialog.html) -- 打印预览对话框，与 QPrintDialog 配合使用的完整流程

[Qt 文档 · QAbstractPrintDialog](https://doc.qt.io/qt-6/qabstractprintdialog.html) -- 打印对话框基类，PrintRange 枚举定义

---

到这里，QPrintDialog 的进阶内容就过了一遍。打印范围控制要注意 setPrintRange 和 setFromTo 只是初始值，用户可以在对话框中修改，最终值从 QPrinter 上读取。选项配置的精细行为受平台影响——macOS 上原生对话框不能完全控制所有选项。非阻塞打印对话框用 open() + 信号槽实现，但要注意生命周期管理。和 QPrintPreviewDialog 配合时，绘制逻辑只需要实现一次，两个入口共享同一套代码。把这些搞清楚了，你的打印功能就能覆盖"全部打印/指定范围/选中内容/预览后打印"这些常见场景了。
