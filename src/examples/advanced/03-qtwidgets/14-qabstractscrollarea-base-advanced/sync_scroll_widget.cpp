/// @file    sync_scroll_widget.cpp
/// @brief   SyncScrollWidget 类实现——双面板同步滚动。
///
/// 对应教程：进阶层 03-QtWidgets/14-QAbstractScrollArea 基类进阶。

#include "sync_scroll_widget.h"
#include "sync_text_area.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

SyncScrollWidget::SyncScrollWidget(QWidget* parent)
    : QWidget(parent)
    , m_syncing(false)
{
    auto* mainLayout = new QVBoxLayout(this);

    // ── 两个并排的滚动区域 ──
    auto* areasLayout = new QHBoxLayout;

    m_leftArea = new SyncTextArea;
    m_rightArea = new SyncTextArea;

    // 为每个区域添加标题标签
    auto* leftContainer = new QVBoxLayout;
    auto* leftTitle = new QLabel(QStringLiteral("Left Panel"));
    leftTitle->setAlignment(Qt::AlignCenter);
    leftContainer->addWidget(leftTitle);
    leftContainer->addWidget(m_leftArea, 1);

    auto* rightContainer = new QVBoxLayout;
    auto* rightTitle = new QLabel(QStringLiteral("Right Panel"));
    rightTitle->setAlignment(Qt::AlignCenter);
    rightContainer->addWidget(rightTitle);
    rightContainer->addWidget(m_rightArea, 1);

    areasLayout->addLayout(leftContainer, 1);
    areasLayout->addLayout(rightContainer, 1);

    mainLayout->addLayout(areasLayout, 1);

    // ── 底部控制栏 ──
    auto* controlLayout = new QHBoxLayout;

    m_syncCheckBox = new QCheckBox(QStringLiteral("Synchronize Scrolling"));
    m_syncCheckBox->setChecked(true);

    m_resetButton = new QPushButton(QStringLiteral("Reset to Top"));

    m_positionLabel = new QLabel(QStringLiteral("Left: 0% | Right: 0%"));
    m_positionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    controlLayout->addWidget(m_syncCheckBox);
    controlLayout->addWidget(m_resetButton);
    controlLayout->addStretch();
    controlLayout->addWidget(m_positionLabel);

    mainLayout->addLayout(controlLayout);

    // ── 信号槽连接 ──

    // 双向同步：各自滚动时通知对方
    connect(m_leftArea, &SyncTextArea::scrollChanged,
            this, &SyncScrollWidget::onLeftScrolled);
    connect(m_rightArea, &SyncTextArea::scrollChanged,
            this, &SyncScrollWidget::onRightScrolled);

    // 控制栏
    connect(m_syncCheckBox, &QCheckBox::toggled,
            this, &SyncScrollWidget::toggleSyncMode);
    connect(m_resetButton, &QPushButton::clicked,
            this, &SyncScrollWidget::resetScroll);

    // 滚动条值变化时更新位置百分比显示
    connect(m_leftArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &SyncScrollWidget::updatePositionLabel);
    connect(m_rightArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &SyncScrollWidget::updatePositionLabel);

    setWindowTitle(QStringLiteral("Synchronized Scrolling Demo"));
    resize(800, 500);
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数
// ─────────────────────────────────────────────────────────────────────────────

void SyncScrollWidget::onLeftScrolled(int value)
{
    if (!m_syncCheckBox->isChecked() || m_syncing) {
        return;
    }

    // blockSignals 防止右侧 setValue 触发 onRightScrolled 导致循环
    m_syncing = true;
    m_rightArea->setScrollValueSilently(value);
    m_syncing = false;

    updatePositionLabel();
}

void SyncScrollWidget::onRightScrolled(int value)
{
    if (!m_syncCheckBox->isChecked() || m_syncing) {
        return;
    }

    m_syncing = true;
    m_leftArea->setScrollValueSilently(value);
    m_syncing = false;

    updatePositionLabel();
}

void SyncScrollWidget::toggleSyncMode(bool checked)
{
    if (checked) {
        // 切回同步模式时，将右侧面板跳到左侧面板的位置
        m_rightArea->setScrollValueSilently(m_leftArea->scrollValue());
    }
    updatePositionLabel();
}

void SyncScrollWidget::resetScroll()
{
    m_syncing = true;
    m_leftArea->setScrollValueSilently(0);
    m_rightArea->setScrollValueSilently(0);
    m_syncing = false;
    updatePositionLabel();
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有方法
// ─────────────────────────────────────────────────────────────────────────────

void SyncScrollWidget::updatePositionLabel()
{
    auto calcPercent = [](QScrollBar* bar) -> int {
        const int maxVal = bar->maximum();
        if (maxVal <= 0) {
            return 0;
        }
        // 比例换算：value / maximum * 100
        return static_cast<int>(static_cast<double>(bar->value()) / maxVal * 100.0);
    };

    const int leftPercent = calcPercent(m_leftArea->verticalScrollBar());
    const int rightPercent = calcPercent(m_rightArea->verticalScrollBar());

    m_positionLabel->setText(
        QStringLiteral("Left: %1% | Right: %2%").arg(leftPercent).arg(rightPercent));
}
