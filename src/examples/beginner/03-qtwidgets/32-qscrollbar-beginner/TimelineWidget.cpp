#include "TimelineWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>

// ============================================================================
// TimelineWidget: 自定义时间轴绘制控件
// 由外部 QScrollBar 驱动水平偏移，内部管理垂直缩放
// ============================================================================
TimelineWidget::TimelineWidget(QWidget *parent)
    : QWidget(parent)
{
    initEvents();
    setMinimumHeight(300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

/// @brief 设置水平偏移量（由外部 ScrollBar 驱动）
void TimelineWidget::setHorizontalOffset(int offset)
{
    m_offsetX = offset;
    update();
}

/// @brief 获取当前内容逻辑总宽度
int TimelineWidget::contentWidth() const
{
    if (m_events.isEmpty()) {
        return 0;
    }
    int lastEnd = 0;
    for (const auto &evt : m_events) {
        int end = (evt.dayOffset + evt.duration) * m_dayWidth + kPadding * 2;
        if (end > lastEnd) {
            lastEnd = end;
        }
    }
    return lastEnd;
}

/// @brief 获取当前缩放系数
double TimelineWidget::zoomFactor() const
{
    return m_zoomFactor;
}

/// @brief 设置缩放系数，由外部 ScrollBar 驱动
void TimelineWidget::setZoomFactor(double factor)
{
    m_zoomFactor = qBound(0.5, factor, 3.0);
    m_dayWidth = static_cast<int>(kBaseDayWidth * m_zoomFactor);
    m_cardHeight = static_cast<int>(kBaseCardHeight * m_zoomFactor);
    update();
}

void TimelineWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 填充背景
    painter.fillRect(rect(), QColor("#FAFAFA"));

    // 绘制时间轴基线（固定位置，不随水平滚动移动）
    int timelineY = height() - 60;
    painter.setPen(QPen(QColor("#BDBDBD"), 2));
    painter.drawLine(0, timelineY, width(), timelineY);

    // 绘制日期标记
    painter.setFont(QFont("monospace", 8));
    painter.setPen(QColor("#999"));
    int dayStart = m_offsetX / m_dayWidth;
    int dayEnd = (m_offsetX + width()) / m_dayWidth + 1;

    for (int day = dayStart; day <= dayEnd; ++day) {
        int x = day * m_dayWidth - m_offsetX + kPadding;
        if (x < -m_dayWidth || x > width() + m_dayWidth) {
            continue;
        }

        // 日期刻度线
        painter.setPen(QPen(QColor("#E0E0E0"), 1));
        painter.drawLine(x, timelineY - 5, x, timelineY + 5);

        // 日期标签（每 5 天显示一次）
        if (day % 5 == 0) {
            painter.setPen(QColor("#999"));
            painter.drawText(x - 15, timelineY + 20,
                             QString("Day %1").arg(day));
        }
    }

    // 绘制事件卡片（只绘制可见的）
    int visibleLeft = m_offsetX;
    int visibleRight = m_offsetX + width();

    for (int i = 0; i < m_events.size(); ++i) {
        const auto &evt = m_events[i];
        int cardX = evt.dayOffset * m_dayWidth + kPadding;
        int cardW = evt.duration * m_dayWidth;
        int cardRight = cardX + cardW;

        // 裁剪不可见的卡片
        if (cardRight < visibleLeft || cardX > visibleRight) {
            continue;
        }

        int cardY = 20 + (i % 3) * (m_cardHeight + 12);
        int drawX = cardX - m_offsetX;

        // 卡片阴影
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 30));
        painter.drawRoundedRect(drawX + 2, cardY + 2,
                                cardW, m_cardHeight, 6, 6);

        // 卡片主体
        painter.setBrush(evt.color);
        painter.drawRoundedRect(drawX, cardY,
                                cardW, m_cardHeight, 6, 6);

        // 卡片标题
        painter.setPen(Qt::white);
        painter.setFont(QFont("sans-serif",
                              static_cast<int>(10 * m_zoomFactor)));
        QRect textRect(drawX + 8, cardY + 4,
                       cardW - 16, m_cardHeight - 8);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                         evt.title);
    }
}

/// @brief 滚轮事件转发给外部处理
void TimelineWidget::wheelEvent(QWheelEvent *event)
{
    event->ignore();  // 让父组件处理
}

/// @brief 初始化示例事件数据
void TimelineWidget::initEvents()
{
    m_events = {
        {0,  "项目启动", QColor("#1976D2"), 3},
        {2,  "需求分析", QColor("#388E3C"), 5},
        {5,  "UI 设计",  QColor("#F57C00"), 4},
        {8,  "后端开发", QColor("#7B1FA2"), 10},
        {10, "前端开发", QColor("#C62828"), 8},
        {15, "联调测试", QColor("#00838F"), 6},
        {18, "性能优化", QColor("#AD1457"), 4},
        {20, "文档编写", QColor("#4E342E"), 5},
        {22, "部署上线", QColor("#1565C0"), 2},
        {25, "运维监控", QColor("#2E7D32"), 8},
        {28, "迭代 v2",  QColor("#E65100"), 10},
        {32, "代码审查", QColor("#6A1B9A"), 3},
        {35, "发布 v2",  QColor("#283593"), 2},
    };
}
