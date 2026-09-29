/// @file    slider_size_demo.cpp
/// @brief   SliderSizeDemo 类实现——QScrollBar 手柄大小公式交互式演示。
///
/// 对应教程：进阶层 03-QtWidgets/32-QScrollBar 进阶。

#include "slider_size_demo.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QSpinBox>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 常量定义
// ─────────────────────────────────────────────────────────────────────────────

/// 演示 QScrollBar 的固定高度（像素），作为 availableLength 的近似值
static constexpr int kScrollBarHeight = 300;

/// 默认 range 最小值
static constexpr int kDefaultMin = 0;

/// 默认 range 最大值
static constexpr int kDefaultMax = 100;

/// 默认 pageStep
static constexpr int kDefaultPageStep = 20;

/// 默认 singleStep
static constexpr int kDefaultSingleStep = 5;

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

SliderSizeDemo::SliderSizeDemo(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(createParamSection());
    mainLayout->addWidget(createScrollBarSection());
    mainLayout->addWidget(createFormulaSection());
    mainLayout->addStretch();

    setWindowTitle(QStringLiteral("QScrollBar Slider Size Formula Demo"));
    resize(500, 600);

    // 初始计算一次
    updateDisplay();
}

// ─────────────────────────────────────────────────────────────────────────────
// 参数控制区域
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SliderSizeDemo::createParamSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* title = new QLabel(QStringLiteral("1. 参数调整"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    layout->addWidget(title);

    // min / max 行
    auto* rangeRow = new QHBoxLayout;
    auto* minLabel = new QLabel(QStringLiteral("min:"));
    m_minSpin = new QSpinBox;
    m_minSpin->setRange(0, 9999);
    m_minSpin->setValue(kDefaultMin);

    auto* maxLabel = new QLabel(QStringLiteral("max:"));
    m_maxSpin = new QSpinBox;
    m_maxSpin->setRange(0, 9999);
    m_maxSpin->setValue(kDefaultMax);

    rangeRow->addWidget(minLabel);
    rangeRow->addWidget(m_minSpin);
    rangeRow->addWidget(maxLabel);
    rangeRow->addWidget(m_maxSpin);
    layout->addLayout(rangeRow);

    // pageStep / singleStep 行
    auto* stepRow = new QHBoxLayout;
    auto* pageStepLabel = new QLabel(QStringLiteral("pageStep:"));
    m_pageStepSpin = new QSpinBox;
    m_pageStepSpin->setRange(1, 9999);
    m_pageStepSpin->setValue(kDefaultPageStep);

    auto* singleStepLabel = new QLabel(QStringLiteral("singleStep:"));
    m_singleStepSpin = new QSpinBox;
    m_singleStepSpin->setRange(1, 9999);
    m_singleStepSpin->setValue(kDefaultSingleStep);

    stepRow->addWidget(pageStepLabel);
    stepRow->addWidget(m_pageStepSpin);
    stepRow->addWidget(singleStepLabel);
    stepRow->addWidget(m_singleStepSpin);
    layout->addLayout(stepRow);

    // 任意参数变化时刷新显示
    connect(m_minSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SliderSizeDemo::updateDisplay);
    connect(m_maxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SliderSizeDemo::updateDisplay);
    connect(m_pageStepSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SliderSizeDemo::updateDisplay);
    connect(m_singleStepSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SliderSizeDemo::updateDisplay);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// QScrollBar 演示区域
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SliderSizeDemo::createScrollBarSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* title = new QLabel(QStringLiteral("2. 实时 QScrollBar 效果"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    layout->addWidget(title);

    m_scrollBar = new QScrollBar(Qt::Vertical);
    m_scrollBar->setFixedHeight(kScrollBarHeight);
    layout->addWidget(m_scrollBar, 0, Qt::AlignHCenter);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 公式计算结果显示区域
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SliderSizeDemo::createFormulaSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* title = new QLabel(QStringLiteral("3. 手柄大小公式计算"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    layout->addWidget(title);

    // 公式代入文本
    m_formulaLabel = new QLabel;
    m_formulaLabel->setWordWrap(true);
    layout->addWidget(m_formulaLabel);

    // 计算结果
    m_resultLabel = new QLabel;
    m_resultLabel->setWordWrap(true);
    layout->addWidget(m_resultLabel);

    // 参数异常警告
    m_warningLabel = new QLabel;
    m_warningLabel->setWordWrap(true);
    m_warningLabel->setStyleSheet(QStringLiteral("color: orange;"));
    layout->addWidget(m_warningLabel);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 核心计算逻辑
// ─────────────────────────────────────────────────────────────────────────────

void SliderSizeDemo::updateDisplay()
{
    const int minVal = m_minSpin->value();
    const int maxVal = m_maxSpin->value();
    const int pageStep = m_pageStepSpin->value();
    const int singleStep = m_singleStepSpin->value();

    // 更新 QScrollBar 参数
    m_scrollBar->setRange(minVal, maxVal);
    m_scrollBar->setPageStep(pageStep);
    m_scrollBar->setSingleStep(singleStep);

    // 公式：sliderSize = pageStep / (max - min + pageStep) * availableLength
    const int range = maxVal - minVal;
    const int divider = range + pageStep;

    // availableLength 近似为滚动条的高度
    const int availableLength = kScrollBarHeight;

    // 公式代入展示
    m_formulaLabel->setText(
        QStringLiteral("公式: sliderSize = pageStep / (max - min + pageStep) * availableLength\n"
                       "代入: %1 / (%2 - %3 + %1) * %4\n"
                       "    = %1 / %5 * %4")
            .arg(pageStep)
            .arg(maxVal)
            .arg(minVal)
            .arg(availableLength)
            .arg(divider));

    // 收集警告信息
    QStringList warnings;

    if (maxVal <= minVal) {
        // range <= 0：无有效范围，手柄撑满 groove
        m_resultLabel->setText(
            QStringLiteral("range = max - min = %1 - %2 = %3\n"
                           "range <= 0，内容不超过视口，手柄撑满 groove (sliderSize = %4 px)")
                .arg(maxVal)
                .arg(minVal)
                .arg(range)
                .arg(availableLength));
        warnings << QStringLiteral("max <= min，滚动条无有效范围");
    } else if (divider <= 0) {
        // 理论上 divider = range + pageStep，pageStep >= 1 所以不会为 0
        m_resultLabel->setText(
            QStringLiteral("divider = %1，无法计算").arg(divider));
    } else {
        const int sliderSize = pageStep * availableLength / divider;
        const double ratio = static_cast<double>(sliderSize) / availableLength * 100.0;

        m_resultLabel->setText(
            QStringLiteral("range = %1, divider = %2\n"
                           "sliderSize = %3 / %4 * %5 = %6 px\n"
                           "手柄占 groove 比例: %7%")
                .arg(range)
                .arg(divider)
                .arg(pageStep)
                .arg(divider)
                .arg(availableLength)
                .arg(sliderSize)
                .arg(QString::number(ratio, 'f', 1)));

        // 手柄过小警告
        if (sliderSize < 20) {
            warnings << QStringLiteral(
                "计算手柄大小仅 %1 px，Qt 会钳位到系统最小值 (PM_ScrollBarSliderMin)");
        }
    }

    // 语义约束警告
    if (pageStep > range + pageStep) {
        // pageStep > max - min + pageStep 意味着 range < 0，已在上面处理
    } else if (range > 0 && pageStep >= range) {
        // pageStep >= range 时手柄几乎撑满，用户几乎无法滚动
        warnings << QStringLiteral(
            "pageStep(%1) >= range(%2)，手柄几乎撑满 groove，滚动空间极小")
                     .arg(pageStep)
                     .arg(range);
    }

    if (singleStep > pageStep) {
        warnings << QStringLiteral(
            "singleStep(%1) > pageStep(%2)，方向键步进比点击 groove 还大，语义不合理")
                     .arg(singleStep)
                     .arg(pageStep);
    }

    if (pageStep <= 0) {
        warnings << QStringLiteral("pageStep 为 0，手柄大小计算结果为 0，无法拖拽");
    }

    // 显示警告（无警告时隐藏）
    if (warnings.isEmpty()) {
        m_warningLabel->hide();
    } else {
        m_warningLabel->setText(QStringLiteral("⚠ 警告: ") + warnings.join(QStringLiteral(" | ")));
        m_warningLabel->show();
    }
}
