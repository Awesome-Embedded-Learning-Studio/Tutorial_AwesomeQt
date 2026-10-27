#include "TimelineBrowser.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWheelEvent>

// ============================================================================
// TimelineBrowser: 时间轴浏览器主窗口
// ============================================================================
TimelineBrowser::TimelineBrowser(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QScrollBar 综合演示 — 自定义时间轴浏览器");
    resize(800, 480);
    initUi();
}

/// @brief 初始化界面
void TimelineBrowser::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ---- 顶部信息栏 ----
    auto *header = new QWidget();
    header->setFixedHeight(40);
    header->setStyleSheet(
        "background-color: #263238;"
        "color: white;"
        "font-size: 13px;"
        "font-weight: bold;");
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 0, 16, 0);

    auto *titleLabel = new QLabel("项目时间轴");
    titleLabel->setStyleSheet("color: white;");
    headerLayout->addWidget(titleLabel);

    m_infoLabel = new QLabel();
    m_infoLabel->setStyleSheet("color: #B0BEC5;");
    m_infoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    headerLayout->addWidget(m_infoLabel);

    mainLayout->addWidget(header);

    // ---- 中央区域：时间轴 + 垂直滚动条 ----
    auto *centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(0);

    m_timeline = new TimelineWidget();
    centerLayout->addWidget(m_timeline, 1);

    // 垂直滚动条控制缩放
    m_zoomScrollBar = new QScrollBar(Qt::Vertical);
    m_zoomScrollBar->setRange(50, 300);  // 映射到 0.5x - 3.0x
    m_zoomScrollBar->setValue(100);      // 默认 1.0x
    m_zoomScrollBar->setSingleStep(10);
    m_zoomScrollBar->setPageStep(50);
    m_zoomScrollBar->setFixedWidth(16);
    m_zoomScrollBar->setStyleSheet(
        "QScrollBar:vertical {"
        "  background: #ECEFF1;"
        "  width: 16px;"
        "  border: none;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #78909C;"
        "  min-height: 30px;"
        "  border-radius: 4px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "  background: none;"
        "}");
    centerLayout->addWidget(m_zoomScrollBar);

    mainLayout->addLayout(centerLayout, 1);

    // ---- 底部：水平滚动条 ----
    m_hScrollBar = new QScrollBar(Qt::Horizontal);
    m_hScrollBar->setSingleStep(30);
    m_hScrollBar->setStyleSheet(
        "QScrollBar:horizontal {"
        "  background: #ECEFF1;"
        "  height: 16px;"
        "  border: none;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #78909C;"
        "  min-width: 40px;"
        "  border-radius: 4px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "  width: 0px;"
        "}"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
        "  background: none;"
        "}");
    mainLayout->addWidget(m_hScrollBar);

    // ---- 状态栏 ----
    auto *statusBar = new QWidget();
    statusBar->setFixedHeight(28);
    statusBar->setStyleSheet(
        "background-color: #ECEFF1;"
        "border-top: 1px solid #CFD8DC;");
    auto *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(12, 0, 12, 0);

    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet(
        "color: #546E7A; font-size: 11px;");
    statusLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(statusBar);

    // ================================================================
    // 信号连接
    // ================================================================
    connect(m_hScrollBar, &QScrollBar::valueChanged,
            m_timeline, &TimelineWidget::setHorizontalOffset);

    connect(m_zoomScrollBar, &QScrollBar::valueChanged,
            this, &TimelineBrowser::onZoomChanged);

    // 滚轮事件转发给水平滚动条
    m_timeline->installEventFilter(this);

    // 初始化
    updateHorizontalScrollBar();
    updateInfo();
}

/// @brief 事件过滤器：转发滚轮事件给水平滚动条
bool TimelineBrowser::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_timeline
        && event->type() == QEvent::Wheel) {
        auto *wheelEvent = static_cast<QWheelEvent *>(event);
        int delta = wheelEvent->angleDelta().y();
        int step = m_hScrollBar->singleStep();

        if (delta < 0) {
            m_hScrollBar->setValue(
                m_hScrollBar->value() + step);
        } else {
            m_hScrollBar->setValue(
                m_hScrollBar->value() - step);
        }
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

/// @brief 缩放变化时更新
void TimelineBrowser::onZoomChanged(int value)
{
    double factor = value / 100.0;
    m_timeline->setZoomFactor(factor);
    updateHorizontalScrollBar();
    updateInfo();
}

/// @brief 根据内容宽度和视口宽度更新水平滚动条
void TimelineBrowser::updateHorizontalScrollBar()
{
    int contentW = m_timeline->contentWidth();
    int viewportW = m_timeline->width();

    if (contentW <= viewportW) {
        m_hScrollBar->setRange(0, 0);
        m_hScrollBar->setValue(0);
    } else {
        m_hScrollBar->setRange(0, contentW - viewportW);
        m_hScrollBar->setPageStep(viewportW);
    }
}

/// @brief 更新信息和状态栏
void TimelineBrowser::updateInfo()
{
    double zoom = m_timeline->zoomFactor();
    m_infoLabel->setText(
        QString("缩放: %1x | 内容宽度: %2 px")
            .arg(zoom, 0, 'f', 1)
            .arg(m_timeline->contentWidth()));

    m_statusLabel->setText(
        QString("偏移: %1 | 缩放: %2x | 视口: %3 x %4")
            .arg(m_hScrollBar->value())
            .arg(zoom, 0, 'f', 1)
            .arg(m_timeline->width())
            .arg(m_timeline->height()));
}

/// @brief 窗口大小变化时重新配置滚动条
void TimelineBrowser::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateHorizontalScrollBar();
    updateInfo();
}
