// QtWidgets 入门示例 73: QPrintDialog 打印对话框
// 演示：exec() 弹出系统打印对话框
//       获取用户配置（份数、打印范围、单双面）
//       与 QPainter 联动的完整打印流程
//       无打印机时的 PDF 回退方案

#include "MainWindow.h"

#include <QApplication>
#include <QFileDialog>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QLabel>
#include <QMarginsF>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 打印对话框演示 + PDF 回退
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QPrintDialog 打印对话框演示");
    resize(700, 550);

    auto *central = new QWidget;
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // ---- 打印机状态信息 ----
    m_printerInfoLabel = new QLabel;
    refreshPrinterInfo();
    mainLayout->addWidget(m_printerInfoLabel);

    // ---- 文本编辑区域 ----
    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText("在此输入要打印的内容...");
    fillSampleText();
    mainLayout->addWidget(m_textEdit);

    // ---- 按钮区域 ----
    auto *btnLayout = new QHBoxLayout;
    auto *printBtn = new QPushButton("打印");
    auto *pdfBtn = new QPushButton("导出 PDF");
    printBtn->setMinimumHeight(36);
    pdfBtn->setMinimumHeight(36);
    btnLayout->addWidget(printBtn);
    btnLayout->addWidget(pdfBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // ---- 刷新打印机列表按钮 ----
    auto *refreshBtn = new QPushButton("刷新打印机列表");
    btnLayout->addWidget(refreshBtn);

    // ---- 信号连接 ----
    connect(printBtn, &QPushButton::clicked,
            this, &MainWindow::onPrint);
    connect(pdfBtn, &QPushButton::clicked,
            this, &MainWindow::onExportPdf);
    connect(refreshBtn, &QPushButton::clicked,
            this, &MainWindow::refreshPrinterInfo);
}

// ====================================================================
// 刷新打印机信息显示
// ====================================================================
void MainWindow::refreshPrinterInfo()
{
    QList<QPrinterInfo> printers =
        QPrinterInfo::availablePrinters();

    if (printers.isEmpty()) {
        m_printerInfoLabel->setText(
            "当前系统未检测到打印机 "
            "(打印功能将回退为 PDF 导出)");
        m_printerInfoLabel->setStyleSheet(
            "color: orange; font-weight: bold;");
    } else {
        QStringList names;
        for (const auto &info : printers) {
            names << info.printerName();
        }
        m_printerInfoLabel->setText(
            QString("可用打印机: %1").arg(names.join(", ")));
        m_printerInfoLabel->setStyleSheet(
            "color: green;");
    }
}

// ====================================================================
// 打印按钮: 检测打印机 -> 弹出对话框 / PDF 回退
// ====================================================================
void MainWindow::onPrint()
{
    QList<QPrinterInfo> printers =
        QPrinterInfo::availablePrinters();

    if (printers.isEmpty()) {
        // 无打印机 -> PDF 回退
        int ret = QMessageBox::question(
            this, "未检测到打印机",
            "当前系统没有可用的打印机。\n"
            "是否将内容导出为 PDF 文件？",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);

        if (ret == QMessageBox::Yes) {
            onExportPdf();
        }
        return;
    }

    // 有打印机 -> 弹出系统打印对话框
    QPrinter printer;
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(
        QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle("打印文档");
    // 启用页面范围选项
    dialog.setOptions(
        QPrintDialog::PrintPageRange |
        QPrintDialog::PrintToFile);

    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage("用户取消了打印");
        return;
    }

    // 用户确认打印——显示用户选择的参数
    logPrintConfig(&printer);

    // 执行打印
    paintDocument(&printer);

    statusBar()->showMessage(
        QString("已发送到打印机: %1")
            .arg(printer.printerName()));
}

// ====================================================================
// 导出 PDF 按钮
// ====================================================================
void MainWindow::onExportPdf()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出 PDF", "document.pdf",
        "PDF 文件 (*.pdf)");

    if (filePath.isEmpty()) return;

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(
        QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    paintDocument(&printer);

    statusBar()->showMessage(
        QString("PDF 已导出: %1").arg(filePath));
    QMessageBox::information(
        this, "导出成功",
        QString("内容已保存到:\n%1").arg(filePath));
}

