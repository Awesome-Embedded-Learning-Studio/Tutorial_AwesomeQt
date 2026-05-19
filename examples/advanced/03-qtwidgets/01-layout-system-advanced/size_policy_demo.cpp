/// @file    size_policy_demo.cpp
/// @brief   SizePolicyDemo 类实现——六种 QSizePolicy 策略对比、动态切换、stretch 因子演示。
///
/// 对应教程：进阶层 03-QtWidgets/01-布局系统进阶。

#include "size_policy_demo.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

/// @brief 六种策略的显示名称，与 QComboBox 下拉项一一对应。
const QStringList kPolicyNames = {
    QStringLiteral("Fixed"),
    QStringLiteral("Minimum"),
    QStringLiteral("Maximum"),
    QStringLiteral("Preferred"),
    QStringLiteral("Expanding"),
    QStringLiteral("Ignored"),
};

/// @brief 六种策略对应的枚举值。
const QVector<QSizePolicy::Policy> kPolicyValues = {
    QSizePolicy::Fixed,     QSizePolicy::Minimum,  QSizePolicy::Maximum,
    QSizePolicy::Preferred, QSizePolicy::Expanding, QSizePolicy::Ignored,
};

/// @brief 六种策略对应的色条颜色。
const QStringList kPolicyColors = {
    QStringLiteral("#E74C3C"),  // Fixed    - 红色
    QStringLiteral("#E67E22"),  // Minimum  - 橙色
    QStringLiteral("#F1C40F"),  // Maximum  - 黄色
    QStringLiteral("#2ECC71"),  // Preferred- 绿色
    QStringLiteral("#3498DB"),  // Expanding- 蓝色
    QStringLiteral("#9B59B6"),  // Ignored  - 紫色
};

}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

SizePolicyDemo::SizePolicyDemo(QWidget* parent) : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 三个演示区域依次排列
    mainLayout->addWidget(createPolicyComparisonSection());
    mainLayout->addWidget(createDynamicSwitchSection());
    mainLayout->addWidget(createStretchSection());
    mainLayout->addStretch();

    setWindowTitle(QStringLiteral("QSizePolicy Advanced Demo"));
    resize(800, 600);
}

