/// @file    state_machine_demo.cpp
/// @brief   StateMachineDemo 实现——QStateMachine 驱动的多状态并行动画。
///
/// 对应教程：进阶层 03-QtWidgets/09-动画进阶。

#include "state_machine_demo.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSequentialAnimationGroup>
#include <QSignalTransition>
#include <QState>
#include <QStateMachine>
#include <QVBoxLayout>

// ─── 常量 ───────────────────────────────────────────────────────────────────────

/// 目标方块在 Normal 状态下的初始几何。
static const QRect kNormalGeometry = QRect(20, 20, 80, 80);

/// 目标方块在 Focused 状态下的几何（放大并右移）。
static const QRect kFocusedGeometry = QRect(60, 10, 140, 140);

/// 目标方块在 Animated 状态下的几何（移到右侧并缩到 100x100）。
static const QRect kAnimatedGeometry = QRect(350, 120, 100, 100);

/// 动画时长——聚焦过渡。
static constexpr int kFocusDuration = 300;

/// 动画时长——动画过渡（并行组）。
static constexpr int kAnimateDuration = 600;

/// 动画时长——重置过渡。
static constexpr int kResetDuration = 400;

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

StateMachineDemo::StateMachineDemo(QWidget* parent) : QWidget(parent) {
    setupUI();
    buildStateMachine();

    setWindowTitle(QStringLiteral("QStateMachine Animation Demo"));
    setFixedSize(500, 350);
}

// ─────────────────────────────────────────────────────────────────────────────
// 界面搭建
// ─────────────────────────────────────────────────────────────────────────────

void StateMachineDemo::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // 上方：动画演示区域（目标方块在此区域内移动）
    auto* stage = new QWidget;
    stage->setMinimumHeight(200);
    stage->setStyleSheet(
        QStringLiteral("background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 4px;"));

    // 目标方块——被动画驱动的 QLabel
    m_target = new QLabel(QStringLiteral("Target"), stage);
    m_target->setAlignment(Qt::AlignCenter);
    m_target->setGeometry(kNormalGeometry);
    m_target->setStyleSheet(
        QStringLiteral("background-color: #4a90d9; color: white; border-radius: 8px; "
                       "font-weight: bold; font-size: 13px;"));

    mainLayout->addWidget(stage, 1);

    // 下方：控制按钮
    auto* btnLayout = new QVBoxLayout;

    m_focusBtn = new QPushButton(QStringLiteral("聚焦（放大 + 右移）"));
    m_animateBtn = new QPushButton(QStringLiteral("动画（并行：移动 + 透明度 + 缩放）"));
    m_resetBtn = new QPushButton(QStringLiteral("重置"));

    btnLayout->addWidget(m_focusBtn);
    btnLayout->addWidget(m_animateBtn);
    btnLayout->addWidget(m_resetBtn);

    mainLayout->addLayout(btnLayout);

    // 状态标签
    m_statusLabel = new QLabel(QStringLiteral("当前状态：Normal"));
    m_statusLabel->setStyleSheet(QStringLiteral("font-weight: bold; color: #333;"));
    mainLayout->addWidget(m_statusLabel);
}

// ─────────────────────────────────────────────────────────────────────────────
// 状态机构建
// ─────────────────────────────────────────────────────────────────────────────

