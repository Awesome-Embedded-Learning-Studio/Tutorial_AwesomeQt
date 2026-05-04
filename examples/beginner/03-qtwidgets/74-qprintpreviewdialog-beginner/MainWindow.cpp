// QtWidgets 入门示例 74: QPrintPreviewDialog 打印预览
// 演示：paintRequested 信号中执行绘制
//       翻页与缩放操作（内置）
//       自定义预览工具栏（添加页面设置快捷按钮）
//       QPageSetupDialog 页面参数配置

#include "MainWindow.h"

#include <QApplication>
#include <QFileDialog>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QLabel>
#include <QMarginsF>
#include <QPageLayout>
#include <QPageSetupDialog>
#include <QPageSize>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 打印预览 + 页面设置 + 打印 三合一演示
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(
        "QPrintPreviewDialog 打印预览演示");
    resize(700, 550);

    auto *central = new QWidget;
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // ---- 当前页面配置信息 ----
    m_infoLabel = new QLabel(
        "页面: A4 纵向 | 边距: 20mm 四边");
    mainLayout->addWidget(m_infoLabel);

    // ---- 文本编辑区域 ----
    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText(
        "在此输入要打印的内容...");
    fillSampleText();
    mainLayout->addWidget(m_textEdit);

    // ---- 按钮区域 ----
    auto *btnLayout = new QHBoxLayout;
    auto *pageSetupBtn = new QPushButton("页面设置");
    auto *previewBtn = new QPushButton("打印预览");
    auto *printBtn = new QPushButton("直接打印");

    for (auto *btn : {pageSetupBtn, previewBtn, printBtn}) {
        btn->setMinimumHeight(36);
    }

    btnLayout->addWidget(pageSetupBtn);
    btnLayout->addWidget(previewBtn);
    btnLayout->addWidget(printBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // ---- 信号连接 ----
    connect(pageSetupBtn, &QPushButton::clicked,
            this, &MainWindow::onPageSetup);
    connect(previewBtn, &QPushButton::clicked,
            this, &MainWindow::onPrintPreview);
    connect(printBtn, &QPushButton::clicked,
            this, &MainWindow::onDirectPrint);
}

// ====================================================================
// 页面设置: QPageSetupDialog
// ====================================================================
void MainWindow::onPageSetup()
{
    QPrinter printer;
    // 用当前显示的配置初始化
    printer.setPageSize(QPageSize(m_pageSize));
    printer.setPageOrientation(m_orientation);
    printer.setPageMargins(
        m_margins, QPageLayout::Millimeter);

    QPageSetupDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 读取用户配置并保存
        m_pageSize = printer.pageLayout().pageSize().id();
        m_orientation = printer.pageLayout().orientation();
        m_margins = printer.pageLayout().margins(
            QPageLayout::Millimeter);

        updateInfoLabel();
        statusBar()->showMessage("页面设置已更新");
    }
}

// ====================================================================
// 打印预览: QPrintPreviewDialog + 自定义工具栏
// ====================================================================
void MainWindow::onPrintPreview()
{
    QPrinter printer;
    printer.setPageSize(QPageSize(m_pageSize));
    printer.setPageOrientation(m_orientation);
    printer.setPageMargins(
        m_margins, QPageLayout::Millimeter);

    QPrintPreviewDialog dialog(&printer, this);
    dialog.setMinimumSize(900, 700);
    dialog.setWindowTitle("打印预览");

    // 尝试在工具栏上添加"页面设置"快捷按钮
    auto toolBars = dialog.findChildren<QToolBar*>();
    if (!toolBars.isEmpty()) {
        QToolBar *toolbar = toolBars.first();
        toolbar->addSeparator();

        auto *setupAction =
            toolbar->addAction("页面设置");
        connect(setupAction, &QAction::triggered,
                this, [&printer, this]() {
                    // 在预览中弹出页面设置
                    QPageSetupDialog pageSetup(
                        &printer, this);
                    if (pageSetup.exec()
                        == QDialog::Accepted) {
                        // 更新本地配置
                        m_pageSize = printer.pageLayout().pageSize().id();
                        m_orientation =
                            printer.pageLayout().orientation();
                        m_margins =
                            printer.pageLayout().margins(
                                QPageLayout::Millimeter);
                        updateInfoLabel();
                        // QPrintPreviewDialog 会自动
                        // 检测到 QPrinter 参数变化
                        // 并重新发射 paintRequested
                    }
                });
    }

    // 核心: paintRequested -> paintDocument
    connect(&dialog, &QPrintPreviewDialog::paintRequested,
            this, [this](QPrinter *p) {
                paintDocument(p);
            });

    dialog.exec();

    statusBar()->showMessage("预览已关闭");
}