// ─────────────────────────────────────────────────────────────────────────────
// 六种策略并排对比
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SizePolicyDemo::createPolicyComparisonSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(
        QStringLiteral("1. 六种 QSizePolicy 策略并排对比（拖拽窗口边缘观察空间分配）"));
    sectionTitle->setWordWrap(true);

    // 色条区域：六根色条水平排列，每根的 sizePolicy 不同
    auto* barsLayout = new QHBoxLayout;

    m_barFixed = createPolicyBar(kPolicyNames[0], kPolicyColors[0], QSizePolicy::Fixed);
    m_barMinimum = createPolicyBar(kPolicyNames[1], kPolicyColors[1], QSizePolicy::Minimum);
    m_barMaximum = createPolicyBar(kPolicyNames[2], kPolicyColors[2], QSizePolicy::Maximum);
    m_barPreferred = createPolicyBar(kPolicyNames[3], kPolicyColors[3], QSizePolicy::Preferred);
    m_barExpanding = createPolicyBar(kPolicyNames[4], kPolicyColors[4], QSizePolicy::Expanding);
    m_barIgnored = createPolicyBar(kPolicyNames[5], kPolicyColors[5], QSizePolicy::Ignored);

    barsLayout->addWidget(m_barFixed);
    barsLayout->addWidget(m_barMinimum);
    barsLayout->addWidget(m_barMaximum);
    barsLayout->addWidget(m_barPreferred);
    barsLayout->addWidget(m_barExpanding);
    barsLayout->addWidget(m_barIgnored);

    // 对比信息标签
    m_comparisonInfo = new QLabel;
    m_comparisonInfo->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(barsLayout);
    layout->addWidget(m_comparisonInfo);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 动态切换 sizePolicy
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SizePolicyDemo::createDynamicSwitchSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(
        QStringLiteral("2. 动态切换 sizePolicy（选择策略后观察色条宽度变化）"));
    sectionTitle->setWordWrap(true);

    // 控制行：下拉框选择策略
    auto* controlRow = new QHBoxLayout;
    auto* comboLabel = new QLabel(QStringLiteral("目标色条策略:"));

    m_policyCombo = new QComboBox;
    for (int i = 0; i < kPolicyNames.size(); ++i) {
        m_policyCombo->addItem(kPolicyNames[i], static_cast<int>(kPolicyValues[i]));
    }
    // 默认选 Preferred
    m_policyCombo->setCurrentIndex(3);

    controlRow->addWidget(comboLabel);
    controlRow->addWidget(m_policyCombo, 1);

    // 被动态控制的色条
    auto* barRow = new QHBoxLayout;
    // 左右各放一个固定色条作参照
    auto* refLeft = new QFrame;
    refLeft->setFixedHeight(40);
    refLeft->setStyleSheet(
        QStringLiteral("background-color: #BDC3C7; border: 1px solid #95A5A6;"));
    refLeft->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    // sizeHint 提供一个合理起始宽度
    refLeft->setMinimumWidth(60);

    m_dynamicBar = new QFrame;
    m_dynamicBar->setFixedHeight(40);
    m_dynamicBar->setStyleSheet(
        QStringLiteral("background-color: #2ECC71; border: 2px solid #27AE60;"));
    m_dynamicBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_dynamicBar->setMinimumWidth(60);

    auto* refRight = new QFrame;
    refRight->setFixedHeight(40);
    refRight->setStyleSheet(
        QStringLiteral("background-color: #BDC3C7; border: 1px solid #95A5A6;"));
    refRight->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    refRight->setMinimumWidth(60);

    barRow->addWidget(refLeft, 1);
    barRow->addWidget(m_dynamicBar, 1);
    barRow->addWidget(refRight, 1);

    // 信息标签
    m_dynamicInfo = new QLabel;
    m_dynamicInfo->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(controlRow);
    layout->addLayout(barRow);
    layout->addWidget(m_dynamicInfo);

    connect(m_policyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &SizePolicyDemo::updateDynamicPolicy);

    // 初始更新
    updateDynamicPolicy();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// stretch 因子演示
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SizePolicyDemo::createStretchSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("3. stretch 因子（所有色条策略为 Expanding，调整 stretch 值）"));
    sectionTitle->setWordWrap(true);

    // stretch 值控制行
    auto* spinRow = new QHBoxLayout;
    auto* labelA = new QLabel(QStringLiteral("A stretch:"));
    m_spinA = new QSpinBox;
    m_spinA->setRange(0, 10);
    m_spinA->setValue(1);

    auto* labelB = new QLabel(QStringLiteral("B stretch:"));
    m_spinB = new QSpinBox;
    m_spinB->setRange(0, 10);
    m_spinB->setValue(2);

    auto* labelC = new QLabel(QStringLiteral("C stretch:"));
    m_spinC = new QSpinBox;
    m_spinC->setRange(0, 10);
    m_spinC->setValue(3);

    spinRow->addWidget(labelA);
    spinRow->addWidget(m_spinA);
    spinRow->addWidget(labelB);
    spinRow->addWidget(m_spinB);
    spinRow->addWidget(labelC);
    spinRow->addWidget(m_spinC);
    spinRow->addStretch();

    // 三根 Expanding 色条
    auto* barRow = new QHBoxLayout;

    m_stretchBarA = new QFrame;
    m_stretchBarA->setFixedHeight(40);
    m_stretchBarA->setStyleSheet(
        QStringLiteral("background-color: #3498DB; border: 1px solid #2980B9;"));
    m_stretchBarA->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_stretchBarB = new QFrame;
    m_stretchBarB->setFixedHeight(40);
    m_stretchBarB->setStyleSheet(
        QStringLiteral("background-color: #E67E22; border: 1px solid #D35400;"));
    m_stretchBarB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_stretchBarC = new QFrame;
    m_stretchBarC->setFixedHeight(40);
    m_stretchBarC->setStyleSheet(
        QStringLiteral("background-color: #2ECC71; border: 1px solid #27AE60;"));
    m_stretchBarC->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    barRow->addWidget(m_stretchBarA);
    barRow->addWidget(m_stretchBarB);
    barRow->addWidget(m_stretchBarC);

    // stretch 信息
    m_stretchInfo = new QLabel;
    m_stretchInfo->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(spinRow);
    layout->addLayout(barRow);
    layout->addWidget(m_stretchInfo);

    // 三个 SpinBox 的值变化都会触发 stretch 重建
    connect(m_spinA, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SizePolicyDemo::updateStretchValues);
    connect(m_spinB, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SizePolicyDemo::updateStretchValues);
    connect(m_spinC, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &SizePolicyDemo::updateStretchValues);

    // 初始设置 stretch
    updateStretchValues();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 辅助：创建策略色条