void StateMachineDemo::buildStateMachine() {
    auto* machine = new QStateMachine(this);

    // 透明度走 QGraphicsOpacityEffect：windowOpacity 只对顶层窗口生效，
    // 方块是舞台里的子控件，动画与状态赋值都指向效果器的 opacity。
    auto* opacityEffect = new QGraphicsOpacityEffect(m_target);
    m_target->setGraphicsEffect(opacityEffect);

    // ── Normal 状态 ──────────────────────────────────────────────────────────
    auto* sNormal = new QState();
    sNormal->assignProperty(m_target, "geometry", kNormalGeometry);
    sNormal->assignProperty(opacityEffect, "opacity", 1.0);
    sNormal->assignProperty(m_statusLabel, "text", QStringLiteral("当前状态：Normal"));
    sNormal->assignProperty(m_focusBtn, "enabled", true);
    sNormal->assignProperty(m_animateBtn, "enabled", true);
    sNormal->assignProperty(m_resetBtn, "enabled", false);

    // ── Focused 状态 ─────────────────────────────────────────────────────────
    auto* sFocused = new QState();
    sFocused->assignProperty(m_target, "geometry", kFocusedGeometry);
    sFocused->assignProperty(m_statusLabel, "text", QStringLiteral("当前状态：Focused"));
    sFocused->assignProperty(m_focusBtn, "enabled", false);
    sFocused->assignProperty(m_animateBtn, "enabled", true);
    sFocused->assignProperty(m_resetBtn, "enabled", true);

    // ── Animated 状态 ────────────────────────────────────────────────────────
    auto* sAnimated = new QState();
    sAnimated->assignProperty(m_target, "geometry", kAnimatedGeometry);
    sAnimated->assignProperty(opacityEffect, "opacity", 0.4);
    sAnimated->assignProperty(m_statusLabel, "text",
                              QStringLiteral("当前状态：Animated (Parallel)"));
    sAnimated->assignProperty(m_focusBtn, "enabled", false);
    sAnimated->assignProperty(m_animateBtn, "enabled", false);
    sAnimated->assignProperty(m_resetBtn, "enabled", true);

    // ── 状态转换 ────────────────────────────────────────────────────────────

    // Normal -> Focused：点击聚焦按钮，带 geometry 平滑放大动画
    auto* focusAnim = new QPropertyAnimation(m_target, "geometry");
    focusAnim->setDuration(kFocusDuration);
    focusAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto* tToFocused = sNormal->addTransition(m_focusBtn, &QPushButton::clicked, sFocused);
    tToFocused->addAnimation(focusAnim);

    // Normal -> Animated：点击动画按钮
    // 并行动画组：几何（位移 + 缩放合并一路）+ 透明度同时播放
    auto* parallelGroup = new QParallelAnimationGroup;

    // 几何一路：起止 QRect 同时覆盖位移与尺寸，终点值由 sAnimated 的赋值提供。
    // 拆成 pos、geometry 两路的话，geometry 每帧重写会盖掉 pos 刚写的位置。
    auto* geometryAnim = new QPropertyAnimation(m_target, "geometry");
    geometryAnim->setDuration(kAnimateDuration);
    geometryAnim->setEasingCurve(QEasingCurve::OutBounce);
    parallelGroup->addAnimation(geometryAnim);

    // 透明度一路：驱动效果器的 opacity，同样由 sAnimated 的赋值提供终点值
    auto* opacityAnim = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnim->setDuration(kAnimateDuration);
    opacityAnim->setEasingCurve(QEasingCurve::InOutQuad);
    parallelGroup->addAnimation(opacityAnim);

    auto* tToAnimated = sNormal->addTransition(m_animateBtn, &QPushButton::clicked, sAnimated);
    tToAnimated->addAnimation(parallelGroup);

    // Focused -> Normal：点击重置按钮
    sFocused->addTransition(m_resetBtn, &QPushButton::clicked, sNormal);

    // Focused -> Animated：不带动画，属性赋值直接切到位（硬切）
    sFocused->addTransition(m_animateBtn, &QPushButton::clicked, sAnimated);

    // Animated -> Normal：点击重置按钮，带恢复动画
    auto* resetAnim = new QPropertyAnimation(m_target, "geometry");
    resetAnim->setDuration(kResetDuration);
    resetAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto* resetOpacity = new QPropertyAnimation(opacityEffect, "opacity");
    resetOpacity->setDuration(kResetDuration);
    resetOpacity->setEasingCurve(QEasingCurve::OutCubic);

    auto* tToNormal = sAnimated->addTransition(m_resetBtn, &QPushButton::clicked, sNormal);
    tToNormal->addAnimation(resetAnim);
    tToNormal->addAnimation(resetOpacity);

    // ── 注册状态 ────────────────────────────────────────────────────────────
    machine->addState(sNormal);
    machine->addState(sFocused);
    machine->addState(sAnimated);
    machine->setInitialState(sNormal);
    machine->start();
}
