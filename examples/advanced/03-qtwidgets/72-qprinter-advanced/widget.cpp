/// @file    widget.cpp
/// @brief   PrintMainWindow 实现：QPrinter 自定义页眉页脚与手动分页。
///
/// 对应教程：进阶层 03-QtWidgets 打印高级用法。
/// 核心演示：QPainter + QPrinter 逐页绘制，QFontMetrics 分页计算。

#include "widget.h"

#include <QDate>
#include <QFileDialog>
#include <QFont>
#include <QFontMetrics>
#include <QLocale>
#include <QMainWindow>
#include <QPainter>
#include <QPrinter>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

PrintMainWindow::PrintMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_textEdit(nullptr)
{
    setupUI();

    setWindowTitle(tr("QPrinter Advanced Demo"));
    resize(700, 500);
}

void PrintMainWindow::setupUI()
{
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText(
        tr("Type or paste content here, then click 'Print to PDF'..."));
    layout->addWidget(m_textEdit);

    auto* printButton = new QPushButton(tr("Print to PDF"));
    connect(printButton, &QPushButton::clicked, this, &PrintMainWindow::printToPdf);
    layout->addWidget(printButton);

    setCentralWidget(centralWidget);
}

void PrintMainWindow::printToPdf()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Save PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (filePath.isEmpty()) {
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter;
    if (!painter.begin(&printer)) {
        return;
    }

    const QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);
    const QString fullText = m_textEdit->toPlainText();
    const QString headerTitle = tr("QPrinter Advanced Demo");

    QFont bodyFont("Sans Serif", 10);
    QFontMetricsF fm(bodyFont, &printer);

    // @note 页眉和页脚各留出固定高度，中间区域用于正文绘制
    const qreal kHeaderHeight = 55.0;
    const qreal kFooterHeight = 35.0;
    const qreal lineSpacing = fm.lineSpacing();
    const qreal bodyTop = pageRect.top() + kHeaderHeight + 10.0;
    const qreal bodyBottom = pageRect.bottom() - kFooterHeight;
    const qreal bodyHeight = bodyBottom - bodyTop;
    const int linesPerPage = static_cast<int>(bodyHeight / lineSpacing);
    if (linesPerPage <= 0) {
        painter.end();
        return;
    }

    // @note 将纯文本按行分割，按 linesPerPage 分页
    const QStringList allLines = fullText.split('\n');
    const int totalLines = allLines.size();
    int totalPages = (totalLines + linesPerPage - 1) / linesPerPage;
    if (totalPages == 0) {
        totalPages = 1;
    }

    // --- 逐页绘制 ---
    int currentLine = 0;
    for (int page = 1; page <= totalPages; ++page) {
        if (page > 1) {
            printer.newPage();
        }

        // 绘制页眉：标题 + 日期 + 分隔线
        drawHeader(painter, pageRect, headerTitle);

        // 绘制正文内容
        painter.setFont(bodyFont);
        qreal y = bodyTop;
        for (int i = 0; i < linesPerPage && currentLine < totalLines; ++i) {
            painter.drawText(QPointF(pageRect.left() + 20.0, y + fm.ascent()),
                             allLines.at(currentLine));
            y += lineSpacing;
            ++currentLine;
        }

        // 绘制页脚：页码
        drawFooter(painter, pageRect, page, totalPages);
    }

    painter.end();
}

qreal PrintMainWindow::drawHeader(QPainter& painter, const QRectF& pageRect,
                                  const QString& title)
{
    QFont headerFont("Sans Serif", 9);
    headerFont.setBold(true);
    painter.setFont(headerFont);

    const qreal headerY = pageRect.top() + 25.0;

    // 左侧：标题
    painter.drawText(QPointF(pageRect.left() + 20.0, headerY), title);

    // 右侧：当前日期
    const QString dateStr = QLocale().toString(QDate::currentDate(), QLocale::LongFormat);
    QFontMetricsF fm(headerFont, painter.device());
    const qreal dateWidth = fm.horizontalAdvance(dateStr);
    painter.drawText(QPointF(pageRect.right() - dateWidth - 20.0, headerY), dateStr);

    // 分隔线
    const qreal lineY = headerY + 8.0;
    painter.drawLine(QPointF(pageRect.left() + 20.0, lineY),
                     QPointF(pageRect.right() - 20.0, lineY));

    // @note 返回页眉总占用高度：文本 + 间距 + 分隔线下方留白
    return lineY - pageRect.top() + 12.0;
}

qreal PrintMainWindow::drawFooter(QPainter& painter, const QRectF& pageRect,
                                  int pageNumber, int totalPages)
{
    QFont footerFont("Sans Serif", 8);
    painter.setFont(footerFont);

    const QString pageStr = tr("Page %1 / %2").arg(pageNumber).arg(totalPages);
    QFontMetricsF fm(footerFont, painter.device());
    const qreal textWidth = fm.horizontalAdvance(pageStr);

    const qreal footerY = pageRect.bottom() - 15.0;

    // 页码居中
    const qreal centerX = pageRect.left() + (pageRect.width() - textWidth) / 2.0;
    painter.drawText(QPointF(centerX, footerY), pageStr);

    // @note 返回页脚占用高度，供外部计算正文可用区域
    return 35.0;
}
