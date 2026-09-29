// QtWidgets 入门示例 14: QAbstractScrollArea 滚动区域基类
// 演示：horizontalScrollBar() / verticalScrollBar() 滚动条控制
//       setHorizontalScrollBarPolicy / setVerticalScrollBarPolicy
//       scrollContentsBy() 内容滚动响应
//       视口（viewport）概念与绘制注意事项

#include "demo_window.h"
#include "timeline_scroll_area.h"

#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

// ============================================================================
// DemoWindow: 主演示窗口
// ============================================================================
DemoWindow::DemoWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QAbstractScrollArea 滚动区域基类演示");
    resize(600, 560);
    initUi();
}

/// @brief 初始化界面
void DemoWindow::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("QAbstractScrollArea 自定义时间轴");
    titleLabel->setFont(QFont("Arial", 14, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ---- 时间轴滚动控件 ----
    m_timeline = new TimelineScrollArea();
    mainLayout->addWidget(m_timeline, 1);

    // ---- 当前位置标签 ----
    m_positionLabel = new QLabel("当前时间单位: 0.0");
    m_positionLabel->setStyleSheet(
        "font-size: 13px; color: #333; padding: 4px 0;");
    mainLayout->addWidget(m_positionLabel);

    // 监听滚动位置变化
    connect(m_timeline->verticalScrollBar(), &QScrollBar::valueChanged,
        this, &DemoWindow::updatePositionLabel);

    // ---- 控制面板 ----
    auto *controlGroup = new QGroupBox("控制面板");
    auto *controlLayout = new QHBoxLayout(controlGroup);

    auto *topBtn = new QPushButton("滚动到顶部");
    connect(topBtn, &QPushButton::clicked, this, [this]() {
        m_timeline->verticalScrollBar()->setValue(0);
    });

    auto *bottomBtn = new QPushButton("滚动到底部");
    connect(bottomBtn, &QPushButton::clicked, this, [this]() {
        m_timeline->verticalScrollBar()->setValue(
            m_timeline->verticalScrollBar()->maximum());
    });

    m_policyBtn = new QPushButton("切换滚动条策略: AsNeeded");
    connect(m_policyBtn, &QPushButton::clicked, this, [this]() {
        m_policyToggle = !m_policyToggle;
        if (m_policyToggle) {
            m_timeline->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            m_policyBtn->setText("切换滚动条策略: AlwaysOn");
        } else {
            m_timeline->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            m_policyBtn->setText("切换滚动条策略: AsNeeded");
        }
    });

    controlLayout->addWidget(topBtn);
    controlLayout->addWidget(bottomBtn);
    controlLayout->addWidget(m_policyBtn);
    mainLayout->addWidget(controlGroup);

    // ---- 策略说明 ----
    auto *infoGroup = new QGroupBox("滚动条策略说明");
    auto *infoLayout = new QVBoxLayout(infoGroup);
    auto *infoLabel = new QLabel(
        "ScrollBarAsNeeded — 内容超出时显示滚动条（默认）\n"
        "ScrollBarAlwaysOn — 始终显示滚动条，保持布局稳定\n"
        "ScrollBarAlwaysOff — 隐藏滚动条，但仍可滚轮滚动");
    infoLabel->setStyleSheet("font-size: 12px; color: #555;");
    infoLayout->addWidget(infoLabel);
    mainLayout->addWidget(infoGroup);

    // ---- 底部提示 ----
    auto *hint = new QLabel(
        "提示: 时间轴继承 QAbstractScrollArea，"
        "在 viewport() 上绘制，使用 scrollContentsBy() 响应滚动");
    hint->setStyleSheet("color: #999; font-size: 11px;");
    mainLayout->addWidget(hint);
}

/// @brief 更新位置标签
void DemoWindow::updatePositionLabel()
{
    double unit = m_timeline->currentTimeUnit();
    m_positionLabel->setText(
        QString("当前时间单位: %1 / %2")
            .arg(unit, 0, 'f', 1)
            .arg(100.0, 0, 'f', 0));
}
