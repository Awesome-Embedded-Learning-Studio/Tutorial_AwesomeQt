// QtWidgets 入门示例 72: QPrinter 打印机抽象类
// 演示：页面配置 (setPageSize/setOrientation/setPageMargins)
//       QPainter + QPrinter 自定义打印内容
//       QPrintPreviewDialog 打印预览
//       导出 PDF (QPrinter::PdfFormat)

#include "MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QLabel>
#include <QMarginsF>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 模拟简单文档编辑器 + 打印/导出功能
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QPrinter 打印机抽象类演示");
    resize(700, 550);

    auto *central = new QWidget;
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // ---- 页面设置区域 ----
    auto *settingsLayout = new QHBoxLayout;
    settingsLayout->addWidget(new QLabel("纸张尺寸:"));

    m_pageSizeCombo = new QComboBox;
    m_pageSizeCombo->addItem("A4", QVariant::fromValue(
        QPageSize(QPageSize::A4)));
    m_pageSizeCombo->addItem("Letter", QVariant::fromValue(
        QPageSize(QPageSize::Letter)));
    m_pageSizeCombo->addItem("B5", QVariant::fromValue(
        QPageSize(QPageSize::B5)));
    settingsLayout->addWidget(m_pageSizeCombo);

    settingsLayout->addWidget(new QLabel("方向:"));
    m_orientationCombo = new QComboBox;
    m_orientationCombo->addItem("纵向",
        static_cast<int>(QPageLayout::Portrait));
    m_orientationCombo->addItem("横向",
        static_cast<int>(QPageLayout::Landscape));
    settingsLayout->addWidget(m_orientationCombo);
    settingsLayout->addStretch();

    mainLayout->addLayout(settingsLayout);

    // ---- 文本编辑区域 ----
    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText("在此输入要打印的内容...");
    m_textEdit->setPlainText(
        "QPrinter 打印机抽象类演示\n"
        "========================\n\n"
        "第一段：QPrinter 是 Qt 打印系统的核心抽象类。\n"
        "它把物理打印机和 PDF 文件统一抽象成一个\n"
        "QPaintDevice，让我们用 QPainter 绘制打印内容。\n\n"
        "第二段：页面配置包括纸张尺寸（A4、Letter、\n"
        "B5 等）、方向（纵向/横向）和页边距。\n"
        "这些参数必须在 QPainter begin 之前设置。\n\n"
        "第三段：QPrintPreviewDialog 提供打印预览功能。\n"
        "用户可以在预览窗口中翻页、缩放，确认效果后\n"
        "再点击打印。预览和打印共享同一份绘制代码。\n\n"
        "第四段：导出 PDF 只需要把输出格式设为\n"
        "QPrinter::PdfFormat 并指定文件名。生成的 PDF\n"
        "是矢量格式，文字可选中复制，放大不模糊。\n\n"
        "第五段：打印的核心流程分三步——配置 QPrinter\n"
        "参数，创建 QPainter 开始绘制，绘制完成后\n"
        "结束。分页通过 newPage() 实现，需要自行\n"
        "跟踪 y 坐标判断何时换页。\n\n"
        "第六段：QPainter 在 QPrinter 上的坐标系原点\n"
        "在页面左上角，单位为磅（1/72 英寸）。\n"
        "DPI 缩放由 QPainter 自动处理。\n\n"
        "以上就是 QPrinter 的基本用法演示文本。\n"
        "点击下方按钮体验打印预览和 PDF 导出。");
    mainLayout->addWidget(m_textEdit);

    // ---- 按钮区域 ----
    auto *btnLayout = new QHBoxLayout;
    auto *previewBtn = new QPushButton("打印预览");
    auto *pdfBtn = new QPushButton("导出 PDF");
    previewBtn->setMinimumHeight(36);
    pdfBtn->setMinimumHeight(36);
    btnLayout->addWidget(previewBtn);
    btnLayout->addWidget(pdfBtn);
    mainLayout->addLayout(btnLayout);

    // ---- 状态标签 ----
    m_statusLabel = new QLabel("就绪");
    mainLayout->addWidget(m_statusLabel);

    // ---- 信号连接 ----
    connect(previewBtn, &QPushButton::clicked,
            this, &MainWindow::onPrintPreview);
    connect(pdfBtn, &QPushButton::clicked,
            this, &MainWindow::onExportPdf);
}

// ====================================================================
// 配置 QPrinter 公共参数
// ====================================================================
void MainWindow::configurePrinter(QPrinter *printer)
{
    // 从下拉框读取纸张尺寸
    auto pageSize = m_pageSizeCombo->currentData()
                        .value<QPageSize>();
    printer->setPageSize(pageSize);

    // 从下拉框读取方向
    auto orientation = static_cast<QPageLayout::Orientation>(
        m_orientationCombo->currentData().toInt());
    printer->setPageOrientation(orientation);

    // 统一页边距：四边 20mm
    printer->setPageMargins(
        QMarginsF(20, 20, 20, 20),
        QPageLayout::Millimeter);
}

