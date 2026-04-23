#include "simpleimageviewer.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>

SimpleImageViewer::SimpleImageViewer(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("简易图片查看器");
    resize(700, 500);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ---- 工具栏 ----
    auto *toolbar = new QHBoxLayout;

    auto *openBtn = new QPushButton("打开文件", this);
    // 动态生成一个文件夹图标
    openBtn->setIcon(createFolderIcon());
    openBtn->setIconSize(QSize(20, 20));

    auto *fitBtn = new QPushButton("适应窗口", this);
    auto *originalBtn = new QPushButton("原始尺寸", this);

    m_infoLabel = new QLabel("未加载图片", this);
    m_infoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    toolbar->addWidget(openBtn);
    toolbar->addWidget(fitBtn);
    toolbar->addWidget(originalBtn);
    toolbar->addStretch();
    toolbar->addWidget(m_infoLabel);

    mainLayout->addLayout(toolbar);

    // ---- 图片显示区域 ----
    m_imageWidget = new ImageDisplayWidget(this);
    mainLayout->addWidget(m_imageWidget, 1);

    // ---- 连接信号 ----
    connect(openBtn, &QPushButton::clicked,
            this, &SimpleImageViewer::openFile);
    connect(fitBtn, &QPushButton::clicked,
            this, [this]() { m_fitMode = true; m_imageWidget->update(); });
    connect(originalBtn, &QPushButton::clicked,
            this, [this]() { m_fitMode = false; m_imageWidget->update(); });
}

QIcon SimpleImageViewer::createFolderIcon()
{
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    // 文件夹主体
    p.setBrush(QColor(240, 200, 80));
    p.setPen(QPen(QColor(180, 140, 30), 1.5));
    p.drawRoundedRect(2, 6, 20, 16, 2, 2);

    // 文件夹标签页
    p.drawRect(2, 3, 8, 4);

    return QIcon(pixmap);
}

void SimpleImageViewer::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择图片", QString(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.svg)");

    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "加载失败",
                             "无法加载图片: " + filePath);
        return;
    }

    m_imageWidget->setPixmap(pixmap);
    m_fitMode = true;

    QFileInfo info(filePath);
    m_infoLabel->setText(
        QString("%1 | %2x%3 | 适应模式")
            .arg(info.fileName())
            .arg(pixmap.width())
            .arg(pixmap.height()));
}