// ====================================================================
// 显示用户的打印配置
// ====================================================================
void MainWindow::logPrintConfig(QPrinter *printer)
{
    QStringList info;
    info << QString("打印机: %1").arg(printer->printerName());
    info << QString("份数: %1").arg(printer->copyCount());

    // 打印范围
    switch (printer->printRange()) {
        case QPrinter::AllPages:
            info << "范围: 全部页面"; break;
        case QPrinter::PageRange:
            info << QString("范围: 第 %1 - %2 页")
                        .arg(printer->fromPage())
                        .arg(printer->toPage());
            break;
        case QPrinter::Selection:
            info << "范围: 仅选中内容"; break;
        default:
            break;
    }

    // 单双面
    switch (printer->duplex()) {
        case QPrinter::DuplexNone:
            info << "模式: 单面打印"; break;
        case QPrinter::DuplexLongSide:
            info << "模式: 双面打印（长边翻转）"; break;
        case QPrinter::DuplexShortSide:
            info << "模式: 双面打印（短边翻转）"; break;
        default:
            break;
    }

    statusBar()->showMessage(info.join(" | "));
}

// ====================================================================
// 核心绘制函数——打印和 PDF 共用
// ====================================================================
void MainWindow::paintDocument(QPrinter *printer)
{
    QPainter painter(printer);
    if (!painter.isActive()) {
        statusBar()->showMessage("错误: 无法启动绘制");
        return;
    }

    QRectF page = printer->pageRect(QPrinter::Point);
    const qreal leftMargin = 20;
    const qreal topMargin = 20;
    const qreal bottomMargin = 30;
    const qreal usableWidth = page.width() - leftMargin * 2;

    QFont titleFont("sans-serif", 16, QFont::Bold);
    QFont bodyFont("sans-serif", 11);
    QFont footerFont("sans-serif", 9);

    const QFontMetricsF bodyFm(bodyFont);
    const qreal bodyLineH = bodyFm.lineSpacing();

    // ---- 先计算总页数 ----
    QStringList allLines =
        m_textEdit->toPlainText().split('\n');
    int totalPages = calculatePages(
        allLines, bodyFm, page.height(),
        topMargin, bottomMargin, bodyLineH);

    // ---- 确定打印范围 ----
    int fromPage = 1;
    int toPage = totalPages;

    if (printer->printRange() == QPrinter::PageRange) {
        fromPage = qMax(1, printer->fromPage());
        toPage = qMin(totalPages, printer->toPage());
    }

    // ---- 标题 ----
    qreal y = topMargin;
    painter.setFont(titleFont);
    painter.setPen(Qt::black);
    painter.drawText(
        QRectF(leftMargin, y, usableWidth, 30),
        Qt::AlignLeft | Qt::TextSingleLine,
        "QPrintDialog 演示文档");
    y += 35;

    // 分隔线
    painter.setPen(Qt::darkGray);
    painter.drawLine(
        static_cast<int>(leftMargin), static_cast<int>(y),
        static_cast<int>(page.width() - leftMargin),
        static_cast<int>(y));
    y += 15;

    // ---- 逐行绘制内容 ----
    painter.setFont(bodyFont);
    painter.setPen(Qt::black);

    bool firstPrintedPage = true;

    // 跳过 fromPage 之前的页面内容
    int linesToSkip = 0;
    {
        qreal tempY = y;
        int lineIdx = 0;
        for (int p = 1; p < fromPage && p <= totalPages;
             ++p) {
            qreal pageEnd =
                page.height() - bottomMargin;
            while (tempY + bodyLineH <= pageEnd
                   && lineIdx < allLines.size()) {
                QRectF bounding = bodyFm.boundingRect(
                    QRectF(leftMargin, tempY,
                           usableWidth, 0),
                    Qt::TextWordWrap | Qt::AlignLeft,
                    allLines[lineIdx]);
                tempY += bounding.height()
                         + bodyLineH * 0.2;
                ++lineIdx;
            }
            tempY = topMargin + 35 + 15;
        }
        linesToSkip = lineIdx;
    }

    // 从 fromPage 开始绘制
    int lineIdx = linesToSkip;
    y = topMargin + 35 + 15;  // 标题 + 分隔线之后

    for (int pageNum = fromPage; pageNum <= toPage; ++pageNum) {
        if (!firstPrintedPage) {
            printer->newPage();
        }
        firstPrintedPage = false;

        // 重新设置起始 y（首页含标题，后续页不含）
        if (pageNum > fromPage) {
            y = topMargin;
        }

        qreal pageEnd = page.height() - bottomMargin;

        // 绘制该页的内容
        while (y + bodyLineH <= pageEnd
               && lineIdx < allLines.size()) {
            const QString &line = allLines[lineIdx];

            if (line.isEmpty()) {
                y += bodyLineH * 0.6;
                ++lineIdx;
                continue;
            }

            QRectF bounding = bodyFm.boundingRect(
                QRectF(leftMargin, y, usableWidth, 0),
                Qt::TextWordWrap | Qt::AlignLeft, line);

            painter.drawText(
                QRectF(leftMargin, y, usableWidth,
                       bounding.height()),
                Qt::TextWordWrap | Qt::AlignLeft, line);

            y += bounding.height() + bodyLineH * 0.2;
            ++lineIdx;
        }

        // 页脚: 页码
        drawPageFooter(&painter, pageNum, page.width());
    }
}

