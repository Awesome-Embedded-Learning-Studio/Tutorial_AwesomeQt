/**
 * QtPdf PDF 渲染基础示例
 *
 * 本示例演示 QtPdf 模块的三个核心组件：
 * - QPdfDocument：加载 PDF 文件，查询页数和元信息
 * - QPdfPageRenderer：将指定页面渲染为 QImage
 * - QPdfView：开箱即用的 PDF 显示控件，支持多页滚动和缩放
 *
 * 工具栏提供：打开文件、上一页/下一页、放大/缩小
 */

#include "pdfviewer.h"

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QPdfDocument>
#include <QPdfPageRenderer>
#include <QPdfView>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

#include <QDebug>

// ========================================
// PDF 查看器主窗口
// ========================================

PdfViewer::PdfViewer(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(QStringLiteral("QtPdf PDF 查看器"));
    resize(900, 700);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupToolbar(mainLayout);
    setupPdfView(mainLayout);

    // 默认状态提示
    m_pageLabel->setText(QStringLiteral("请打开 PDF 文件"));
}

void PdfViewer::setupToolbar(QVBoxLayout *mainLayout)
{
    auto *toolbar = new QToolBar();
    toolbar->setMovable(false);

    // 打开文件按钮
    auto *btnOpen = new QPushButton(QStringLiteral("打开文件"));
    toolbar->addWidget(btnOpen);

    toolbar->addSeparator();

    // 翻页按钮 + 页码显示
    auto *btnPrev = new QPushButton(QStringLiteral("上一页"));
    toolbar->addWidget(btnPrev);

    m_pageLabel = new QLabel(QStringLiteral("0 / 0"));
    m_pageLabel->setMinimumWidth(80);
    m_pageLabel->setAlignment(Qt::AlignCenter);
    toolbar->addWidget(m_pageLabel);

    auto *btnNext = new QPushButton(QStringLiteral("下一页"));
    toolbar->addWidget(btnNext);

    toolbar->addSeparator();

    // 缩放按钮 + 缩放比例显示
    auto *btnZoomOut = new QPushButton(QStringLiteral("缩小"));
    toolbar->addWidget(btnZoomOut);

    m_zoomLabel = new QLabel(QStringLiteral("100%"));
    m_zoomLabel->setMinimumWidth(60);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    toolbar->addWidget(m_zoomLabel);

    auto *btnZoomIn = new QPushButton(QStringLiteral("放大"));
    toolbar->addWidget(btnZoomIn);

    mainLayout->addWidget(toolbar);

    // 信号连接
    connect(btnOpen, &QPushButton::clicked, this, &PdfViewer::openFile);
    connect(btnPrev, &QPushButton::clicked, this, &PdfViewer::prevPage);
    connect(btnNext, &QPushButton::clicked, this, &PdfViewer::nextPage);
    connect(btnZoomIn, &QPushButton::clicked, this, &PdfViewer::zoomIn);
    connect(btnZoomOut, &QPushButton::clicked, this, &PdfViewer::zoomOut);
}

void PdfViewer::setupPdfView(QVBoxLayout *mainLayout)
{
    // PDF 文档对象
    m_pdfDoc = new QPdfDocument(this);

    // 监听文档加载状态变化
    connect(m_pdfDoc, &QPdfDocument::statusChanged, this, [this]() {
        if (m_pdfDoc->status() == QPdfDocument::Status::Ready) {
            m_pageLabel->setText(
                QStringLiteral("1 / %1").arg(m_pdfDoc->pageCount()));
            updateZoomLabel();
        }
    });

    // PDF 视图控件
    m_pdfView = new QPdfView();
    m_pdfView->setPageMode(QPdfView::PageMode::MultiPage);
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);

    mainLayout->addWidget(m_pdfView);
}

void PdfViewer::openFile()
{
    // 弹出文件选择对话框，过滤 PDF 文件
    QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择 PDF 文件"), QString(),
        QStringLiteral("PDF 文件 (*.pdf);;所有文件 (*)"));

    if (path.isEmpty()) {
        return;
    }

    // 关闭之前加载的文档
    if (m_pdfDoc->status() == QPdfDocument::Status::Ready) {
        m_pdfDoc->close();
    }

    // 加载新的 PDF 文件
    m_pdfDoc->load(path);

    if (m_pdfDoc->status() == QPdfDocument::Status::Error) {
        qWarning() << "PDF 加载失败:" << path;
        return;
    }

    // 将文档绑定到视图
    m_pdfView->setDocument(m_pdfDoc);

    // 重置导航状态
    m_currentPage = 0;
    m_zoomFactor = 1.0;
    m_pdfView->setZoomFactor(m_zoomFactor);

    qDebug() << "PDF 加载成功:" << path
             << "页数:" << m_pdfDoc->pageCount();
}

void PdfViewer::prevPage()
{
    if (m_currentPage > 0) {
        m_currentPage--;
        updatePageLabel();
    }
}

void PdfViewer::nextPage()
{
    if (m_currentPage < m_pdfDoc->pageCount() - 1) {
        m_currentPage++;
        updatePageLabel();
    }
}

void PdfViewer::zoomIn()
{
    // 每次放大 25%，上限 500%
    m_zoomFactor = qMin(m_zoomFactor * 1.25, 5.0);
    m_pdfView->setZoomFactor(m_zoomFactor);
    updateZoomLabel();
}

void PdfViewer::zoomOut()
{
    // 每次缩小 25%，下限 25%
    m_zoomFactor = qMax(m_zoomFactor / 1.25, 0.25);
    m_pdfView->setZoomFactor(m_zoomFactor);
    updateZoomLabel();
}

void PdfViewer::updatePageLabel()
{
    m_pageLabel->setText(
        QStringLiteral("%1 / %2")
            .arg(m_currentPage + 1)
            .arg(m_pdfDoc->pageCount()));
}

void PdfViewer::updateZoomLabel()
{
    m_zoomLabel->setText(
        QStringLiteral("%1%").arg(static_cast<int>(m_zoomFactor * 100)));
}
