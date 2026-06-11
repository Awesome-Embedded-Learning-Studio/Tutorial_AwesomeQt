---
title: "5.7 打印进阶：复杂报表生成与多页面布局"
description: "入门篇我们把 QPrinter + QPainter 的基本打印流程跑通了——创建打印设备、画文字和简单图形、输出到 PDF 或打印机。打印一张简单的标签或收据确实够用了。但如果你要打印复杂的多栏报表——表头表尾、分页、汇总行、图表嵌入——入门篇的那套方案就不够了。"
---

# 现代Qt开发教程（进阶篇）5.7——打印进阶：复杂报表生成与多页面布局

## 1. 前言 / 从「打印一页」到「打印一本报表」

入门篇我们把 QPrinter + QPainter 的基本打印流程跑通了——创建打印设备、画文字和简单图形、输出到 PDF 或打印机。打印一张简单的标签或收据确实够用了。但如果你要打印复杂的多栏报表——表头表尾、分页、汇总行、图表嵌入——入门篇的那套方案就不够了。

复杂报表的核心挑战是布局计算。你需要精确控制每个元素在页面上的位置，处理文本自动换行、表格列宽分配、跨页分断（一行数据不能被切到两页上）等细节。QPainter 是一个底层绘图 API，它不帮你做这些——你需要自己写布局引擎。

这篇我们把表头表尾的自动绘制、数据表格的分页逻辑、以及 QPainter 文本排版工具这三个核心能力拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::PrintSupport 模块（注意：该模块在 Qt 6 中从 QtWidgets 中独立出来），CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS PrintSupport)` 引入。

## 3. 核心概念讲解

### 3.1 报表页面模型——表头、表尾、内容区

一个典型的报表页面分三个区域：页眉（page header）——每页顶部显示标题、日期、公司名称；内容区（content area）——数据行；页脚（page footer）——页码、总页数。打印时每页的三区域位置固定，内容区的高度 = 页面高度 - 页眉高度 - 页脚高度。

```cpp
class ReportPrinter
{
public:
    void printReport(const QList<QMap<QString, QVariant>> &data)
    {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName("report.pdf");
        printer.setPageSize(QPageSize(QPageSize::A4));

        QPainter painter;
        if (!painter.begin(&printer)) return;

        // 页面尺寸（单位：点，1点≈0.35mm）
        QRectF pageRect = printer.pageRect(QPrinter::Point);
        const double headerHeight = 50;
        const double footerHeight = 30;
        const double contentHeight = pageRect.height()
                                     - headerHeight - footerHeight;

        const double rowHeight = 20;
        const int rowsPerPage = static_cast<int>(
            contentHeight / rowHeight);

        int totalPages = (data.size() + rowsPerPage - 1) / rowsPerPage;
        int currentPage = 0;

        for (int row = 0; row < data.size(); ) {
            if (row > 0 && row % rowsPerPage == 0) {
                printer.newPage();
                currentPage++;
            }

            double yOffset = 0;

            // 绘制页眉
            if (row % rowsPerPage == 0) {
                drawHeader(&painter, pageRect, headerHeight,
                           currentPage + 1, totalPages);
                yOffset = headerHeight;
            }

            // 绘制数据行
            int rowsThisPage = qMin(rowsPerPage,
                                     data.size() - row);
            drawTableRows(&painter, data, row, rowsThisPage,
                          pageRect, yOffset, rowHeight);

            // 绘制页脚
            drawFooter(&painter, pageRect, footerHeight,
                       currentPage + 1, totalPages);

            row += rowsThisPage;
        }

        painter.end();
    }

private:
    void drawHeader(QPainter *p, const QRectF &page,
                     double height, int pageNum, int totalPages)
    {
        QFont headerFont("Arial", 14, QFont::Bold);
        p->setFont(headerFont);
        p->drawText(QRectF(page.left(), page.top(),
                            page.width(), height),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     "Sales Report");
        p->drawText(QRectF(page.left(), page.top(),
                            page.width(), height),
                     Qt::AlignRight | Qt::AlignVCenter,
                     QDate::currentDate().toString("yyyy-MM-dd"));

        // 页眉底部线
        p->drawLine(page.left(), page.top() + height,
                     page.right(), page.top() + height);
    }

