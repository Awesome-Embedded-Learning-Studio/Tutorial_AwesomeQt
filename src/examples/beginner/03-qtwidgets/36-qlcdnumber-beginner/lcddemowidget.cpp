// QtWidgets 入门示例 36: QLCDNumber 液晶数字显示
// 演示：display(int/double/QString) 三种显示方式
//       setDigitCount 位数控制 + setSmallDecimalPoint 小数点策略
//       setMode 十进制/十六进制/八进制/二进制切换
//       QTimer + QLCDNumber 仪表盘计时器

#include "lcddemowidget.h"

// ============================================================================
// LcdDemoWidget: QLCDNumber 综合演示窗口
// ============================================================================
LcdDemoWidget::LcdDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QLCDNumber 综合演示 — 仪表盘数字显示");
    resize(560, 520);
    initUi();
}

/// @brief 初始化界面
void LcdDemoWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ================================================================
    // 区域 1: 主 LCD 显示器
    // ================================================================
    m_mainLcd = new QLCDNumber;
    m_mainLcd->setDigitCount(8);
    m_mainLcd->setMinimumHeight(80);
    m_mainLcd->setSegmentStyle(QLCDNumber::Flat);
    m_mainLcd->display(0);
    mainLayout->addWidget(m_mainLcd);

    // overflow 监控
    connect(m_mainLcd, &QLCDNumber::overflow, this,
            [this]() {
        m_statusLabel->setText(
            "overflow: 显示位数不足，请增大 digitCount");
        m_statusLabel->setStyleSheet(
            "color: #D32F2F; font-size: 12px;"
            "padding: 8px;"
            "background-color: #FFEBEE;"
            "border: 1px solid #EF9A9A;"
            "border-radius: 4px;");
    });

    // ================================================================
    // 区域 2: 整数滑块 + 进制切换
    // ================================================================
    auto *sliderGroup = new QGroupBox("整数输入 + 进制切换");
    auto *sliderLayout = new QVBoxLayout(sliderGroup);

    // 滑块
    auto *sliderRow = new QHBoxLayout;
    auto *sliderLabel = new QLabel("数值:");
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 9999);
    m_slider->setValue(0);
    m_sliderValueLabel = new QLabel("0");
    m_sliderValueLabel->setMinimumWidth(50);

    sliderRow->addWidget(sliderLabel);
    sliderRow->addWidget(m_slider, 1);
    sliderRow->addWidget(m_sliderValueLabel);
    sliderLayout->addLayout(sliderRow);

    // 进制选择
    auto *modeRow = new QHBoxLayout;
    auto *modeLabel = new QLabel("显示模式:");
    m_modeCombo = new QComboBox;
    m_modeCombo->addItem("十进制 (Dec)", static_cast<int>(QLCDNumber::Dec));
    m_modeCombo->addItem("十六进制 (Hex)", static_cast<int>(QLCDNumber::Hex));
    m_modeCombo->addItem("八进制 (Oct)", static_cast<int>(QLCDNumber::Oct));
    m_modeCombo->addItem("二进制 (Bin)", static_cast<int>(QLCDNumber::Bin));
    modeRow->addWidget(modeLabel);
    modeRow->addWidget(m_modeCombo, 1);
    modeRow->addStretch();
    sliderLayout->addLayout(modeRow);

    mainLayout->addWidget(sliderGroup);

    // 滑块值变化 → 更新 LCD
    connect(m_slider, &QSlider::valueChanged, this,
            [this](int value) {
        m_sliderValueLabel->setText(QString::number(value));
        updateLcdFromSlider(value);
    });

    // 进制切换 → 重新渲染当前值
    connect(m_modeCombo, &QComboBox::currentIndexChanged, this,
            [this]() {
        updateLcdFromSlider(m_slider->value());
    });

    // ================================================================
    // 区域 3: 浮点数输入 + 小数点策略
    // ================================================================
    auto *floatGroup = new QGroupBox("浮点数输入 + setSmallDecimalPoint");
    auto *floatLayout = new QHBoxLayout(floatGroup);

    m_doubleSpin = new QDoubleSpinBox;
    m_doubleSpin->setRange(-999.99, 999.99);
    m_doubleSpin->setDecimals(2);
    m_doubleSpin->setValue(0.0);
    m_doubleSpin->setSingleStep(0.1);

    m_smallDecimalBtn = new QPushButton("SmallDecimalPoint: OFF");
    m_smallDecimalBtn->setCheckable(true);

    floatLayout->addWidget(m_doubleSpin, 1);
    floatLayout->addWidget(m_smallDecimalBtn);

    mainLayout->addWidget(floatGroup);

    // 浮点数值变化 → 更新 LCD
    connect(m_doubleSpin, &QDoubleSpinBox::valueChanged, this,
            [this](double value) {
        if (!m_timerRunning) {
            m_mainLcd->display(value);
            clearOverflowStatus();
        }
    });

    // 小数点策略切换
    connect(m_smallDecimalBtn, &QPushButton::toggled, this,
            [this](bool checked) {
        m_mainLcd->setSmallDecimalPoint(checked);
        m_smallDecimalBtn->setText(
            checked ? "SmallDecimalPoint: ON"
                    : "SmallDecimalPoint: OFF");
        // 刷新当前显示
        if (!m_timerRunning) {
            m_mainLcd->display(m_doubleSpin->value());
        }
    });

    // ================================================================
    // 区域 4: 计时器
    // ================================================================
    auto *timerGroup = new QGroupBox("仪表盘计时器 (QTimer + QLCDNumber)");
    auto *timerLayout = new QHBoxLayout(timerGroup);

    m_timerBtn = new QPushButton("启动计时器");
    m_timerBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 20px; font-size: 13px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:pressed { background-color: #0D47A1; }");

    m_resetBtn = new QPushButton("重置");
    m_resetBtn->setEnabled(false);

    timerLayout->addWidget(m_timerBtn, 1);
    timerLayout->addWidget(m_resetBtn);

    mainLayout->addWidget(timerGroup);

    // 计时器按钮切换
    connect(m_timerBtn, &QPushButton::clicked, this,
            [this]() {
        if (m_timerRunning) {
            stopTimer();
        } else {
            startTimer();
        }
    });

    // 重置按钮
    connect(m_resetBtn, &QPushButton::clicked, this,
            [this]() {
        m_elapsedSeconds = 0;
        m_mainLcd->display(0);
        clearOverflowStatus();
    });

    // QTimer
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this,
            [this]() {
        ++m_elapsedSeconds;
        const int hours = m_elapsedSeconds / 3600;
        const int minutes = (m_elapsedSeconds % 3600) / 60;
        const int seconds = m_elapsedSeconds % 60;
        m_mainLcd->display(
            QString("%1:%2:%3")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0')));
    });

    // ================================================================
    // 状态标签
    // ================================================================
    m_statusLabel = new QLabel("拖动滑块或修改浮点数来测试 QLCDNumber");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "color: #888; font-size: 12px;"
        "padding: 8px;"
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    mainLayout->addWidget(m_statusLabel);
}

