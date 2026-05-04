#include "ImageExportPanel.h"

// ============================================================================
// ImageExportPanel: 图像导出设置面板
// 覆盖 QSpinBox 和 QDoubleSpinBox 的核心用法
// ============================================================================

ImageExportPanel::ImageExportPanel(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(
        "QSpinBox / QDoubleSpinBox 综合演示 — 图像导出设置");
    resize(680, 480);
    initUi();
}

/// @brief 初始化界面
void ImageExportPanel::initUi()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ================================================================
    // 左侧：设置面板
    // ================================================================
    auto *settingsWidget = new QWidget();
    auto *settingsLayout = new QVBoxLayout(settingsWidget);
    settingsLayout->setSpacing(8);

    // ---- 尺寸设置组 ----
    auto *sizeGroup = new QGroupBox("尺寸设置");
    auto *sizeForm = new QFormLayout(sizeGroup);
    sizeForm->setSpacing(8);

    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(1, 4096);
    m_widthSpin->setSingleStep(10);
    m_widthSpin->setSuffix(" px");
    m_widthSpin->setValue(1920);
    sizeForm->addRow("宽度:", m_widthSpin);

    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(1, 4096);
    m_heightSpin->setSingleStep(10);
    m_heightSpin->setSuffix(" px");
    m_heightSpin->setValue(1080);
    sizeForm->addRow("高度:", m_heightSpin);

    m_lockRatioCheck = new QCheckBox("锁定宽高比");
    m_lockRatioCheck->setChecked(true);
    sizeForm->addRow(m_lockRatioCheck);

    // 记住初始宽高比
    m_aspectRatio = 1920.0 / 1080.0;

    settingsLayout->addWidget(sizeGroup);

    // ---- 导出参数组 ----
    auto *exportGroup = new QGroupBox("导出参数");
    auto *exportForm = new QFormLayout(exportGroup);
    exportForm->setSpacing(8);

    m_dpiSpin = new QSpinBox();
    m_dpiSpin->setRange(72, 1200);
    m_dpiSpin->setSingleStep(72);
    m_dpiSpin->setSuffix(" DPI");
    m_dpiSpin->setValue(300);
    exportForm->addRow("DPI:", m_dpiSpin);

    m_scaleSpin = new QDoubleSpinBox();
    m_scaleSpin->setRange(0.1, 5.0);
    m_scaleSpin->setSingleStep(0.1);
    m_scaleSpin->setDecimals(1);
    m_scaleSpin->setSuffix(" x");
    m_scaleSpin->setValue(1.0);
    exportForm->addRow("缩放:", m_scaleSpin);

    m_qualitySpin = new QSpinBox();
    m_qualitySpin->setRange(1, 100);
    m_qualitySpin->setSingleStep(5);
    m_qualitySpin->setSuffix(" %");
    m_qualitySpin->setValue(85);
    exportForm->addRow("质量:", m_qualitySpin);

    settingsLayout->addWidget(exportGroup);
    settingsLayout->addStretch();

    mainLayout->addWidget(settingsWidget, 1);

    // ================================================================
    // 右侧：摘要与日志
    // ================================================================
    auto *rightWidget = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(12);

    // ---- 摘要标签 ----
    auto *summaryGroup = new QGroupBox("导出设置摘要");
    auto *summaryLayout = new QVBoxLayout(summaryGroup);

    m_summaryLabel = new QLabel();
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setAlignment(Qt::AlignCenter);
    m_summaryLabel->setStyleSheet(
        "background-color: #E3F2FD;"
        "border: 1px solid #90CAF9;"
        "border-radius: 6px;"
        "padding: 16px;"
        "font-size: 14px;"
        "color: #1565C0;");
    summaryLayout->addWidget(m_summaryLabel);

    rightLayout->addWidget(summaryGroup);

    // ---- 操作日志 ----
    auto *logGroup = new QGroupBox("操作日志");
    auto *logLayout = new QVBoxLayout(logGroup);

    m_logEdit = new QPlainTextEdit();
    m_logEdit->setReadOnly(true);
    m_logEdit->setPlaceholderText("数值变化日志将在此显示...");
    logLayout->addWidget(m_logEdit);

    rightLayout->addWidget(logGroup, 1);

    mainLayout->addWidget(rightWidget, 1);

    // ================================================================
    // 信号连接（在所有初始值设置完毕后）
    // ================================================================
    connect(m_widthSpin, &QSpinBox::valueChanged,
            this, &ImageExportPanel::onWidthChanged);
    connect(m_heightSpin, &QSpinBox::valueChanged,
            this, &ImageExportPanel::onHeightChanged);
    connect(m_dpiSpin, &QSpinBox::valueChanged,
            this, &ImageExportPanel::onSettingsChanged);
    connect(m_scaleSpin, &QDoubleSpinBox::valueChanged,
            this, &ImageExportPanel::onSettingsChanged);
    connect(m_qualitySpin, &QSpinBox::valueChanged,
            this, &ImageExportPanel::onSettingsChanged);
    connect(m_lockRatioCheck, &QCheckBox::toggled,
            this, &ImageExportPanel::onLockRatioToggled);

    // 初始化摘要显示
    updateSummary();
}

