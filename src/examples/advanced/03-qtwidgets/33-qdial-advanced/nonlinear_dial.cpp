/// @file    nonlinear_dial.cpp
/// @brief   NonlinearDial 类实现——对数刻度映射与穿越点修正。
///
/// 对应教程：进阶层 03-QtWidgets/33-QDial 进阶。

#include "nonlinear_dial.h"

#include <cmath>

#include <QDial>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

NonlinearDial::NonlinearDial(QWidget* parent)
    : QWidget(parent)
    , m_dial(nullptr)
    , m_rawValueLabel(nullptr)
    , m_logValueLabel(nullptr)
    , m_wrappingInfoLabel(nullptr)
    , m_notchInfoLabel(nullptr)
    , m_lastRawValue(0)
    , m_accumulatedDelta(0)
{
    setupUI();
}

// ─────────────────────────────────────────────────────────────────────────────
// 界面搭建
// ─────────────────────────────────────────────────────────────────────────────

void NonlinearDial::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // ── 旋钮区域 ──
    m_dial = new QDial;
    m_dial->setRange(0, 99);
    m_dial->setWrapping(false);
    m_dial->setNotchesVisible(true);
    // notchTarget 只是建议值，QDial 内部会根据 range 和尺寸取整
    m_dial->setNotchTarget(10.0);
    m_dial->setFixedSize(180, 180);

    // ── 信息面板（旋钮右侧） ──
    auto* infoLayout = new QVBoxLayout;

    // 内部线性值
    auto* rawTitle = new QLabel(QStringLiteral("内部线性值 (0~99):"));
    m_rawValueLabel = new QLabel(QStringLiteral("0"));

    // 对数映射输出值
    auto* logTitle = new QLabel(QStringLiteral("对数映射输出 (1~1000):"));
    m_logValueLabel = new QLabel(QStringLiteral("1"));

    // 累计旋转增量（模拟旋转编码器）
    auto* accTitle = new QLabel(QStringLiteral("累计旋转增量:"));
    auto* accLabel = new QLabel(QStringLiteral("0"));
    accLabel->setObjectName(QStringLiteral("accLabel"));

    // Wrapping 模式说明
    m_wrappingInfoLabel = new QLabel(
        QStringLiteral("Wrapping: OFF\n有效角度范围约 270 度，底部有死角"));

    // 刻度系统信息
    // notchCount 实际值 = range / notchSize，notchSize 不一定等于 notchTarget
    m_notchInfoLabel = new QLabel(
        QStringLiteral("notchTarget: 10.0 (建议值)\n实际刻度数由 QDial 内部取整决定"));

    // 切换 wrapping 按钮
    auto* wrapBtn = new QPushButton(QStringLiteral("切换 Wrapping 模式"));

    infoLayout->addWidget(rawTitle);
    infoLayout->addWidget(m_rawValueLabel);
    infoLayout->addWidget(logTitle);
    infoLayout->addWidget(m_logValueLabel);
    infoLayout->addWidget(accTitle);
    infoLayout->addWidget(accLabel);
    infoLayout->addWidget(m_wrappingInfoLabel);
    infoLayout->addWidget(m_notchInfoLabel);
    infoLayout->addWidget(wrapBtn);
    infoLayout->addStretch();

    // ── 旋钮 + 信息面板水平排列 ──
    auto* contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_dial, 0, Qt::AlignCenter);
    contentLayout->addLayout(infoLayout, 1);

    mainLayout->addLayout(contentLayout);

    // ── 信号槽连接 ──
    connect(m_dial, &QDial::valueChanged, this, &NonlinearDial::handleValueChanged);

    connect(wrapBtn, &QPushButton::clicked, this, &NonlinearDial::toggleWrapping);

    // 需要找到 accLabel 来更新它——通过 objectName 查找
    auto* accLabelPtr = findChild<QLabel*>(QStringLiteral("accLabel"));
    if (accLabelPtr) {
        connect(m_dial, &QDial::valueChanged, this,
                [this, accLabelPtr]()
                { accLabelPtr->setText(QString::number(m_accumulatedDelta)); });
    }

    // 初始更新
    handleValueChanged(m_dial->value());

    setWindowTitle(QStringLiteral("QDial Advanced Demo"));
    resize(500, 250);
}

// ─────────────────────────────────────────────────────────────────────────────
// 对数映射
// ─────────────────────────────────────────────────────────────────────────────

int NonlinearDial::linearToLog(int dialValue)
{
    // dialValue: 0~99 -> 输出: 1~1000
    // 公式: output = 10^(dialValue / 99 * 3)
    // dialValue=0  -> 10^0   = 1
    // dialValue=99 -> 10^3   = 1000
    double exponent = static_cast<double>(dialValue) / 99.0 * 3.0;
    int result = static_cast<int>(std::round(std::pow(10.0, exponent)));

    // 确保不超出声明范围
    return qBound(kLogMin, result, kLogMax);
}

// ─────────────────────────────────────────────────────────────────────────────
// 值变化处理——更新显示和穿越点修正
// ─────────────────────────────────────────────────────────────────────────────

void NonlinearDial::handleValueChanged(int newValue)
{
    // 更新内部线性值显示
    m_rawValueLabel->setText(QString::number(newValue));

    // 更新对数映射输出
    int logValue = linearToLog(newValue);
    m_logValueLabel->setText(QString::number(logValue));

    // ── 穿越点修正 ──
    // 在 wrapping 模式下，值从 max 跳到 min 时差值是 -(max-min)，
    // 但用户实际只顺时针转了一小格，真正的 delta 应该是 +1。
    // 通用阈值 = (max - min + 1) / 2
    int delta = newValue - m_lastRawValue;
    int range = m_dial->maximum() - m_dial->minimum() + 1;
    int threshold = range / 2;

    if (delta > threshold) {
        // 实际是逆时针（减小方向），修正为负值
        delta -= range;
    } else if (delta < -threshold) {
        // 实际是顺时针（增大方向），修正为正值
        delta += range;
    }

    m_accumulatedDelta += delta;
    m_lastRawValue = newValue;
}

// ─────────────────────────────────────────────────────────────────────────────
// 切换 Wrapping 模式
// ─────────────────────────────────────────────────────────────────────────────

void NonlinearDial::toggleWrapping()
{
    bool wrapping = !m_dial->wrapping();
    m_dial->setWrapping(wrapping);

    if (wrapping) {
        m_wrappingInfoLabel->setText(
            QStringLiteral("Wrapping: ON\n有效角度 360 度，指针可在 0 和 max 间穿越"));
    } else {
        m_wrappingInfoLabel->setText(
            QStringLiteral("Wrapping: OFF\n有效角度范围约 270 度，底部有死角"));
    }

    // 切换模式后重置累计值，避免旧状态的增量干扰
    m_accumulatedDelta = 0;
    m_lastRawValue = m_dial->value();
}
