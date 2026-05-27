---
title: "3.72 QPrinter 进阶"
description: "入门篇我们学会了 QPrinter 的基本用法：设置输出格式、创建 QPainter 绑定 QPrinter、绘制内容、结束绘制。进阶篇要解决的是工程中真正复杂的问题——怎么把长内容正确分页、怎么在每一页上画页眉页脚、以及页面坐标系和绘制坐标系之间的换算关系。"
---

# 现代Qt开发教程（进阶篇）3.72——QPrinter 进阶

## 1. 前言 / 打印不是把内容画上去就完事的

入门篇我们学会了 QPrinter 的基本用法：设置输出格式、创建 QPainter 绑定 QPrinter、绘制内容、结束绘制。说实话，打印一页简单内容到 PDF 已经够用了——但如果你的应用需要打印报表、导出多页文档、或者在每一页上添加统一的页眉页脚，事情就变得复杂了。分页逻辑、页面边距与绘制区域的计算、每页都要重复绘制的页眉页脚——这些才是打印功能里真正费心思的部分。

这篇文章要深挖的是四个核心议题：QPrinter 的输出格式和页面配置（setPageSize、setPageMargins），QPainter + QPrinter 的分页绘制逻辑（newPage、内容切分），自定义页眉页脚的绘制（每页绘制前后插入页眉页脚），以及页面坐标系和绘制区域的精确换算。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QPrinter 属于 QtPrintSupport 模块，需要在 CMakeLists.txt 中添加 `find_package(Qt6 REQUIRED COMPONENTS PrintSupport)` 并链接 `Qt6::PrintSupport`。输出 PDF 格式不需要额外的依赖，输出到物理打印机则需要系统安装打印机驱动。

## 3. 核心概念讲解

### 3.1 QPrinter 输出格式设置

QPrinter 支持多种输出格式，通过 setOutputFormat 设置。QPrinter::NativeFormat 是默认值，使用系统原生的打印驱动输出到物理打印机。QPrinter::PdfFormat 直接输出 PDF 文件，不经过打印机驱动——这是最常用的开发调试方式，因为不需要物理打印机就能验证打印效果。QPrinter::PostScriptFormat 输出 PostScript 文件（在某些平台上不可用）。

```cpp
QPrinter printer;
printer.setOutputFormat(QPrinter::PdfFormat);
printer.setOutputFileName("report.pdf");
```

输出 PDF 时，setOutputFileName 指定文件路径。如果输出格式是 NativeFormat 且没有调用 setOutputFileName，打印内容会发送到系统默认打印机。这里有一个工程中很实用的技巧：开发阶段始终使用 PdfFormat 输出到文件，方便调试分页和布局，调试完成后再切换到 NativeFormat 适配真实打印机。

QPrinter 还支持 setPrinterName 选择特定的打印机。如果你的系统安装了多台打印机（包括虚拟 PDF 打印机），可以用 QPrinterInfo::availablePrinters() 获取可用打印机列表，然后让用户选择。

### 3.2 setPageSize 与 setPageMargins 页面配置

页面配置是打印的第一步——不搞清楚纸张大小和边距，后面的绘制全都是空中楼阁。

setPageSize 设置纸张大小，可以用预定义的尺寸（QPageSize::A4、QPageSize::Letter 等）或自定义尺寸。setPageMargins 设置页面边距，有四个值分别对应上、右、下、左（注意是 TRBL 顺序，不是常用的上左下右）。边距可以用 QPageLayout::Millimeter 或 QPageLayout::Point 等单位指定。

```cpp
printer.setPageSize(QPageSize(QPageSize::A4));
printer.setPageMargins(QMarginsF(15, 20, 15, 20), QPageLayout::Millimeter);
```

这里有一个关键概念要搞清楚：QPrinter 的 pageRect 和 paperRect 的区别。paperRect 是整张纸的矩形区域（从 (0,0) 到纸张右下角），pageRect 是减去边距后可用的绘制区域。QPainter 在 QPrinter 上绘制时，坐标系的原点是 paperRect 的左上角，也就是纸张的物理左上角。所以如果你想在边距内的区域绘制内容，需要从 pageRect 的左上角开始。

```cpp
QRectF page_rect = printer.pageRect(QPrinter::DevicePixel);
// page_rect 的 top-left 就是边距后的起始位置
// page_rect 的 width/height 就是可绘制区域的宽高
```

这里还有一个容易搞混的单位问题。QPrinter 内部使用的单位是 DevicePixel（设备像素），和屏幕像素不同。打印机的分辨率通常是 72 DPI 或 300 DPI，而屏幕可能是 96 DPI 或更高。如果你习惯用毫米来思考布局，建议始终用 QPageLayout::Millimeter 设置边距，然后在绘制时通过 pageRect(QPrinter::Millimeter) 获取毫米单位的可用区域，这样不用关心 DPI 转换。