/// @brief 宽度变化时的联动处理
void ImageExportPanel::onWidthChanged(int width)
{
    if (m_lockRatioCheck->isChecked()) {
        int newHeight = qBound(
            m_heightSpin->minimum(),
            static_cast<int>(width / m_aspectRatio + 0.5),
            m_heightSpin->maximum());

        // blockSignals 防止循环触发
        m_heightSpin->blockSignals(true);
        m_heightSpin->setValue(newHeight);
        m_heightSpin->blockSignals(false);
    }
    appendLog(QString("宽度 → %1 px").arg(width));
    updateSummary();
}

/// @brief 高度变化时的联动处理
void ImageExportPanel::onHeightChanged(int height)
{
    if (m_lockRatioCheck->isChecked()) {
        int newWidth = qBound(
            m_widthSpin->minimum(),
            static_cast<int>(height * m_aspectRatio + 0.5),
            m_widthSpin->maximum());

        m_widthSpin->blockSignals(true);
        m_widthSpin->setValue(newWidth);
        m_widthSpin->blockSignals(false);
    }
    appendLog(QString("高度 → %1 px").arg(height));
    updateSummary();
}

/// @brief 锁定宽高比状态切换
void ImageExportPanel::onLockRatioToggled(bool checked)
{
    if (checked) {
        // 以当前宽度为基准重新计算宽高比
        m_aspectRatio =
            static_cast<double>(m_widthSpin->value())
            / m_heightSpin->value();
        appendLog(QString("锁定宽高比: %1")
            .arg(QString::number(m_aspectRatio, 'f', 3)));
    } else {
        appendLog("解锁宽高比");
    }
    updateSummary();
}

/// @brief 其他设置变化
void ImageExportPanel::onSettingsChanged()
{
    appendLog(QString("DPI: %1 | 缩放: %2x | 质量: %3%")
        .arg(m_dpiSpin->value())
        .arg(m_scaleSpin->value(), 0, 'f', 1)
        .arg(m_qualitySpin->value()));
    updateSummary();
}

/// @brief 更新摘要显示
void ImageExportPanel::updateSummary()
{
    int w = m_widthSpin->value();
    int h = m_heightSpin->value();
    int dpi = m_dpiSpin->value();
    double scale = m_scaleSpin->value();
    int quality = m_qualitySpin->value();

    // 计算实际导出尺寸
    int exportW = qBound(1, static_cast<int>(w * scale + 0.5), 4096);
    int exportH = qBound(1, static_cast<int>(h * scale + 0.5), 4096);

    // 简单模拟文件大小估算（假设 32 位 RGBA）
    // 文件大小 ≈ 宽 * 高 * 4 字节 * 压缩比
    double compressionRatio = quality / 100.0;
    double estimatedMB =
        (exportW * exportH * 4 * compressionRatio) / (1024.0 * 1024.0);

    QString summary = QString(
        "%1 x %2 px → 导出 %3 x %4 px\n"
        "DPI: %5 | 缩放: %6x | 质量: %7%\n"
        "预估文件大小: ~%8 MB\n"
        "宽高比%9锁定")
        .arg(w).arg(h)
        .arg(exportW).arg(exportH)
        .arg(dpi)
        .arg(scale, 0, 'f', 1)
        .arg(quality)
        .arg(estimatedMB, 0, 'f', 1)
        .arg(m_lockRatioCheck->isChecked() ? "" : "未");

    m_summaryLabel->setText(summary);
}

/// @brief 追加一条操作日志
void ImageExportPanel::appendLog(const QString &message)
{
    QString timestamp = QTime::currentTime().toString("HH:mm:ss");
    m_logEdit->appendPlainText("[" + timestamp + "] " + message);
}
