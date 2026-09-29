/**
 * QtPrintSupport 打印体系概览示例
 *
 * 本示例演示 QtPrintSupport 模块的核心用法：
 * 1. QPrinter 配置输出目标（打印机 / PDF）
 * 2. QPrintDialog 让用户选择打印机
 * 3. QPrintPreviewDialog 所见即所得的打印预览
 * 4. 统一的 QPainter 渲染管线——打印和 PDF 共用同一份渲染代码
 */

#include <QApplication>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFileInfo>

#include "printexamplewindow.h"

// ============================================================================
// 统一渲染函数：将标题、正文、图片绘制到 QPrinter 上
// 打印、PDF 导出、打印预览三条路径共用此函数
// ============================================================================
static void renderDocument(QPrinter *printer, const QString &title,
                           const QString &body, const QPixmap &image)
{
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);

    // 可打印区域（已扣除页边距）
    QRectF page = printer->pageRect(QPrinter::DevicePixel);
    qreal y = page.top();

    // ---- 标题 ----
    QFont titleFont("Sans", 18, QFont::Bold);
    painter.setFont(titleFont);
    QRectF titleRect(page.left(), y, page.width(), 40);
    painter.drawText(titleRect, Qt::AlignCenter, title);
    y += 50;

    // ---- 分隔线 ----
    QPen linePen(Qt::gray, 1);
    painter.setPen(linePen);
    painter.drawLine(QPointF(page.left(), y), QPointF(page.right(), y));
    y += 10;

    // ---- 正文（自动换行） ----
    QFont bodyFont("Sans", 10);
    painter.setFont(bodyFont);
    painter.setPen(Qt::black);

    QRectF textRect(page.left(), y, page.width(), page.bottom() - y);

    // 如果有图片，正文只占上半部分，下半部分留给图片
    if (!image.isNull()) {
        textRect.setHeight((page.bottom() - y) * 0.55);
    }

    QRectF usedRect;
    painter.drawText(textRect,
                     Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                     body, &usedRect);
    y = usedRect.bottom() + 20;

    // ---- 图片 ----
    if (!image.isNull() && y < page.bottom() - 80) {
        qreal availW = page.width();
        qreal imgW = qMin<qreal>(image.width(), availW);
        qreal imgH = imgW * image.height() / image.width();

        // 如果图片高度超出剩余空间，按比例缩小
        if (y + imgH > page.bottom()) {
            imgH = page.bottom() - y - 10;
            imgW = imgH * image.width() / image.height();
        }
        painter.drawPixmap(
            QRectF(page.left(), y, imgW, imgH), image,
            QRectF(0, 0, image.width(), image.height()));
    }

    // ---- 页脚：页码 ----
    painter.setFont(QFont("Sans", 8));
    painter.setPen(Qt::gray);
    painter.drawText(QRectF(page.left(), page.bottom() - 20,
                            page.width(), 20),
                     Qt::AlignCenter, "第 1 页");

    painter.end();
}

