// QtWidgets 入门示例 31: QSlider 滑动条
// 演示：水平/垂直方向
//       setRange / setValue / setSingleStep / setPageStep
//       valueChanged / sliderMoved / sliderReleased 信号区别
//       QSS 自定义滑块外观（handle / groove）

#include "color_palette_panel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QSlider>
#include <QVBoxLayout>

// ============================================================================
// ColorPalettePanel: 颜色调色板
// 覆盖 QSlider 的核心用法和 QSS 自定义外观
// ============================================================================

/// @brief 为指定颜色通道创建带 QSS 样式的 Slider
static QSlider *createChannelSlider(const QString &colorHex)
{
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 255);
    slider->setValue(128);
    slider->setSingleStep(1);
    slider->setPageStep(16);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(32);

    slider->setStyleSheet(
        QString(
            "QSlider::groove:horizontal {"
            "  height: 6px;"
            "  background: #E0E0E0;"
            "  border-radius: 3px;"
            "}"
            "QSlider::handle:horizontal {"
            "  width: 18px;"
            "  height: 18px;"
            "  margin: -6px 0;"
            "  background: %1;"
            "  border-radius: 9px;"
            "}"
            "QSlider::handle:horizontal:hover {"
            "  background: %1;"
            "  border: 2px solid #333;"
            "}"
            "QSlider::sub-page:horizontal {"
            "  background: %1;"
            "  border-radius: 3px;"
            "}"
            "QSlider::tick-mark {"
            "  color: #BBB;"
            "}")
            .arg(colorHex));

    return slider;
}

/// @brief 为通用参数创建灰色风格 Slider
static QSlider *createParamSlider(int minVal, int maxVal, int defaultVal)
{
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(minVal, maxVal);
    slider->setValue(defaultVal);
    slider->setSingleStep(1);
    slider->setPageStep(5);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(
        qMax(1, (maxVal - minVal) / 10));

    slider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "  height: 6px;"
        "  background: #E0E0E0;"
        "  border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "  width: 18px;"
        "  height: 18px;"
        "  margin: -6px 0;"
        "  background: #78909C;"
        "  border-radius: 9px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "  background: #546E7A;"
        "}"
        "QSlider::sub-page:horizontal {"
        "  background: #90A4AE;"
        "  border-radius: 3px;"
        "}");

    return slider;
}

ColorPalettePanel::ColorPalettePanel(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QSlider 综合演示 — 颜色调色板");
    resize(700, 480);
    initUi();
}