// ─────────────────────────────────────────────────────────────────────────────

QFrame* SizePolicyDemo::createPolicyBar(const QString& policyName, const QString& color,
                                        QSizePolicy::Policy policy, QWidget* parent)
{
    auto* frame = new QFrame(parent);
    frame->setFixedHeight(50);
    frame->setStyleSheet(
        QStringLiteral("background-color: %1; border: 2px solid #333;").arg(color));

    // 垂直布局：策略名 + sizeHint 标签
    auto* innerLayout = new QVBoxLayout(frame);
    innerLayout->setContentsMargins(4, 2, 4, 2);

    auto* nameLabel = new QLabel(policyName);
    nameLabel->setAlignment(Qt::AlignCenter);
    // 白色文字在深色背景上清晰可见
    nameLabel->setStyleSheet(QStringLiteral("color: white; font-weight: bold; font-size: 12px;"));
    nameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    innerLayout->addWidget(nameLabel);

    // 设置策略——水平方向用传入的策略，垂直方向 Fixed
    frame->setSizePolicy(policy, QSizePolicy::Fixed);

    return frame;
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数：动态切换策略
// ─────────────────────────────────────────────────────────────────────────────

void SizePolicyDemo::updateDynamicPolicy()
{
    const auto policy =
        static_cast<QSizePolicy::Policy>(m_policyCombo->currentData().toInt());

    // 设置水平策略，垂直保持 Fixed
    m_dynamicBar->setSizePolicy(policy, QSizePolicy::Fixed);

    // 根据策略类型给出解释
    QString description;
    switch (policy) {
        case QSizePolicy::Fixed:
            description = QStringLiteral("Fixed: 只接受 sizeHint 大小，多余空间全给其他控件");
            break;
        case QSizePolicy::Minimum:
            description = QStringLiteral("Minimum: 至少需要 sizeHint 大小，可以更大");
            break;
        case QSizePolicy::Maximum:
            description = QStringLiteral("Maximum: 最多接受 sizeHint 大小，不能更大");
            break;
        case QSizePolicy::Preferred:
            description = QStringLiteral("Preferred: 希望 sizeHint 大小，但可大可小（默认策略）");
            break;
        case QSizePolicy::Expanding:
            description = QStringLiteral("Expanding: 希望 sizeHint 大小，且主动要求更多空间");
            break;
        case QSizePolicy::Ignored:
            description = QStringLiteral("Ignored: 完全忽略 sizeHint，大小由布局决定");
            break;
        default:
            description = QStringLiteral("未知策略");
            break;
    }

    m_dynamicInfo->setText(
        QStringLiteral("当前策略: %1\n%2").arg(m_policyCombo->currentText(), description));
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数：更新 stretch 值
// ─────────────────────────────────────────────────────────────────────────────

void SizePolicyDemo::updateStretchValues()
{
    const int stretchA = m_spinA->value();
    const int stretchB = m_spinB->value();
    const int stretchC = m_spinC->value();

    // 找到色条所在的 QHBoxLayout 并更新 stretch 因子
    auto* barLayout = qobject_cast<QHBoxLayout*>(m_stretchBarA->parentWidget()->layout());
    if (!barLayout) {
        return;
    }

    // 找到色条在布局中的索引并设置 stretch
    // stretch 只对愿意接受额外空间的策略（如 Expanding）生效
    int idxA = barLayout->indexOf(m_stretchBarA);
    int idxB = barLayout->indexOf(m_stretchBarB);
    int idxC = barLayout->indexOf(m_stretchBarC);

    if (idxA >= 0) {
        barLayout->setStretch(idxA, stretchA);
    }
    if (idxB >= 0) {
        barLayout->setStretch(idxB, stretchB);
    }
    if (idxC >= 0) {
        barLayout->setStretch(idxC, stretchC);
    }

    // 强制立即重新计算布局
    barLayout->invalidate();
    barLayout->activate();

    const int total = stretchA + stretchB + stretchC;
    if (total == 0) {
        m_stretchInfo->setText(
            QStringLiteral("所有 stretch 值为 0，三个色条平分空间（stretch=0 时等权分配）"));
    } else {
        m_stretchInfo->setText(
            QStringLiteral("stretch 比例 A:B:C = %1:%2:%3（总计 %4 份）\n"
                           "提示：stretch 只对 Expanding/Preferred 策略生效，对 Fixed 无效")
                .arg(stretchA)
                .arg(stretchB)
                .arg(stretchC)
                .arg(total));
    }
}
