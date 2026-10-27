/// @file    widget.cpp
/// @brief   PrintDialogMainWindow 实现：QPrintDialog 打印范围选择与自定义页边距。
///
/// 对应教程：进阶层 03-QtWidgets 打印对话框高级用法。
/// 核心演示：QPrinter::PrintRange、setFromTo、setFullPage、自定义边距打印。

#include "widget.h"

#include <QFont>
#include <QFontMetrics>
#include <QMainWindow>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

PrintDialogMainWindow::PrintDialogMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_textEdit(nullptr)
    , m_printer(new QPrinter(QPrinter::HighResolution))
{
    setupUI();
    populateSampleText();

    setWindowTitle(tr("QPrintDialog Advanced Demo"));
    resize(700, 500);
}

PrintDialogMainWindow::~PrintDialogMainWindow()
{
    delete m_printer;
}

void PrintDialogMainWindow::setupUI()
{
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    m_textEdit = new QPlainTextEdit;
    m_textEdit->setReadOnly(false);
    layout->addWidget(m_textEdit);

    auto* printButton = new QPushButton(tr("Print..."));
    connect(printButton, &QPushButton::clicked,
            this, &PrintDialogMainWindow::openPrintDialog);
    layout->addWidget(printButton);

    setCentralWidget(centralWidget);
}

void PrintDialogMainWindow::populateSampleText()
{
    // @note 生成约 120 行文本，在 A4 纸上约 3-4 页，方便演示打印范围
    QString sample;
    for (int page = 1; page <= 4; ++page) {
        sample += tr("--- Page %1 Section ---").arg(page) + "\n\n";
        for (int line = 1; line <= 25; ++line) {
            sample += tr("This is line %1 of page %2. "
                         "Print range selection allows printing "
                         "only the pages you need.").arg(line).arg(page);
            sample += "\n";
        }
        sample += "\n";
    }
    m_textEdit->setPlainText(sample);
}

void PrintDialogMainWindow::openPrintDialog()
{
    // @note 设置 PrintRange 选项使对话框启用页码范围选择控件
    m_printer->setPrintRange(QPrinter::AllPages);
    m_printer->setFromTo(1, 9999);
    // @note setFullPage(true) 让我们完全控制边距，包括页眉页脚区域
    m_printer->setFullPage(true);

    QPrintDialog dialog(m_printer, this);
    dialog.setWindowTitle(tr("Print Document"));
    // @note 启用 PrintPageRange 选项，允许用户选择"页码范围"
    dialog.setOption(QPrintDialog::PrintPageRange, true);
    dialog.setMinMax(1, 9999);

    if (dialog.exec() == QDialog::Accepted) {
        performPrint(m_printer);
    }
}

void PrintDialogMainWindow::performPrint(QPrinter* printer)
{
    QPainter painter;
    if (!painter.begin(printer)) {
        return;
    }

    const QRectF pageRect = printer->pageRect(QPrinter::DevicePixel);
    const QString fullText = m_textEdit->toPlainText();

    QFont bodyFont("Sans Serif", 10);
    painter.setFont(bodyFont);
    QFontMetricsF fm(bodyFont, printer);

    // 自定义边距
    const qreal kMarginLeft = 30.0;
    const qreal kMarginRight = 30.0;
    const qreal kMarginTop = 30.0;
    const qreal kMarginBottom = 30.0;

    const qreal bodyTop = pageRect.top() + kMarginTop;
    const qreal bodyBottom = pageRect.bottom() - kMarginBottom;
    const qreal bodyWidth = pageRect.width() - kMarginLeft - kMarginRight;
    const qreal lineSpacing = fm.lineSpacing();

    // @note 计算每页可容纳行数，用 bodyWidth 限制单行可用宽度
    const int linesPerPage = static_cast<int>((bodyBottom - bodyTop) / lineSpacing);
    if (linesPerPage <= 0) {
        painter.end();
        return;
    }

    const QStringList allLines = fullText.split('\n');
    const int totalLines = allLines.size();
    int totalPages = (totalLines + linesPerPage - 1) / linesPerPage;
    if (totalPages == 0) {
        totalPages = 1;
    }

    // @note 读取对话框返回的打印范围
    const QPrinter::PrintRange range = printer->printRange();
    const int fromPage = printer->fromPage();
    const int toPage = printer->toPage();

    int effectiveFrom = 1;
    int effectiveTo = totalPages;

    if (range == QPrinter::PageRange) {
        // @note 用户选择了页码范围，限制输出页面
        effectiveFrom = qMax(1, fromPage);
        effectiveTo = qMin(totalPages, toPage);
    }

    // 逐页绘制，跳过范围外的页面
    int currentLine = 0;
    for (int page = 1; page <= totalPages; ++page) {
        const bool isInRange = (page >= effectiveFrom && page <= effectiveTo);
        const int pageStartLine = (page - 1) * linesPerPage;
        const int pageEndLine = qMin(page * linesPerPage, totalLines);

        if (isInRange) {
            // @note 首页以外的页面需要调用 newPage
            if (page > effectiveFrom) {
                printer->newPage();
            }

            painter.setFont(bodyFont);
            qreal y = bodyTop;
            for (int i = pageStartLine; i < pageEndLine; ++i) {
                painter.drawText(
                    QRectF(pageRect.left() + kMarginLeft, y,
                           bodyWidth, lineSpacing),
                    Qt::AlignLeft | Qt::TextSingleLine,
                    allLines.at(i));
                y += lineSpacing;
            }

            // 页码
            QFont footerFont("Sans Serif", 8);
            painter.setFont(footerFont);
            const QString pageStr = tr("Page %1 / %2").arg(page).arg(totalPages);
            painter.drawText(
                QRectF(pageRect.left(), bodyBottom + 5.0,
                       pageRect.width(), 20.0),
                Qt::AlignCenter, pageStr);
        }

        currentLine = pageEndLine;
    }

    painter.end();
}