    void drawFooter(QPainter *p, const QRectF &page,
                     double height, int pageNum, int totalPages)
    {
        double y = page.bottom() - height;
        QFont footerFont("Arial", 9);
        p->setFont(footerFont);
        p->drawText(QRectF(page.left(), y, page.width(), height),
                     Qt::AlignCenter,
                     QString("Page %1 of %2").arg(pageNum).arg(totalPages));
    }

    void drawTableRows(QPainter *p,
                        const QList<QMap<QString, QVariant>> &data,
                        int startRow, int count,
                        const QRectF &page, double yOffset,
                        double rowHeight)
    {
        QFont cellFont("Arial", 10);
        p->setFont(cellFont);

        for (int i = 0; i < count; ++i) {
            double y = yOffset + i * rowHeight;
            const auto &row = data[startRow + i];

            // 绘制单元格
            p->drawText(QRectF(page.left() + 10, y, 200, rowHeight),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         row["name"].toString());
            p->drawText(QRectF(page.left() + 220, y, 150, rowHeight),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(row["amount"].toDouble(), 'f', 2));

            // 行分隔线
            p->drawLine(page.left(), y + rowHeight,
                         page.right(), y + rowHeight);
        }
    }
};
```

分页逻辑的关键是 `rowsPerPage` 的计算。内容区高度除以行高得到每页最多能放多少行。数据循环中用 `row % rowsPerPage == 0` 判断是否需要换页。换页时调用 `printer.newPage()`，然后重新绘制页眉和页脚。

### 3.2 文本自动换行与 QTextDocument

QPainter 的 `drawText()` 支持 `Qt::TextWordWrap` 标志做自动换行，但它的控制粒度有限。如果单元格中的文本可能很长并且需要精确控制行数，建议用 `QTextDocument` 做排版。

```cpp
// 用 QTextDocument 做精确排版
QTextDocument doc;
doc.setPageSize(QSizeF(cellWidth, maxHeight));
doc.setPlainText(longText);
doc.setDefaultFont(cellFont);

// 渲染到 QPainter
painter->save();
painter->translate(x, y);
doc.drawContents(painter);
painter->restore();

// 检查实际高度（用于分页判断）
double actualHeight = doc.size().height();
```

`QTextDocument` 会根据页面宽度和字体自动计算换行和分页。`doc.size()` 返回文档的实际尺寸，你可以用这个值判断一行内容是否超出了当前页的内容区域，如果超出就在下一页绘制。

### 3.3 嵌入图表到报表

如果你的报表需要包含图表（比如柱状图或饼图），可以用 QChart 渲染到 QPixmap，然后 drawPixmap 嵌入到报表中。

```cpp
// QChart 渲染为 QPixmap
auto *chart = new QChart();
// ... 配置 chart ...
auto *view = new QChartView(chart);
view->resize(800, 400);
QPixmap chartPixmap = view->grab();

// 嵌入到报表
painter->drawPixmap(targetRect, chartPixmap, chartPixmap.rect());
```

`QChartView::grab()` 返回一个 QPixmap，包含图表的完整渲染结果。这个 pixmap 可以像普通图片一样嵌入到 QPainter 的输出中。

## 4. 踩坑预防

第一个坑是打印分辨率差异。QPrinter 的默认分辨率因设备而异——PDF 输出通常是 72 DPI，真实打印机可能是 300 或 600 DPI。用 `QPrinter::Point` 而不是 `QPrinter::DevicePixel` 作为坐标单位，因为点（point）是设备无关的（1 点 = 1/72 英寸）。这样你的报表在不同分辨率设备上的物理尺寸是一致的。

第二个坑是字体度量差异。`QFontMetrics` 在屏幕和打印机上的结果可能不同（因为渲染引擎不同）。打印时应该用 `QFontMetrics(painter.font())` 而不是 `QFontMetrics(screenFont)` 来计算文本尺寸，确保布局在打印输出上是正确的。

## 5. 练习项目

练习项目：月度销售报表生成器。从 SQLite 数据库读取当月销售数据，生成 A4 尺寸的 PDF 报表。包含：公司名称和日期的页眉、分栏表格（产品名、数量、单价、小计）、每页底部的小计汇总行、最后一页的总计行和饼图。

完成标准：数据超过一页时正确分页、每页小计正确、总计和饼图在最后一页、PDF 输出可正常打开。

## 6. 官方文档参考链接

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印设备配置，包含页面尺寸、分辨率和输出格式

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 富文本文档，支持精确排版和分页

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。报表页面模型、分页逻辑、文本排版、图表嵌入——搞定了这些，你就能用 Qt 生成专业的多页报表了。