/// @brief 根据滑块值和当前进制更新 LCD 显示
void LcdDemoWidget::updateLcdFromSlider(int value)
{
    if (m_timerRunning) {
        return;
    }

    const auto mode = static_cast<QLCDNumber::Mode>(
        m_modeCombo->currentData().toInt());

    // 根据进制调整 digitCount
    switch (mode) {
    case QLCDNumber::Bin:
        m_mainLcd->setDigitCount(16);  // 二进制最多 16 位
        break;
    case QLCDNumber::Oct:
        m_mainLcd->setDigitCount(8);
        break;
    case QLCDNumber::Hex:
        m_mainLcd->setDigitCount(6);
        break;
    case QLCDNumber::Dec:
    default:
        m_mainLcd->setDigitCount(8);
        break;
    }

    m_mainLcd->setMode(mode);
    m_mainLcd->display(value);
    clearOverflowStatus();
}

/// @brief 启动计时器
void LcdDemoWidget::startTimer()
{
    m_timerRunning = true;
    m_timerBtn->setText("停止计时器");
    m_timerBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #D32F2F; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 20px; font-size: 13px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #C62828; }"
        "QPushButton:pressed { background-color: #B71C1C; }");
    m_resetBtn->setEnabled(false);
    m_statusLabel->setText("计时器运行中...");
    m_statusLabel->setStyleSheet(
        "color: #1976D2; font-size: 12px;"
        "padding: 8px;"
        "background-color: #E3F2FD;"
        "border: 1px solid #90CAF9;"
        "border-radius: 4px;");

    // 恢复默认进制和位数用于计时器显示
    m_mainLcd->setMode(QLCDNumber::Dec);
    m_mainLcd->setDigitCount(8);

    m_timer->start();
}

/// @brief 停止计时器
void LcdDemoWidget::stopTimer()
{
    m_timerRunning = false;
    m_timer->stop();
    m_timerBtn->setText("启动计时器");
    m_timerBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 20px; font-size: 13px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:pressed { background-color: #0D47A1; }");
    m_resetBtn->setEnabled(true);
    m_statusLabel->setText(
        QString("计时器已停止，累计 %1 秒").arg(m_elapsedSeconds));
    m_statusLabel->setStyleSheet(
        "color: #388E3C; font-size: 12px;"
        "padding: 8px;"
        "background-color: #E8F5E9;"
        "border: 1px solid #A5D6A7;"
        "border-radius: 4px;");
}

/// @brief 清除 overflow 状态标签
void LcdDemoWidget::clearOverflowStatus()
{
    m_statusLabel->setText("拖动滑块或修改浮点数来测试 QLCDNumber");
    m_statusLabel->setStyleSheet(
        "color: #888; font-size: 12px;"
        "padding: 8px;"
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
}
