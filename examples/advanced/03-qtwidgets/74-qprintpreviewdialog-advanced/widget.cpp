/// @file    widget.cpp
/// @brief   PreviewMainWindow 实现：QPrintPreviewDialog 自定义工具栏与水印绘制。
///
/// 对应教程：进阶层 03-QtWidgets 打印预览对话框高级用法。
/// 核心演示：paintRequested 信号、自定义工具栏 Action、条件水印绘制。

#include "widget.h"

#include <QAction>
#include <QFont>
#include <QFontMetrics>
#include <QMainWindow>
#include <QPainter>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTextEdit>
#include <QToolBar>
#include <QTransform>
#include <QVBoxLayout>
#include <QWidget>

PreviewMainWindow::PreviewMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_textEdit(nullptr)
    , m_watermarkEnabled(false)
{
    setupUI();
    populateSampleText();

    setWindowTitle(tr("QPrintPreviewDialog Advanced Demo"));
    resize(700, 500);
}

void PreviewMainWindow::setupUI()
{
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText(
        tr("Content here will be shown in the print preview..."));
    layout->addWidget(m_textEdit);

    auto* previewButton = new QPushButton(tr("Print Preview..."));
    connect(previewButton, &QPushButton::clicked,
            this, &PreviewMainWindow::openPrintPreview);
    layout->addWidget(previewButton);

    setCentralWidget(centralWidget);
}

void PreviewMainWindow::populateSampleText()
{
    // @note 生成约 100 行文本，在 A4 上约 3-4 页，便于预览翻页
    QString sample;
    for (int page = 1; page <= 4; ++page) {
        sample += tr("=== Section %1 ===").arg(page) + "\n\n";
        for (int line = 1; line <= 20; ++line) {
            sample += tr("Line %1 of section %2: "
                         "Use the 'Add Watermark' button in the preview "
                         "toolbar to toggle diagonal DRAFT text.")
                          .arg(line)
                          .arg(page);
            sample += "\n";
        }
        sample += "\n";
    }
    m_textEdit->setPlainText(sample);
}

void PreviewMainWindow::openPrintPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPrintPreviewDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Preview"));
    dialog.resize(1000, 700);

    // @note 在预览对话框的工具栏末尾添加自定义"水印"开关按钮
    // QPrintPreviewDialog 内部创建工具栏后可通过 findChild 获取
    QToolBar* toolBar = dialog.findChild<QToolBar*>();
    if (toolBar) {
        toolBar->addSeparator();
        QAction* watermarkAction = toolBar->addAction(tr("Add Watermark"));
        watermarkAction->setCheckable(true);
        watermarkAction->setChecked(m_watermarkEnabled);

        // @note 切换水印状态并触发预览刷新
        connect(watermarkAction, &QAction::toggled, this, [this](bool checked) {
            m_watermarkEnabled = checked;
        });
    }

    // @note 连接 paintRequested 信号到渲染槽，预览和打印都走同一逻辑
    connect(&dialog, &QPrintPreviewDialog::paintRequested,
            this, &PreviewMainWindow::renderPages);

    dialog.exec();
}

void PreviewMainWindow::renderPages(QPrinter* printer)
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

    const qreal kMargin = 30.0;
    const qreal bodyTop = pageRect.top() + kMargin;
    const qreal bodyBottom = pageRect.bottom() - kMargin;
    const qreal lineSpacing = fm.lineSpacing();
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

    int currentLine = 0;
    for (int page = 1; page <= totalPages; ++page) {
        if (page > 1) {
            printer->newPage();
        }

        // 绘制正文
        painter.setFont(bodyFont);
        qreal y = bodyTop;
        for (int i = 0; i < linesPerPage && currentLine < totalLines; ++i) {
            painter.drawText(QPointF(pageRect.left() + kMargin, y + fm.ascent()),
                             allLines.at(currentLine));
            y += lineSpacing;
            ++currentLine;
        }

        // 条件绘制水印
        if (m_watermarkEnabled) {
            drawWatermark(painter, pageRect);
        }
    }

    painter.end();
}

void PreviewMainWindow::drawWatermark(QPainter& painter, const QRectF& pageRect)
{
    painter.save();

    QFont watermarkFont("Sans Serif", 60);
    watermarkFont.setBold(true);
    painter.setFont(watermarkFont);

    // @note 半透明灰色确保水印不遮挡正文阅读
    painter.setPen(QColor(180, 180, 180, 100));

    // 将坐标原点移到页面中心，旋转 45 度
    painter.translate(pageRect.center());
    painter.rotate(-45.0);

    const QString text = "DRAFT";
    QFontMetricsF fm(watermarkFont, painter.device());
    const qreal textWidth = fm.horizontalAdvance(text);

    // @note 以页面中心为原点绘制，旋转后自然成为对角线效果
    painter.drawText(QPointF(-textWidth / 2.0, 0), text);

    painter.restore();
}