// ====================================================================
// 直接打印: QPrintDialog
// ====================================================================
void MainWindow::onDirectPrint()
{
    QPrinter printer;
    printer.setPageSize(QPageSize(m_pageSize));
    printer.setPageOrientation(m_orientation);
    printer.setPageMargins(
        m_margins, QPageLayout::Millimeter);

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle("打印文档");

    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage("用户取消了打印");
        return;
    }

    paintDocument(&printer);
    statusBar()->showMessage(
        QString("已发送到: %1")
            .arg(printer.printerName()));
}

// ====================================================================
// 核心绘制函数——预览和打印共用
// ====================================================================
void MainWindow::paintDocument(QPrinter *printer)
{
    QPainter painter(printer);
    if (!painter.isActive()) {
        statusBar()->showMessage(
            "错误: 无法启动绘制");
        return;
    }

    QRectF page = printer->pageRect(QPrinter::Point);
    const qreal left = 30;
    const qreal topStart = 30;
    const qreal bottomMargin = 35;
    const qreal usableWidth = page.width() - left * 2;

    // ---- 字体配置 ----
    QFont titleFont("sans-serif", 16, QFont::Bold);
    QFont subtitleFont("sans-serif", 10);
    QFont bodyFont("sans-serif", 11);

    const QFontMetricsF titleFm(titleFont);
    const QFontMetricsF bodyFm(bodyFont);
    const qreal bodyLineH = bodyFm.lineSpacing();

    qreal y = topStart;

    // ---- 页眉: 标题 ----
    painter.setFont(titleFont);
    painter.setPen(Qt::black);
    painter.drawText(
        QRectF(left, y, usableWidth, titleFm.height()),
        Qt::AlignLeft | Qt::TextSingleLine,
        "QPrintPreviewDialog 演示文档");
    y += titleFm.height() + 5;

    // ---- 页眉: 日期 ----
    painter.setFont(subtitleFont);
    painter.setPen(Qt::darkGray);
    QString dateStr =
        QDate::currentDate().toString("yyyy-MM-dd");
    painter.drawText(
        QRectF(left, y, usableWidth,
               QFontMetricsF(subtitleFont).height()),
        Qt::AlignLeft | Qt::TextSingleLine,
        QString("生成日期: %1").arg(dateStr));
    y += QFontMetricsF(subtitleFont).height() + 8;

    // ---- 分隔线 ----
    painter.setPen(QPen(Qt::darkGray, 1));
    painter.drawLine(
        static_cast<int>(left), static_cast<int>(y),
        static_cast<int>(page.width() - left),
        static_cast<int>(y));
    y += 12;

    // ---- 正文内容 ----
    painter.setFont(bodyFont);
    painter.setPen(Qt::black);

    QStringList lines =
        m_textEdit->toPlainText().split('\n');

    int pageNum = 1;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];

        // 空行
        if (line.isEmpty()) {
            y += bodyLineH * 0.5;
            // 空行也可能触发换页
            if (y > page.height() - bottomMargin) {
                drawPageFooter(
                    &painter, pageNum, page);
                printer->newPage();
                pageNum++;
                y = topStart;
            }
            continue;
        }

        // 计算文本所需高度
        QRectF bounding = bodyFm.boundingRect(
            QRectF(left, y, usableWidth, 0),
            Qt::TextWordWrap | Qt::AlignLeft, line);

        // 换页判断
        if (y + bounding.height()
            > page.height() - bottomMargin) {
            drawPageFooter(&painter, pageNum, page);
            printer->newPage();
            pageNum++;
            y = topStart;
        }

        // 绘制文本
        painter.drawText(
            QRectF(left, y, usableWidth,
                   bounding.height()),
            Qt::TextWordWrap | Qt::AlignLeft, line);

        y += bounding.height() + bodyLineH * 0.15;
    }

    // 最后一页的页脚
    drawPageFooter(&painter, pageNum, page);
}

