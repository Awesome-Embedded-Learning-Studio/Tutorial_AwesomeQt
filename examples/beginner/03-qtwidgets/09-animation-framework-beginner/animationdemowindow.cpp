// QtWidgets 入门示例 09: 属性动画框架基础
// AnimationDemoWindow 实现

#include "animationdemowindow.h"
#include "colorwidget.h"

#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

AnimationDemoWindow::AnimationDemoWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("属性动画框架基础演示");
    resize(800, 500);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("QPropertyAnimation + QEasingCurve 演示");
    titleLabel->setFont(QFont("Arial", 16, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ---- 缓动曲线选择 ----
    auto *curveLayout = new QHBoxLayout();
    auto *curveLabel = new QLabel("缓动曲线:");
    m_curveCombo = new QComboBox();
    m_curveCombo->addItems({
        "Linear", "InQuad", "OutQuad", "InOutQuad",
        "InCubic", "OutCubic", "InOutCubic",
        "InBack", "OutBack", "OutBounce"
    });
    m_curveCombo->setCurrentText("OutCubic");
    curveLayout->addWidget(curveLabel);
    curveLayout->addWidget(m_curveCombo);
    curveLayout->addStretch();
    mainLayout->addLayout(curveLayout);

    // ---- 色块区域 ----
    auto *blockArea = new QWidget();
    auto *blockLayout = new QHBoxLayout(blockArea);
    blockLayout->setSpacing(30);
    blockLayout->setContentsMargins(0, 30, 0, 30);

    m_block1 = new ColorWidget(QColor("#2980B9"), "Block A");
    m_block2 = new ColorWidget(QColor("#27AE60"), "Block B");
    m_block3 = new ColorWidget(QColor("#E74C3C"), "Block C");

    blockLayout->addWidget(m_block1);
    blockLayout->addWidget(m_block2);
    blockLayout->addWidget(m_block3);
    blockLayout->addStretch();

    mainLayout->addWidget(blockArea, 1);

    // ---- 按钮栏 ----
    auto *btnLayout = new QHBoxLayout();

    auto *playBtn = new QPushButton("播放动画");
    connect(playBtn, &QPushButton::clicked,
            this, &AnimationDemoWindow::playAnimation);

    auto *resetBtn = new QPushButton("重置");
    connect(resetBtn, &QPushButton::clicked,
            this, &AnimationDemoWindow::resetAll);

    auto *colorBtn = new QPushButton("颜色渐变");
    connect(colorBtn, &QPushButton::clicked,
            this, &AnimationDemoWindow::playColorAnimation);

    btnLayout->addWidget(playBtn);
    btnLayout->addWidget(colorBtn);
    btnLayout->addWidget(resetBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    // ---- 状态标签 ----
    m_statusLabel = new QLabel("就绪 — 点击\"播放动画\"查看效果");
    m_statusLabel->setStyleSheet("color: #666; font-size: 12px;");
    mainLayout->addWidget(m_statusLabel);

    // 初始位置
    resetAll();
}

QEasingCurve AnimationDemoWindow::currentCurve() const
{
    QString name = m_curveCombo->currentText();
    if (name == "Linear")     return QEasingCurve::Linear;
    if (name == "InQuad")     return QEasingCurve::InQuad;
    if (name == "OutQuad")    return QEasingCurve::OutQuad;
    if (name == "InOutQuad")  return QEasingCurve::InOutQuad;
    if (name == "InCubic")    return QEasingCurve::InCubic;
    if (name == "OutCubic")   return QEasingCurve::OutCubic;
    if (name == "InOutCubic") return QEasingCurve::InOutCubic;
    if (name == "InBack")     return QEasingCurve::InBack;
    if (name == "OutBack")    return QEasingCurve::OutBack;
    if (name == "OutBounce")  return QEasingCurve::OutBounce;
    return QEasingCurve::Linear;
}

/// @brief 播放串行位移动画：三个色块依次从左侧飞入
void AnimationDemoWindow::playAnimation()
{
    m_statusLabel->setText("播放中 — 串行位移动画...");

    // 先将色块移到左侧屏幕外
    int baseY = m_block1->y();
    m_block1->move(-200, baseY);
    m_block2->move(-200, baseY);
    m_block3->move(-200, baseY);

    QEasingCurve curve = currentCurve();

    auto *group = new QSequentialAnimationGroup(this);

    // 第一个色块飞入
    auto *anim1 = new QPropertyAnimation(m_block1, "pos");
    anim1->setDuration(600);
    anim1->setEndValue(m_pos1);
    anim1->setEasingCurve(curve);
    group->addAnimation(anim1);

    // 短暂停顿
    group->addPause(150);

    // 第二个色块飞入
    auto *anim2 = new QPropertyAnimation(m_block2, "pos");
    anim2->setDuration(600);
    anim2->setEndValue(m_pos2);
    anim2->setEasingCurve(curve);
    group->addAnimation(anim2);

    // 短暂停顿
    group->addPause(150);

    // 第三个色块飞入
    auto *anim3 = new QPropertyAnimation(m_block3, "pos");
    anim3->setDuration(600);
    anim3->setEndValue(m_pos3);
    anim3->setEasingCurve(curve);
    group->addAnimation(anim3);

    connect(group, &QSequentialAnimationGroup::finished, this, [this]() {
        m_statusLabel->setText("位移动画完成 — 试试切换缓动曲线");
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/// @brief 播放颜色渐变动画：三个色块同时从白色渐变到目标色
void AnimationDemoWindow::playColorAnimation()
{
    m_statusLabel->setText("播放中 — 颜色渐变动画...");

    auto *group = new QParallelAnimationGroup(this);

    auto targets = {m_block1, m_block2, m_block3};
    for (auto *block : targets) {
        auto *anim = new QPropertyAnimation(block, "backgroundColor");
        anim->setDuration(1000);
        anim->setStartValue(QColor(Qt::white));
        anim->setEndValue(block->property("backgroundColor").value<QColor>());
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        group->addAnimation(anim);
    }

    connect(group, &QParallelAnimationGroup::finished, this, [this]() {
        m_statusLabel->setText("颜色渐变完成");
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/// @brief 重置所有色块到初始位置和颜色
void AnimationDemoWindow::resetAll()
{
    // 记录色块的初始位置（用于动画目标）
    m_block1->move(0, 0);
    m_block2->move(0, 0);
    m_block3->move(0, 0);

    // 等布局完成后记录位置
    QTimer::singleShot(50, this, [this]() {
        m_pos1 = m_block1->pos();
        m_pos2 = m_block2->pos();
        m_pos3 = m_block3->pos();
    });

    m_block1->reset();
    m_block2->reset();
    m_block3->reset();
    m_statusLabel->setText("就绪 — 点击\"播放动画\"查看效果");
}