// ====================================================================
// 计算总页数
// ====================================================================
int MainWindow::calculatePages(const QStringList &lines,
                               const QFontMetricsF &fm,
                               qreal pageHeight,
                               qreal topMargin,
                               qreal bottomMargin,
                               qreal lineSpacing)
{
    const qreal usableWidth =
        pageHeight > 0 ? 500.0 : 500.0;  // 近似宽度
    qreal usablePageHeight =
        pageHeight - topMargin - bottomMargin - 50;
    qreal y = 0;
    int pages = 1;

    for (const auto &line : lines) {
        if (line.isEmpty()) {
            y += lineSpacing * 0.6;
            continue;
        }

        QRectF bounding = fm.boundingRect(
            QRectF(0, 0, usableWidth, 0),
            Qt::TextWordWrap | Qt::AlignLeft, line);
        y += bounding.height() + lineSpacing * 0.2;

        if (y > usablePageHeight) {
            pages++;
            y = bounding.height() + lineSpacing * 0.2;
        }
    }

    return pages;
}

// ====================================================================
// 绘制页脚
// ====================================================================
void MainWindow::drawPageFooter(QPainter *painter, int pageNum,
                                qreal pageWidth)
{
    QFont footerFont("sans-serif", 9);
    const QFontMetricsF fm(footerFont);
    QString text = QString("- %1 -").arg(pageNum);
    qreal tw = fm.horizontalAdvance(text);

    painter->setFont(footerFont);
    painter->setPen(Qt::gray);
    // 放在页面底部居中
    painter->drawText(
        QRectF((pageWidth - tw) / 2, -25, tw, 15),
        Qt::AlignCenter, text);
    painter->setPen(Qt::black);
}

// ====================================================================
// 填充示例文本
// ====================================================================
void MainWindow::fillSampleText()
{
    m_textEdit->setPlainText(
        "QPrintDialog 打印对话框演示\n"
        "==========================\n\n"
        "本文档用于演示 QPrintDialog 的完整打印流程。\n\n"
        "第一部分：打印对话框基础\n"
        "QPrintDialog::exec() 弹出操作系统原生的打印\n"
        "对话框，用户可以选择打印机、设置份数、指定\n"
        "页面范围。用户的配置直接写入 QPrinter 对象。\n\n"
        "第二部分：用户配置读取\n"
        "通过 copyCount() 获取份数，printRange() 获取\n"
        "打印范围类型，fromPage()/toPage() 获取页面\n"
        "范围，duplex() 获取单双面设置。\n\n"
        "第三部分：与 QPainter 联动\n"
        "打印和 PDF 导出共享同一个绘制函数。绘制时\n"
        "根据 fromPage/toPage 只绘制用户指定的页面\n"
        "范围。\n\n"
        "第四部分：PDF 回退\n"
        "当系统没有可用打印机时，自动回退为 PDF 导出\n"
        "模式。使用 QPrinterInfo::availablePrinters()\n"
        "检测打印机列表。\n\n"
        "第五部分：测试内容（用于验证分页效果）\n"
        "下面是一些填充内容，确保文档足够长以产生\n"
        "多页效果。\n\n"
        "Qt 的打印系统基于 QPagedPaintDevice 抽象，\n"
        "QPrinter 同时支持物理打印机输出和 PDF 文件\n"
        "输出。这种统一抽象使得同一份绘制代码可以\n"
        "在两种输出目标之间无缝切换。\n\n"
        "打印坐标系的单位是磅（point），即 1/72 英寸。\n"
        "QPainter 会自动处理不同打印机 DPI 的缩放，\n"
        "确保绘制内容在不同分辨率的打印机上物理\n"
        "大小一致。\n\n"
        "分页逻辑的核心是跟踪当前 y 坐标，每次绘制\n"
        "一行前检查剩余空间是否足够。不足时调用\n"
        "newPage() 开启新页并重置 y 坐标。\n\n"
        "以上就是全部演示内容。点击打印按钮体验\n"
        "完整的打印流程。");
}