// ====================================================================
// 绘制页脚: 页码居中
// ====================================================================
void MainWindow::drawPageFooter(QPainter *painter, int pageNum,
                                const QRectF &pageRect)
{
    QFont footerFont("sans-serif", 9);
    const QFontMetricsF fm(footerFont);
    QString text = QString("- %1 -").arg(pageNum);
    qreal tw = fm.horizontalAdvance(text);

    painter->setFont(footerFont);
    painter->setPen(Qt::gray);
    painter->drawText(
        QRectF(
            (pageRect.width() - tw) / 2,
            pageRect.height() - 30,
            tw, 15),
        Qt::AlignCenter, text);

    // 恢复画笔
    painter->setPen(Qt::black);
}

// ====================================================================
// 更新页面配置信息标签
// ====================================================================
void MainWindow::updateInfoLabel()
{
    QString sizeStr;
    switch (m_pageSize) {
        case QPageSize::A4:
            sizeStr = "A4"; break;
        case QPageSize::Letter:
            sizeStr = "Letter"; break;
        case QPageSize::B5:
            sizeStr = "B5"; break;
        default:
            sizeStr = "自定义"; break;
    }

    QString orientStr =
        (m_orientation == QPageLayout::Portrait)
            ? "纵向" : "横向";

    m_infoLabel->setText(
        QString("页面: %1 %2 | "
                "边距: %3/%4/%5/%6 mm")
            .arg(sizeStr, orientStr)
            .arg(m_margins.left())
            .arg(m_margins.top())
            .arg(m_margins.right())
            .arg(m_margins.bottom()));
}

// ====================================================================
// 填充示例文本
// ====================================================================
void MainWindow::fillSampleText()
{
    m_textEdit->setPlainText(
        "QPrintPreviewDialog 打印预览演示\n"
        "================================\n\n"
        "本文档用于演示 QPrintPreviewDialog 的\n"
        "完整预览功能。\n\n"
        "第一部分：paintRequested 信号\n"
        "QPrintPreviewDialog 通过 paintRequested\n"
        "信号将绘制逻辑委托给调用方。每当对话框\n"
        "需要刷新预览时（初次打开、参数变更等），\n"
        "它都会发射这个信号。你的槽函数需要绘制\n"
        "所有页面的内容——对话框会自动缓存并管理\n"
        "页面展示。\n\n"
        "第二部分：翻页与缩放\n"
        "预览对话框内置了完整的翻页（上一页、\n"
        "下一页、跳转指定页码）和缩放（放大、\n"
        "缩小、适应宽度、适应页面）功能。\n"
        "这些操作不需要你写任何额外代码。\n\n"
        "第三部分：自定义工具栏\n"
        "通过 findChildren<QToolBar*>() 获取\n"
        "预览对话框的工具栏，在上面添加自定义\n"
        "QAction。本示例添加了\"页面设置\"快捷\n"
        "按钮，让用户在预览过程中直接调整页面参数。\n\n"
        "第四部分：QPageSetupDialog\n"
        "QPageSetupDialog 提供纸张尺寸、方向、\n"
        "页边距的配置界面。修改参数后，\n"
        "QPrintPreviewDialog 会自动检测到\n"
        "QPrinter 参数变化并刷新预览。\n\n"
        "第五部分：测试内容（验证分页）\n"
        "下面是额外的填充文本，确保文档有足够\n"
        "多的内容产生多页效果。\n\n"
        "打印预览是桌面应用打印流程中不可缺少的\n"
        "环节。用户可以在预览中确认效果后\n"
        "再发送到打印机，避免纸张浪费。\n\n"
        "QPrintPreviewDialog 的预览渲染不依赖\n"
        "物理打印机，在没有打印机的环境下\n"
        "也能正常工作。\n\n"
        "预览支持单页、双页并排、四页网格等\n"
        "多种布局模式。用户可以在工具栏上\n"
        "切换不同的预览模式。\n\n"
        "以上就是全部演示内容。请尝试点击\n"
        "\"页面设置\"修改纸张参数，然后点击\n"
        "\"打印预览\"查看效果变化。");
}
