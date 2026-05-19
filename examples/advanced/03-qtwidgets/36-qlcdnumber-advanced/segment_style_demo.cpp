/// @file    segment_style_demo.cpp
/// @brief   SegmentStyleDemo 类实现——QLCDNumber SegmentStyle、digitCount、setMode 演示。
///
/// 对应教程：进阶层 03-QtWidgets/36-QLCDNumber 进阶。

#include "segment_style_demo.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

SegmentStyleDemo::SegmentStyleDemo(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(createStyleSection());
    mainLayout->addWidget(createOverflowSection());
    mainLayout->addWidget(createModeSection());
    mainLayout->addStretch();

    setWindowTitle(QStringLiteral("QLCDNumber Advanced Demo"));
    resize(600, 600);
}

// ─────────────────────────────────────────────────────────────────────────────
// SegmentStyle 三种风格对比
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SegmentStyleDemo::createStyleSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(QStringLiteral("1. SegmentStyle 三种风格对比"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // 值输入——三个 LCD 共享同一个值
    auto* inputRow = new QHBoxLayout;
    auto* inputLabel = new QLabel(QStringLiteral("显示值:"));
    m_styleValueSpin = new QSpinBox;
    m_styleValueSpin->setRange(0, 999999);
    m_styleValueSpin->setValue(1234);
    inputRow->addWidget(inputLabel);
    inputRow->addWidget(m_styleValueSpin, 1);

    // 三个 LCD 并排展示三种 SegmentStyle
    auto* lcdRow = new QHBoxLayout;

    // Outline（默认）：只画轮廓边框
    auto* outlineCol = new QVBoxLayout;
    auto* outlineTitle = new QLabel(QStringLiteral("Outline"));
    outlineTitle->setAlignment(Qt::AlignCenter);
    m_outlineLcd = new QLCDNumber(6);
    m_outlineLcd->setSegmentStyle(QLCDNumber::Outline);
    m_outlineLcd->setFixedHeight(60);
    outlineCol->addWidget(outlineTitle);
    outlineCol->addWidget(m_outlineLcd);

    // Filled：填充前景色，工业仪表盘最常用
    auto* filledCol = new QVBoxLayout;
    auto* filledTitle = new QLabel(QStringLiteral("Filled"));
    filledTitle->setAlignment(Qt::AlignCenter);
    m_filledLcd = new QLCDNumber(6);
    m_filledLcd->setSegmentStyle(QLCDNumber::Filled);
    m_filledLcd->setStyleSheet(
        QStringLiteral("background-color: #1a1a2e; color: #00ff88;"));
    m_filledLcd->setFixedHeight(60);
    filledCol->addWidget(filledTitle);
    filledCol->addWidget(m_filledLcd);

    // Flat：无轮廓，扁平简洁
    auto* flatCol = new QVBoxLayout;
    auto* flatTitle = new QLabel(QStringLiteral("Flat"));
    flatTitle->setAlignment(Qt::AlignCenter);
    m_flatLcd = new QLCDNumber(6);
    m_flatLcd->setSegmentStyle(QLCDNumber::Flat);
    m_flatLcd->setStyleSheet(
        QStringLiteral("background-color: #f5f5f5; color: #333333;"));
    m_flatLcd->setFixedHeight(60);
    flatCol->addWidget(flatTitle);
    flatCol->addWidget(m_flatLcd);

    lcdRow->addLayout(outlineCol);
    lcdRow->addLayout(filledCol);
    lcdRow->addLayout(flatCol);

    layout->addWidget(sectionTitle);
    layout->addLayout(inputRow);
    layout->addLayout(lcdRow);

    // 值变化时更新三个 LCD
    connect(m_styleValueSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SegmentStyleDemo::updateStyleDisplay);

    // 初始填充
    updateStyleDisplay();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// digitCount 溢出行为
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SegmentStyleDemo::createOverflowSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(QStringLiteral("2. digitCount 溢出行为"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // digitCount 调节
    auto* digitRow = new QHBoxLayout;
    auto* digitLabel = new QLabel(QStringLiteral("digitCount:"));
    m_digitCountSpin = new QSpinBox;
    m_digitCountSpin->setRange(1, 16);
    m_digitCountSpin->setValue(4);
    digitRow->addWidget(digitLabel);
    digitRow->addWidget(m_digitCountSpin);

    // 测试值输入（浮点数，可以观察小数点占位行为）
    auto* valueRow = new QHBoxLayout;
    auto* valueLabel = new QLabel(QStringLiteral("显示值（double）:"));
    m_overflowValueSpin = new QDoubleSpinBox;
    m_overflowValueSpin->setRange(-99999.99, 99999.99);
    m_overflowValueSpin->setDecimals(2);
    m_overflowValueSpin->setValue(12.34);
    valueRow->addWidget(valueLabel);
    valueRow->addWidget(m_overflowValueSpin, 1);

    // LCD 显示
    m_overflowLcd = new QLCDNumber(4);
    m_overflowLcd->setSegmentStyle(QLCDNumber::Filled);
    m_overflowLcd->setStyleSheet(
        QStringLiteral("background-color: #1a1a2e; color: #00ff88;"));
    m_overflowLcd->setFixedHeight(50);

    // 溢出状态提示
    m_overflowInfo = new QLabel;
    m_overflowInfo->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(digitRow);
    layout->addLayout(valueRow);
    layout->addWidget(m_overflowLcd);
    layout->addWidget(m_overflowInfo);

    // 任何一个参数变化都重新检测溢出
    connect(m_digitCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SegmentStyleDemo::checkOverflow);
    connect(m_overflowValueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &SegmentStyleDemo::checkOverflow);

    // 初始检测
    checkOverflow();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// setMode 进制切换——内部存储值不变，只改变显示格式
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SegmentStyleDemo::createModeSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("3. setMode 进制切换（内部值不变，只改显示格式）"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    sectionTitle->setWordWrap(true);

    // 值输入
    auto* valueRow = new QHBoxLayout;
    auto* valueLabel = new QLabel(QStringLiteral("整数值:"));
    m_modeValueSpin = new QSpinBox;
    m_modeValueSpin->setRange(0, 65535);
    m_modeValueSpin->setValue(255);
    valueRow->addWidget(valueLabel);
    valueRow->addWidget(m_modeValueSpin, 1);

    // 进制选择按钮组
    auto* modeRow = new QHBoxLayout;
    auto* modeLabel = new QLabel(QStringLiteral("进制:"));
    m_modeCombo = new QComboBox;
    m_modeCombo->addItem(QStringLiteral("Dec（十进制）"), static_cast<int>(QLCDNumber::Dec));
    m_modeCombo->addItem(QStringLiteral("Hex（十六进制）"), static_cast<int>(QLCDNumber::Hex));
    m_modeCombo->addItem(QStringLiteral("Oct（八进制）"), static_cast<int>(QLCDNumber::Oct));
    m_modeCombo->addItem(QStringLiteral("Bin（二进制）"), static_cast<int>(QLCDNumber::Bin));
    modeRow->addWidget(modeLabel);
    modeRow->addWidget(m_modeCombo, 1);

    // LCD 显示——根据进制动态调整 digitCount
    m_modeLcd = new QLCDNumber(5);
    m_modeLcd->setSegmentStyle(QLCDNumber::Filled);
    m_modeLcd->setStyleSheet(
        QStringLiteral("background-color: #1a1a2e; color: #00ff88;"));
    m_modeLcd->setFixedHeight(50);

    // 信息标签——显示内部 value() 和当前 mode
    m_modeInfo = new QLabel;
    m_modeInfo->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(valueRow);
    layout->addLayout(modeRow);
    layout->addWidget(m_modeLcd);
    layout->addWidget(m_modeInfo);

    // 值或进制变化时更新显示
    connect(m_modeValueSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SegmentStyleDemo::switchMode);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &SegmentStyleDemo::switchMode);

    // 初始显示
    switchMode();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数实现
// ─────────────────────────────────────────────────────────────────────────────

void SegmentStyleDemo::updateStyleDisplay()
{
    const int value = m_styleValueSpin->value();
    m_outlineLcd->display(value);
    m_filledLcd->display(value);
    m_flatLcd->display(value);
}

void SegmentStyleDemo::checkOverflow()
{
    // 先更新 digitCount，再 display
    const int digits = m_digitCountSpin->value();
    const double value = m_overflowValueSpin->value();

    m_overflowLcd->setDigitCount(digits);
    m_overflowLcd->display(value);

    // 计算实际需要的槽位数：小数点在默认 smallDecimalPoint==false 时独立占位
    const QString text = QString::number(value, 'f', m_overflowValueSpin->decimals());
    const int needed = text.length();

    // 去掉前导零的特殊情况——QLCDNumber 内部处理了，这里仅作粗略提示
    if (needed > digits) {
        m_overflowInfo->setText(
            QStringLiteral("提示：文本 \"%1\" 需要 %2 个槽位，但 digitCount 为 %3。"
                           "超出范围会触发 overflow 信号，LCD 显示全横线。"
                           "注意：默认 smallDecimalPoint==false 时小数点独立占位。")
                .arg(text)
                .arg(needed)
                .arg(digits));
    } else {
        m_overflowInfo->setText(
            QStringLiteral("正常：文本 \"%1\" 占 %2 个槽位，digitCount 为 %3，不会溢出。")
                .arg(text)
                .arg(needed)
                .arg(digits));
    }
}

void SegmentStyleDemo::switchMode()
{
    const int value = m_modeValueSpin->value();
    const QLCDNumber::Mode mode =
        static_cast<QLCDNumber::Mode>(m_modeCombo->currentData().toInt());

    // 根据进制动态调整 digitCount，避免溢出
    // 关键：先设 digitCount 再 display，否则切换到二进制时容易溢出
    int requiredDigits = 0;
    QString modeName;

    switch (mode) {
    case QLCDNumber::Dec:
        // 十进制 0-65535 最多 5 位
        requiredDigits = 5;
        modeName = QStringLiteral("Dec");
        break;
    case QLCDNumber::Hex:
        // 十六进制 0xFFFF 最多 4 位
        requiredDigits = 4;
        modeName = QStringLiteral("Hex");
        break;
    case QLCDNumber::Oct:
        // 八进制 0177777 最多 6 位
        requiredDigits = 6;
        modeName = QStringLiteral("Oct");
        break;
    case QLCDNumber::Bin:
        // 二进制 0-65535 最多 16 位
        requiredDigits = 16;
        modeName = QStringLiteral("Bin");
        break;
    }

    // 先设置 digitCount 确保不溢出，再切换 mode，最后 display
    m_modeLcd->setDigitCount(requiredDigits);
    m_modeLcd->setMode(mode);
    m_modeLcd->display(value);

    // 验证 value() 不变——setMode 只影响显示，不影响内部存储
    m_modeInfo->setText(
        QStringLiteral("mode: %1 | digitCount: %2 | value(): %3（内部存储值不变）")
            .arg(modeName)
            .arg(requiredDigits)
            .arg(m_modeLcd->value()));
}