### 3.3 QPainter + QPrinter 的分页绘制逻辑

单页内容打印很简单——画完就结束。但多页内容的分页逻辑才是打印功能的核心难点。

分页的基本思路是：我们有一个"逻辑内容流"（比如一个很长的 QTextDocument 或者一大段自定义绘制内容），需要把它切分成若干页。每一页开始时，我们创建 QPainter，在当前页的 pageRect 内绘制内容，画满一页后调用 QPrinter::newPage() 创建新的一页，继续绘制剩余内容。

```cpp
QPainter painter(&printer);
QRectF page = printer.pageRect(QPrinter::DevicePixel);

qreal y_offset = 0;
const qreal page_height = page.height();

for (const auto& content_block : content_blocks) {
    qreal block_height = calculate_block_height(content_block, page.width());

    // 当前页放不下了，换页
    if (y_offset + block_height > page_height) {
        printer.newPage();
        y_offset = 0;
        // 可选：在新页面绘制页眉
        draw_header(&painter, page);
        y_offset += header_height;
    }

    draw_content_block(&painter, content_block, page.x(), page.y() + y_offset, page.width());
    y_offset += block_height;
}

painter.end();
```

这段伪代码展示了分页的核心逻辑：维护一个 y_offset 追踪当前页面的垂直位置，每画一个内容块前检查是否能放得下，放不下就 newPage 并重置 y_offset。实际工程中，这个逻辑需要根据你的内容类型做调整——如果是文本内容，可能需要按行切分；如果是表格，可能需要在行边界处分页；如果是图片，可能需要缩放以适应页面。

这里有一个非常重要的行为需要理解：调用 newPage() 后，QPainter 的绘制状态（画笔、画刷、字体等）会被保留——你不需要重新设置。但 QPainter 的坐标系不会自动重置，也就是说调用 newPage() 后 QPainter 的坐标原点仍然是上一页的某个位置。所以换页后必须手动将 y 坐标重置（或者用 save/restore + translate 来管理坐标）。

现在有一道调试题给大家。下面这段代码有什么问题？

```cpp
QPainter painter(&printer);
QRectF page = printer.pageRect(QPrinter::DevicePixel);

for (int i = 0; i < 50; ++i) {
    painter.drawText(page.toRect(), Qt::AlignTop | Qt::TextWordWrap,
                     QString("Line %1: some long text...").arg(i));
    if ((i + 1) % 10 == 0) {
        printer.newPage();
    }
}
```

问题出在每一行文字都画在同一个位置——page.toRect() 的 top-left。drawText 不会自动偏移，所以 10 行文字全部重叠在第一行。正确的做法是维护一个 y 坐标，每次 drawText 后根据 QFontMetrics 计算文字高度并累加 y。另外 newPage() 后 y 坐标需要重置为 page.top()。

### 3.4 自定义页眉页脚绘制

页眉页脚是打印报表的标配——页码、文档标题、打印日期、公司名称这些信息通常放在每一页的顶部和底部。实现方式是在每一页的绘制过程中，先画页眉，再画内容，最后画页脚。

关键是要把可用绘制区域正确地分为三个部分：页眉区域、内容区域、页脚区域。页眉和页脚的高度是固定的（你决定的），内容区域的高度 = pageRect 高度 - 页眉高度 - 页脚高度。

```cpp
const qreal kHeaderHeight = 30.0;  // 页眉高度
const qreal kFooterHeight = 30.0;  // 页脚高度

void draw_header(QPainter* painter, const QRectF& page, int page_num)
{
    // 页眉画在 page 顶部
    QRectF header_rect(page.x(), page.y(), page.width(), kHeaderHeight);
    painter->drawText(header_rect, Qt::AlignVCenter | Qt::AlignLeft,
                      "Document Title");
    painter->drawText(header_rect, Qt::AlignVCenter | Qt::AlignRight,
                      QString("Page %1").arg(page_num));
    // 页眉下方的分隔线
    qreal line_y = header_rect.bottom() + 2;
    painter->drawLine(QPointF(page.x(), line_y), QPointF(page.right(), line_y));
}

void draw_footer(QPainter* painter, const QRectF& page, const QString& date)
{
    // 页脚画在 page 底部
    QRectF footer_rect(page.x(), page.bottom() - kFooterHeight,
                       page.width(), kFooterHeight);
    painter->drawText(footer_rect, Qt::AlignVCenter | Qt::AlignCenter,
                      QString("Printed on %1").arg(date));
}
```

内容区域就是页眉下方到页脚上方之间的空间：

```cpp
QRectF content_rect(page.x(), page.y() + kHeaderHeight,
                    page.width(),
                    page.height() - kHeaderHeight - kFooterHeight);
```