// ====================================================================
// 打印预览
// ====================================================================
void MainWindow::onPrintPreview()
{
    QPrinter printer;
    configurePrinter(&printer);

    QPrintPreviewDialog dialog(&printer, this);
    dialog.setMinimumSize(900, 700);

    // paintRequested: 对话框需要渲染预览时发射
    connect(&dialog, &QPrintPreviewDialog::paintRequested,
            this, [this](QPrinter *p) {
                paintDocument(p);
            });

    dialog.exec();
    m_statusLabel->setText("预览已关闭");
}

// ====================================================================
// 导出 PDF
// ====================================================================
void MainWindow::onExportPdf()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出 PDF", "document.pdf",
        "PDF 文件 (*.pdf)");

    if (filePath.isEmpty()) return;

    QPrinter printer(QPrinter::ScreenResolution);
    configurePrinter(&printer);

    // 设置输出为 PDF 格式
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);

    paintDocument(&printer);

    m_statusLabel->setText(
        QString("PDF 已导出: %1").arg(filePath));
    QMessageBox::information(
        this, "导出成功",
        QString("PDF 文件已保存到:\n%1").arg(filePath));
}

// ====================================================================
// 核心绘制函数——打印和导出 PDF 共用
// ====================================================================
void MainWindow::paintDocument(QPrinter *printer)
{
    QPainter painter(printer);
    if (!painter.isActive()) {
        m_statusLabel->setText("错误: 无法启动绘制");
        return;
    }

    // 获取可用页面区域（单位: 磅）
    QRectF page = printer->pageRect(QPrinter::Point);
    const qreal leftMargin = 20;
    const qreal topMargin = 20;
    const qreal bottomMargin = 30;
    const qreal usableWidth =
        page.width() - leftMargin * 2;

    // 标题字体
    QFont titleFont("sans-serif", 18, QFont::Bold);
    QFont bodyFont("sans-serif", 11);

    const QFontMetricsF titleFm(titleFont);
    const QFontMetricsF bodyFm(bodyFont);
    const qreal titleLineH = titleFm.lineSpacing();
    const qreal bodyLineH = bodyFm.lineSpacing();

    qreal y = topMargin;

    // ---- 绘制页眉分隔线 ----
    painter.setPen(Qt::darkGray);
    painter.drawLine(
        static_cast<int>(leftMargin),
        static_cast<int>(y),
        static_cast<int>(page.width() - leftMargin),
        static_cast<int>(y));
    y += 10;

    // ---- 绘制标题 ----
    painter.setFont(titleFont);
    painter.setPen(Qt::black);
    painter.drawText(
        QRectF(leftMargin, y, usableWidth, titleLineH),
        Qt::AlignLeft | Qt::TextSingleLine,
        "QPrinter 演示文档");
    y += titleLineH + 5;

    // ---- 绘制日期 ----
    painter.setFont(bodyFont);
    painter.setPen(Qt::darkGray);
    QString dateStr =
        QDate::currentDate().toString("yyyy-MM-dd");
    painter.drawText(
        QRectF(leftMargin, y, usableWidth, bodyLineH),
        Qt::AlignLeft | Qt::TextSingleLine,
        QString("生成日期: %1").arg(dateStr));
    y += bodyLineH + 10;

    // ---- 分隔线 ----
    painter.drawLine(
        static_cast<int>(leftMargin),
        static_cast<int>(y),
        static_cast<int>(page.width() - leftMargin),
        static_cast<int>(y));
    y += 15;

    // ---- 正文内容 ----
    painter.setPen(Qt::black);
    QString fullText = m_textEdit->toPlainText();
    QStringList paragraphs = fullText.split('\n');

    int pageNum = 1;

    for (const auto &para : paragraphs) {
        // 空段落画一个空行
        if (para.isEmpty()) {
            y += bodyLineH * 0.6;
            continue;
        }

        // 使用 boundingRect 计算文本需要的矩形高度
        QRectF bounding = bodyFm.boundingRect(
            QRectF(leftMargin, y, usableWidth, 0),
            Qt::TextWordWrap | Qt::AlignLeft,
            para);

        // 检查是否需要换页
        if (y + bounding.height() >
            page.height() - bottomMargin) {
            // 页脚页码
            drawPageNumber(&painter, printer, pageNum);

            printer->newPage();
            pageNum++;
            y = topMargin;
        }

        // 绘制该段落
        painter.drawText(
            QRectF(leftMargin, y, usableWidth,
                   bounding.height()),
            Qt::TextWordWrap | Qt::AlignLeft,
            para);

        y += bounding.height() + bodyLineH * 0.2;
    }

    // 最后一页的页码
    drawPageNumber(&painter, printer, pageNum);
}

// ====================================================================
// 绘制页码（页脚居中）
// ====================================================================
void MainWindow::drawPageNumber(QPainter *painter, QPrinter *printer,
                                int pageNum)
{
    QRectF page = printer->pageRect(QPrinter::Point);
    QString text = QString("- %1 -").arg(pageNum);

    QFont footerFont("sans-serif", 9);
    const QFontMetricsF fm(footerFont);
    qreal textWidth = fm.horizontalAdvance(text);

    painter->setFont(footerFont);
    painter->setPen(Qt::gray);
    painter->drawText(
        QRectF((page.width() - textWidth) / 2,
               page.height() - 25,
               textWidth, 15),
        Qt::AlignCenter, text);

    // 恢复默认画笔
    painter->setPen(Qt::black);
}
