// QtWidgets 入门示例 64: QColorDialog 颜色选择对话框
// 演示：getColor 模态选择
//       ShowAlphaChannel 透明度通道
//       currentColorChanged 实时预览
//       棋盘格可视化透明度效果

#include "mainwindow.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 演示 QColorDialog 的三种用法
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QColorDialog 颜色选择对话框演示");
    resize(600, 500);

    auto *central = new QWidget;
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    // ---- 颜色信息标签 ----
    m_infoLabel = new QLabel("当前颜色: #FFFFFF (255, 255, 255) "
                             "Alpha: 255");
    m_infoLabel->setWordWrap(true);
    mainLayout->addWidget(m_infoLabel);

    // ---- 预览区域 ----
    m_preview = new ColorPreviewWidget;
    mainLayout->addWidget(m_preview, 1);

    // ---- 按钮区域 ----
    auto *btnLayout = new QHBoxLayout;

    auto *pickBtn = new QPushButton("选择颜色");
    auto *alphaBtn = new QPushButton("选择透明色");
    auto *liveBtn = new QPushButton("实时预览");

    for (auto *btn : {pickBtn, alphaBtn, liveBtn}) {
        btn->setMinimumHeight(36);
    }

    btnLayout->addWidget(pickBtn);
    btnLayout->addWidget(alphaBtn);
    btnLayout->addWidget(liveBtn);
    mainLayout->addLayout(btnLayout);

    // ---- 信号连接 ----
    connect(pickBtn, &QPushButton::clicked,
            this, &MainWindow::onPickColor);
    connect(alphaBtn, &QPushButton::clicked,
            this, &MainWindow::onPickAlphaColor);
    connect(liveBtn, &QPushButton::clicked,
            this, &MainWindow::onLivePreview);
}

// ====================================================================
// 模态选择颜色（无 Alpha）
// ====================================================================
void MainWindow::onPickColor()
{
    QColor color = QColorDialog::getColor(
        m_preview->color(),
        this,
        "选择颜色");

    if (color.isValid()) {
        applyColor(color);
    }
}

// ====================================================================
// 模态选择颜色（带 Alpha 通道）
// ====================================================================
void MainWindow::onPickAlphaColor()
{
    QColor initial(255, 0, 0, 128);  // 半透明红
    QColor color = QColorDialog::getColor(
        initial,
        this,
        "选择透明颜色",
        QColorDialog::ShowAlphaChannel);

    if (color.isValid()) {
        applyColor(color);
    }
}

// ====================================================================
// 非模态实时预览
// ====================================================================
void MainWindow::onLivePreview()
{
    if (!m_liveDialog) {
        m_liveDialog = new QColorDialog(this);
        m_liveDialog->setOption(QColorDialog::NoButtons);
        m_liveDialog->setWindowTitle("实时预览 - 选择颜色");
        m_liveDialog->setOption(
            QColorDialog::ShowAlphaChannel);

        connect(m_liveDialog,
                &QColorDialog::currentColorChanged,
                this, &MainWindow::onLiveColorChanged);

        connect(m_liveDialog,
                &QColorDialog::finished,
                this, [this]() {
                    // 对话框关闭后清理指针标记
                });
    }

    m_liveDialog->setCurrentColor(m_preview->color());
    m_liveDialog->show();
    m_liveDialog->raise();
    m_liveDialog->activateWindow();
}

void MainWindow::onLiveColorChanged(const QColor &color)
{
    applyColor(color);
}

// ====================================================================
// 应用颜色到预览区域并更新信息标签
// ====================================================================
void MainWindow::applyColor(const QColor &color)
{
    m_preview->setColor(color);
    m_infoLabel->setText(
        QString("当前颜色: %1 (%2, %3, %4) Alpha: %5")
            .arg(color.name(QColor::HexArgb))
            .arg(color.red())
            .arg(color.green())
            .arg(color.blue())
            .arg(color.alpha()));
}