// ============================================================================
// 主窗口：图文报表打印与 PDF 导出工具
// ============================================================================
PrintExampleWindow::PrintExampleWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QtPrintSupport 打印体系示例");
    resize(700, 500);

    // ---- 左侧：内容编辑区 ----
    auto *left_panel = new QWidget(this);
    auto *left_layout = new QVBoxLayout(left_panel);

    // 标题输入
    left_layout->addWidget(new QLabel("报表标题:", this));
    title_edit_ = new QTextEdit(this);
    title_edit_->setMaximumHeight(40);
    title_edit_->setPlaceholderText("输入标题...");
    title_edit_->setText("QtPrintSupport 演示报表");
    left_layout->addWidget(title_edit_);

    // 正文输入
    left_layout->addWidget(new QLabel("正文内容:", this));
    body_edit_ = new QTextEdit(this);
    body_edit_->setPlaceholderText("输入正文内容...");
    body_edit_->setText(
        "这是一份使用 QtPrintSupport 生成的演示报表。本程序展示了 "
        "QPrinter、QPrintDialog、QPrintPreviewDialog 三个核心类的协作方式。"
        "\n\n核心设计原则：渲染逻辑只写一份，通过 QPrinter 的输出格式切换"
        "来实现打印到纸张和导出到PDF两条路径。\n\n"
        "你可以加载一张图片，它会被渲染在正文的下方。"
        "点击右侧的按钮分别测试打印、导出 PDF 和打印预览功能。");
    left_layout->addWidget(body_edit_, 1);

    // 图片加载
    auto *img_layout = new QHBoxLayout();
    img_path_label_ = new QLabel("未加载图片", this);
    img_path_label_->setFrameShape(QFrame::StyledPanel);
    auto *load_img_btn = new QPushButton("选择图片", this);
    img_layout->addWidget(img_path_label_, 1);
    img_layout->addWidget(load_img_btn);
    left_layout->addLayout(img_layout);

    // ---- 右侧：操作面板 ----
    auto *right_panel = new QWidget(this);
    auto *right_layout = new QVBoxLayout(right_panel);

    right_layout->addWidget(
        new QLabel("输出方式:", this));

    auto *print_btn = new QPushButton("打印到打印机", this);
    auto *pdf_btn = new QPushButton("导出为 PDF", this);
    auto *preview_btn = new QPushButton("打印预览", this);

    // 设置按钮最小高度，方便点击
    for (auto *btn : {print_btn, pdf_btn, preview_btn}) {
        btn->setMinimumHeight(40);
        right_layout->addWidget(btn);
    }

    right_layout->addStretch();

    // 状态标签
    status_label_ = new QLabel("就绪", this);
    status_label_->setWordWrap(true);
    right_layout->addWidget(status_label_);

    // ---- 主布局 ----
    auto *central = new QWidget(this);
    auto *main_layout = new QHBoxLayout(central);
    main_layout->addWidget(left_panel, 3);
    main_layout->addWidget(right_panel, 1);
    setCentralWidget(central);

    // ---- 信号槽 ----

    // 选择图片
    connect(load_img_btn, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(
            this, "选择图片", {},
            "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)");
        if (!file.isEmpty()) {
            if (current_image_.load(file)) {
                img_path_label_->setText(file);
                status_label_->setText("已加载图片: "
                                       + QFileInfo(file).fileName());
            } else {
                status_label_->setText("图片加载失败");
            }
        }
    });

    // 打印到打印机
    connect(print_btn, &QPushButton::clicked, this, [this]() {
        QPrinter printer(QPrinter::HighResolution);
        QPrintDialog dialog(&printer, this);
        dialog.setWindowTitle("打印报表");
        if (dialog.exec() == QDialog::Accepted) {
            renderDocument(&printer,
                           title_edit_->toPlainText(),
                           body_edit_->toPlainText(),
                           current_image_);
            status_label_->setText("已发送到打印机");
        }
    });

    // 导出 PDF
    connect(pdf_btn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this, "导出 PDF", "report.pdf",
            "PDF Files (*.pdf)");
        if (!path.isEmpty()) {
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(path);
            printer.setPageSize(QPageSize(QPageSize::A4));
            renderDocument(&printer,
                           title_edit_->toPlainText(),
                           body_edit_->toPlainText(),
                           current_image_);
            status_label_->setText("已导出 PDF: " + path);
        }
    });

    // 打印预览
    connect(preview_btn, &QPushButton::clicked, this, [this]() {
        QPrinter printer(QPrinter::HighResolution);
        printer.setPageSize(QPageSize(QPageSize::A4));
        QPrintPreviewDialog preview(&printer, this);
        preview.setWindowTitle("打印预览");
        preview.resize(800, 600);

        connect(&preview, &QPrintPreviewDialog::paintRequested,
                this, [this](QPrinter *p) {
            renderDocument(p,
                           title_edit_->toPlainText(),
                           body_edit_->toPlainText(),
                           current_image_);
        });

        preview.exec();
        status_label_->setText("预览已关闭");
    });
}
