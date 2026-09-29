/// @file    busy_bar_demo.cpp
/// @brief   BusyBarDemo 类实现——busy 模式动画驱动与 paintEvent 自定义文本演示。
///
/// 对应教程：进阶层 03-QtWidgets/35-QProgressBar 进阶。

#include "busy_bar_demo.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

// ── OverlayTextBar 实现 ──────────────────────────────────────────────────────

OverlayTextBar::OverlayTextBar(QWidget* parent)
    : QProgressBar(parent)
    , m_overlayText()
{
}

void OverlayTextBar::setOverlayText(const QString& text)
{
    m_overlayText = text;
    update();
}

void OverlayTextBar::paintEvent(QPaintEvent* event)
{
    // 必须先让父类画好背景和填充色块——画家算法，后画的覆盖先画的
    QProgressBar::paintEvent(event);

    if (m_overlayText.isEmpty()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 计算填充区域宽度，只在填充区域内绘制白色文字
    const int maxVal = maximum();
    const int minVal = minimum();
    const int range = maxVal - minVal;
    const double ratio =
        (range > 0) ? static_cast<double>(value() - minVal) / range : 0.0;
    const int fillWidth =
        static_cast<int>(width() * ratio);

    painter.setPen(Qt::white);
    painter.setFont(font());

    // clipRect 保证文字只在填充区域内可见，避免深浅色交界处看不清
    painter.setClipRect(0, 0, fillWidth, height());
    painter.drawText(rect(), Qt::AlignCenter, m_overlayText);
}

// ── BusyBarDemo 主窗口 ───────────────────────────────────────────────────────

BusyBarDemo::BusyBarDemo(QWidget* parent)
    : QWidget(parent)
    , m_overlayBar(new OverlayTextBar(this))
    , m_stepCount(0)
{
    setWindowTitle(QStringLiteral("QProgressBar Advanced Demo"));
    resize(520, 360);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(createBusySection(this));
    mainLayout->addWidget(createOverlaySection(this));
    mainLayout->addStretch();
}

// ── 分区 1：busy 模式动画驱动 ────────────────────────────────────────────────

QWidget* BusyBarDemo::createBusySection(QWidget* parent)
{
    auto* box = new QWidget(parent);
    auto* layout = new QVBoxLayout(box);

    auto* title = new QLabel(
        QStringLiteral("1. Busy 模式（setRange(0,0) + QTimer 驱动动画）"));
    title->setStyleSheet(QStringLiteral("font-weight: bold;"));

    // setRange(0, 0) 进入无限进度模式
    auto* busyBar = new QProgressBar;
    busyBar->setRange(0, 0);
    busyBar->setTextVisible(false);

    // 有些 QStyle 的 busy 动画需要外部不断 setValue 才能滚动
    // 这里用 QTimer 以约 30ms 间隔强制刷新，保证跨 style 兼容
    auto* animDriver = new QTimer(box);
    connect(animDriver, &QTimer::timeout, this, [busyBar]() {
        busyBar->setValue(busyBar->value() + 1);
    });
    animDriver->start(30);

    auto* controlRow = new QHBoxLayout;
    auto* startBtn = new QPushButton(QStringLiteral("开始"));
    auto* stopBtn = new QPushButton(QStringLiteral("停止"));

    connect(startBtn, &QPushButton::clicked, this, [animDriver]() {
        animDriver->start(30);
    });
    connect(stopBtn, &QPushButton::clicked, this, [animDriver]() {
        animDriver->stop();
    });

    auto* hint = new QLabel(
        QStringLiteral("某些 style 下关闭定时器后动画会停止——"
                       "说明动画靠 setValue 驱动而非内部定时器"));

    layout->addWidget(title);
    layout->addWidget(busyBar);
    controlRow->addWidget(startBtn);
    controlRow->addWidget(stopBtn);
    layout->addLayout(controlRow);
    layout->addWidget(hint);

    return box;
}

// ── 分区 2：paintEvent 自定义文本覆盖 ────────────────────────────────────────

QWidget* BusyBarDemo::createOverlaySection(QWidget* parent)
{
    auto* box = new QWidget(parent);
    auto* layout = new QVBoxLayout(box);

    auto* title = new QLabel(
        QStringLiteral("2. paintEvent 自定义文本（文字仅在填充区域内可见）"));
    title->setStyleSheet(QStringLiteral("font-weight: bold;"));

    // 使用自定义的 OverlayTextBar
    m_overlayBar->setRange(0, 10);
    m_overlayBar->setValue(0);
    m_overlayBar->setTextVisible(false);
    m_overlayBar->setFixedHeight(32);
    m_overlayBar->setOverlayText(
        QStringLiteral("第 0 / 10 步"));

    auto* controlRow = new QHBoxLayout;
    auto* nextBtn = new QPushButton(QStringLiteral("下一步"));
    auto* resetBtn = new QPushButton(QStringLiteral("重置"));

    connect(nextBtn, &QPushButton::clicked, this, [this]() {
        if (m_stepCount < 10) {
            ++m_stepCount;
            m_overlayBar->setValue(m_stepCount);
            m_overlayBar->setOverlayText(
                QStringLiteral("第 %1 / 10 步").arg(m_stepCount));
        }
    });
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_stepCount = 0;
        m_overlayBar->setValue(0);
        m_overlayBar->setOverlayText(
            QStringLiteral("第 0 / 10 步"));
    });

    layout->addWidget(title);
    layout->addWidget(m_overlayBar);
    controlRow->addWidget(nextBtn);
    controlRow->addWidget(resetBtn);
    layout->addLayout(controlRow);

    return box;
}
