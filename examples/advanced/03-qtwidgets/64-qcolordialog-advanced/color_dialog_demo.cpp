/// @file    color_dialog_demo.cpp
/// @brief   ColorDialogDemo 实现——自定义颜色选择面板。
///
/// 对应教程：进阶层 03-QtWidgets/64-QColorDialog 进阶。

#include "color_dialog_demo.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

/// @brief 预定义颜色列表。
const QVector<QColor> kPresetColors = {
    QColor("#E74C3C"), QColor("#E67E22"), QColor("#F1C40F"),
    QColor("#2ECC71"), QColor("#3498DB"), QColor("#9B59B6"),
    QColor("#1ABC9C"), QColor("#34495E"), QColor("#ECF0F1"),
};

}  // namespace

ColorDialogDemo::ColorDialogDemo(QWidget* parent)
    : QWidget(parent)
    , m_previewLabel(nullptr)
    , m_infoLabel(nullptr)
    , m_currentColor(Qt::black)
{
    setupUI();
}

void ColorDialogDemo::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // 预览色块
    m_previewLabel = new QLabel(this);
    m_previewLabel->setFixedSize(200, 80);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setFrameShape(QFrame::Box);
    mainLayout->addWidget(m_previewLabel, 0, Qt::AlignCenter);

    // 颜色信息
    m_infoLabel = new QLabel(this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_infoLabel);

    // 预定义色板
    auto* gridLayout = new QGridLayout;
    for (int i = 0; i < kPresetColors.size(); ++i) {
        auto* btn = new QPushButton(this);
        btn->setFixedSize(40, 40);
        // @note 使用 QSS 设置按钮背景色，简化演示
        btn->setStyleSheet(
            QStringLiteral("background-color: %1; border: 1px solid #333;")
                .arg(kPresetColors[i].name()));
        btn->setProperty("color", kPresetColors[i]);

        connect(btn, &QPushButton::clicked, this, &ColorDialogDemo::selectPresetColor);

        gridLayout->addWidget(btn, i / 3, i % 3);
    }
    mainLayout->addLayout(gridLayout);

    // 自定义颜色按钮
    auto* customBtn = new QPushButton(tr("Custom Color..."), this);
    connect(customBtn, &QPushButton::clicked, this, &ColorDialogDemo::pickColor);
    mainLayout->addWidget(customBtn);

    updateColorDisplay(m_currentColor);

    setWindowTitle(tr("QColorDialog Advanced Demo"));
    setFixedSize(280, 380);
}

void ColorDialogDemo::pickColor()
{
    // @note QColorDialog::ShowAlphaChannel 允许选择透明度
    const QColor color = QColorDialog::getColor(
        m_currentColor, this, tr("Select Color"),
        QColorDialog::ShowAlphaChannel);

    if (color.isValid()) {
        updateColorDisplay(color);
    }
}

void ColorDialogDemo::selectPresetColor()
{
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) {
        return;
    }

    const QColor color = btn->property("color").value<QColor>();
    updateColorDisplay(color);
}

void ColorDialogDemo::updateColorDisplay(const QColor& color)
{
    m_currentColor = color;

    // 更新预览色块
    m_previewLabel->setStyleSheet(
        QStringLiteral("background-color: %1; border: 2px solid #333;")
            .arg(color.name(QColor::HexArgb)));

    // 显示颜色信息
    // @note 同时展示 hex 和 RGB 格式，方便对比
    m_infoLabel->setText(
        tr("Hex: %1\nRGB: (%2, %3, %4)\nAlpha: %5")
            .arg(color.name(QColor::HexArgb))
            .arg(color.red())
            .arg(color.green())
            .arg(color.blue())
            .arg(color.alpha()));
}