这个 content_rect 就是你在分页逻辑中用于放置实际内容的可用区域——y_offset 从 content_rect.top() 开始累加，超过 content_rect.bottom() 就换页。

页眉页脚绘制的一个常见需求是"总页数"，比如 "Page 3 of 12"。但问题是：在打印过程中你不知道总共有多少页（除非先做一次预计算）。有两种解决方案：第一种是先做一次"干运行"（dry run）——只计算内容分页，不实际绘制，统计总页数后再做第二次"真正绘制"。第二种是先输出到 PDF，然后读取 PDF 的页数，再在页眉中填入总页数——但这需要修改 PDF 文件，比较麻烦。工程中推荐第一种方案，虽然需要两次遍历但逻辑清晰可靠。

## 4. 踩坑预防

第一个坑是 QPainter 绑定 QPrinter 后坐标系原点是 paperRect 的左上角而非 pageRect 的左上角。这意味着如果你直接用 (0, 0) 作为绘制起点，内容会画在纸张的物理边缘上——也就是边距区域内。解决方案是始终使用 pageRect() 获取可绘制区域的起始坐标，从 pageRect.topLeft() 开始绘制。

第二个坑是 newPage() 后 QPainter 坐标不重置。调用 newPage() 只是告诉 QPrinter "开始新的一页"，QPainter 的所有变换矩阵（translate、scale 等）都保持不变。如果你在第一页用了 translate 移动了绘制位置，到第二页这个 translate 还在——你的内容可能画在页面之外看不见。解决方案是在每页开始时用 save/restore 管理状态，或者在换页后显式 resetMatrix 重新设置变换。

第三个坑是 setFullPage 的影响。QPrinter 默认 setFullPage(false)，此时 QPainter 的原点是 pageRect 的左上角（已经减去了边距）。如果调用 setFullPage(true)，QPainter 的原点变成 paperRect 的左上角（纸张物理边缘），此时 pageRect 和 paperRect 相同，你需要自己计算边距。很多教程代码没有说明这个设置，导致边距行为不一致。建议保持默认的 setFullPage(false)，不要轻易改动。

第四个坑是高分辨率打印机上的字体大小问题。打印机的 DPI 通常是 300 或 600，而屏幕 DPI 是 96。如果你在屏幕上用了 12pt 的字体觉得大小合适，在打印机上同样的 12pt 字体在视觉上会小很多——因为打印机的点更密集。解决方案是使用 QFont 的 pointSize 而不是 pixelSize——pointSize 是物理单位（1 pt = 1/72 英寸），在不同 DPI 的设备上能保持一致的物理大小。如果你确实需要更大的字体，调整 pointSize 而不是切换到 pixelSize。

## 5. 练习项目

练习项目：多页报表打印器。我们要实现一个简单的报表打印功能：将一个 QStringList 的长文本列表（比如 100 行数据）分页输出到 PDF，每页都有页眉（报表标题 + 页码）和页脚（打印日期），内容区域均匀分布文本行。要求支持 A4 纸张，上下左右边距各 15mm，页眉高度 25pt，页脚高度 25pt。

完成标准是：输出的 PDF 能正确分页，每页行数合理不溢出；页眉显示 "Report" 和 "Page X of Y"（总页数需要预计算）；页脚显示打印日期；每页内容不与页眉页脚重叠；最后一页即使只有少量内容也有正确的页眉页脚。提示几个关键点：先做一次 dry run 计算总页数（用 QFontMetrics 计算每行高度，除以每页内容区域高度得到行数/页），然后再做正式绘制；换页时别忘了重置 y 坐标并重新绘制页眉。

## 6. 官方文档参考链接

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机类，包含输出格式、页面配置、newPage 方法说明

[Qt 文档 · QPageSize](https://doc.qt.io/qt-6/qpagesize.html) -- 页面尺寸类，包含 A4/Letter 等预定义尺寸和自定义尺寸

[Qt 文档 · QPageLayout](https://doc.qt.io/qt-6/qpagelayout.html) -- 页面布局类，包含边距设置和 pageRect/paperRect 计算

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- 绘制类，绑定 QPrinter 后的坐标系和绘制行为

---

到这里，QPrinter 的进阶内容就过了一遍。输出格式用 PdfFormat 开发调试最方便，上线再切 NativeFormat。pageRect 和 paperRect 的区别是打印布局的基础——搞不清这个，边距和坐标系全都会乱。分页逻辑的核心是维护 y_offset 累加并在超出可用区域时 newPage，换页后必须重置坐标。页眉页脚需要在每页绘制前后插入固定区域，总页数通过 dry run 预计算。把这些搞清楚了，你的打印功能就能正确地处理多页内容了。