/// @brief 初始化界面
void ColorPalettePanel::initUi()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ================================================================
    // 左侧：RGB 通道控制 + 透明度/笔刷
    // ================================================================
    auto *leftWidget = new QWidget();
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setSpacing(10);

    // ---- RGB 通道 ----
    auto *rgbGroup = new QGroupBox("颜色通道 (RGB)");
    auto *rgbForm = new QFormLayout(rgbGroup);
    rgbForm->setSpacing(10);

    m_redSlider = createChannelSlider("#E53935");
    m_redLabel = new QLabel("128");
    m_redLabel->setMinimumWidth(30);
    auto *redRow = new QHBoxLayout();
    redRow->addWidget(m_redSlider, 1);
    redRow->addWidget(m_redLabel);
    rgbForm->addRow("R:", redRow);

    m_greenSlider = createChannelSlider("#43A047");
    m_greenLabel = new QLabel("128");
    m_greenLabel->setMinimumWidth(30);
    auto *greenRow = new QHBoxLayout();
    greenRow->addWidget(m_greenSlider, 1);
    greenRow->addWidget(m_greenLabel);
    rgbForm->addRow("G:", greenRow);

    m_blueSlider = createChannelSlider("#1E88E5");
    m_blueLabel = new QLabel("128");
    m_blueLabel->setMinimumWidth(30);
    auto *blueRow = new QHBoxLayout();
    blueRow->addWidget(m_blueSlider, 1);
    blueRow->addWidget(m_blueLabel);
    rgbForm->addRow("B:", blueRow);

    leftLayout->addWidget(rgbGroup);

    // ---- 参数控制 ----
    auto *paramGroup = new QGroupBox("参数设置");
    auto *paramForm = new QFormLayout(paramGroup);
    paramForm->setSpacing(10);

    m_opacitySlider = createParamSlider(0, 100, 80);
    m_opacityLabel = new QLabel("80%");
    m_opacityLabel->setMinimumWidth(40);
    auto *opacityRow = new QHBoxLayout();
    opacityRow->addWidget(m_opacitySlider, 1);
    opacityRow->addWidget(m_opacityLabel);
    paramForm->addRow("透明度:", opacityRow);

    m_brushSlider = createParamSlider(1, 50, 10);
    m_brushLabel = new QLabel("10 px");
    m_brushLabel->setMinimumWidth(50);
    auto *brushRow = new QHBoxLayout();
    brushRow->addWidget(m_brushSlider, 1);
    brushRow->addWidget(m_brushLabel);
    paramForm->addRow("笔刷:", brushRow);

    leftLayout->addWidget(paramGroup);
    leftLayout->addStretch();

    mainLayout->addWidget(leftWidget, 1);

    // ================================================================
    // 右侧：颜色预览 + 信息
    // ================================================================
    auto *rightWidget = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(12);

    // ---- 颜色预览 ----
    auto *previewGroup = new QGroupBox("颜色预览");
    auto *previewLayout = new QVBoxLayout(previewGroup);

    m_colorPreview = new QLabel();
    m_colorPreview->setMinimumHeight(180);
    m_colorPreview->setAlignment(Qt::AlignCenter);
    m_colorPreview->setStyleSheet(
        "border: 2px solid #CCC;"
        "border-radius: 8px;");
    previewLayout->addWidget(m_colorPreview);

    rightLayout->addWidget(previewGroup);

    // ---- 颜色信息 ----
    auto *infoGroup = new QGroupBox("颜色信息");
    auto *infoLayout = new QVBoxLayout(infoGroup);

    m_infoLabel = new QLabel();
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet(
        "font-family: monospace;"
        "font-size: 13px;"
        "padding: 8px;");
    infoLayout->addWidget(m_infoLabel);

    rightLayout->addWidget(infoGroup);

    // ---- 信号日志 ----
    auto *logGroup = new QGroupBox("信号追踪");
    auto *logLayout = new QVBoxLayout(logGroup);

    m_logLabel = new QLabel();
    m_logLabel->setWordWrap(true);
    m_logLabel->setStyleSheet(
        "font-size: 11px;"
        "color: #666;"
        "padding: 6px;"
        "background: #FAFAFA;"
        "border: 1px solid #EEE;"
        "border-radius: 4px;");
    logLayout->addWidget(m_logLabel);

    rightLayout->addWidget(logGroup, 1);

    mainLayout->addWidget(rightWidget, 1);

    // ================================================================
    // 信号连接
    // ================================================================
    connect(m_redSlider, &QSlider::valueChanged,
            this, &ColorPalettePanel::onColorChanged);
    connect(m_greenSlider, &QSlider::valueChanged,
            this, &ColorPalettePanel::onColorChanged);
    connect(m_blueSlider, &QSlider::valueChanged,
            this, &ColorPalettePanel::onColorChanged);

    connect(m_opacitySlider, &QSlider::valueChanged,
            this, &ColorPalettePanel::onParamChanged);
    connect(m_brushSlider, &QSlider::valueChanged,
            this, &ColorPalettePanel::onParamChanged);

    // 演示 sliderMoved 和 sliderReleased 信号
    connect(m_redSlider, &QSlider::sliderMoved,
            this, [this](int pos) {
                m_logLabel->setText(
                    QString("sliderMoved(R): %1").arg(pos));
            });
    connect(m_redSlider, &QSlider::sliderReleased,
            this, [this]() {
                m_logLabel->setText(
                    QString("sliderReleased(R), 最终值: %1")
                        .arg(m_redSlider->value()));
            });

    // 初始化显示
    onColorChanged();
    onParamChanged();
}

/// @brief RGB 通道值变化时更新预览
void ColorPalettePanel::onColorChanged()
{
    int r = m_redSlider->value();
    int g = m_greenSlider->value();
    int b = m_blueSlider->value();

    m_redLabel->setText(QString::number(r));
    m_greenLabel->setText(QString::number(g));
    m_blueLabel->setText(QString::number(b));

    QColor color(r, g, b);
    double opacity = m_opacitySlider->value() / 100.0;

    // 背景色带透明度效果（用棋盘格模拟）
    QString previewStyle = QString(
        "background-color: %1;"
        "border: 2px solid #CCC;"
        "border-radius: 8px;"
        "font-size: 16px;"
        "font-weight: bold;"
        "color: %2;")
        .arg(color.name(QColor::HexRgb))
        .arg((r * 299 + g * 587 + b * 114) / 1000 > 128
                 ? "#000000"
                 : "#FFFFFF");

    m_colorPreview->setStyleSheet(previewStyle);
    m_colorPreview->setText(color.name(QColor::HexRgb).toUpper());

    m_infoLabel->setText(
        QString("HEX: %1\n"
                "RGB: rgb(%2, %3, %4)\n"
                "透明度: %5%")
            .arg(color.name(QColor::HexRgb).toUpper())
            .arg(r)
            .arg(g)
            .arg(b)
            .arg(static_cast<int>(opacity * 100)));

    Q_UNUSED(opacity);
}

/// @brief 参数变化时更新显示
void ColorPalettePanel::onParamChanged()
{
    int opacity = m_opacitySlider->value();
    int brushSize = m_brushSlider->value();

    m_opacityLabel->setText(QString("%1%").arg(opacity));
    m_brushLabel->setText(QString("%1 px").arg(brushSize));
}
