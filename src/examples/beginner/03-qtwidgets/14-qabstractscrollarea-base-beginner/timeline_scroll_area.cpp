// QtWidgets 入门示例 14: QAbstractScrollArea 滚动区域基类
// 演示：horizontalScrollBar() / verticalScrollBar() 滚动条控制
//       setHorizontalScrollBarPolicy / setVerticalScrollBarPolicy
//       scrollContentsBy() 内容滚动响应
//       视口（viewport）概念与绘制注意事项

#include "timeline_scroll_area.h"

#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>

// ============================================================================
// TimelineScrollArea: 继承 QAbstractScrollArea 的时间轴控件
// ============================================================================
TimelineScrollArea::TimelineScrollArea(QWidget *parent)
    : QAbstractScrollArea(parent)
{
    // 每个时间单位占 20 像素，总共 100 个时间单位
    kPixelsPerUnit = 20;
    kTotalUnits = 100;
    m_contentHeight = kTotalUnits * kPixelsPerUnit;

    // 滚动条配置
    verticalScrollBar()->setSingleStep(kPixelsPerUnit);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    setMinimumHeight(200);
}

/// @brief 获取当前滚动位置对应的时间单位
double TimelineScrollArea::currentTimeUnit() const
{
    int value = verticalScrollBar()->value();
    return static_cast<double>(value) / kPixelsPerUnit;
}

/// @brief 视口大小变化时更新滚动条参数
void TimelineScrollArea::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBarRange();
}

/// @brief 内容滚动时的回调
void TimelineScrollArea::scrollContentsBy(int dx, int dy)
{
    // 调用基类实现：利用位块传输优化滚动
    QAbstractScrollArea::scrollContentsBy(dx, dy);
    // 刷新视口以绘制新暴露的区域
    viewport()->update();
}

/// @brief 在视口上绘制时间轴内容
void TimelineScrollArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    int scrollOffset = verticalScrollBar()->value();
    int viewWidth = viewport()->width();
    int viewHeight = viewport()->height();

    // 背景色
    painter.fillRect(viewport()->rect(), QColor("#FAFAFA"));

    // 平移坐标系，使得绘制内容随滚动偏移
    painter.translate(0, -scrollOffset);

    // 绘制刻度区域背景
    QRect timelineRect(0, 0, kLabelWidth, m_contentHeight);
    painter.fillRect(timelineRect, QColor("#ECEFF1"));

    // 绘制时间轴刻度线和标签
    for (int unit = 0; unit <= kTotalUnits; ++unit) {
        int y = unit * kPixelsPerUnit;
        bool isMajor = (unit % 10 == 0);
        bool isMinor = (unit % 2 == 0);

        if (isMajor) {
            // 主刻度线
            painter.setPen(QPen(QColor("#37474F"), 2));
            painter.drawLine(kLabelWidth, y, viewWidth, y);

            // 主刻度标签
            painter.setPen(QColor("#263238"));
            painter.setFont(QFont("Arial", 9, QFont::Bold));
            QRect labelRect(4, y - 12, kLabelWidth - 8, 24);
            painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter,
                QString("T=%1").arg(unit));
        } else if (isMinor) {
            // 次刻度线
            painter.setPen(QPen(QColor("#B0BEC5"), 1, Qt::DashLine));
            painter.drawLine(kLabelWidth + 20, y, viewWidth, y);

            // 次刻度标签
            painter.setPen(QColor("#78909C"));
            painter.setFont(QFont("Arial", 8));
            QRect labelRect(16, y - 10, kLabelWidth - 24, 20);
            painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter,
                QString::number(unit));
        }
    }

    // 绘制一些示例事件标记
    struct Event {
        int time;
        QString name;
        QColor color;
    };
    Event events[] = {
        {5,  "初始化",     QColor("#4CAF50")},
        {15, "加载数据",   QColor("#2196F3")},
        {25, "处理中",     QColor("#FF9800")},
        {40, "渲染完成",   QColor("#9C27B0")},
        {55, "用户输入",   QColor("#F44336")},
        {70, "网络请求",   QColor("#00BCD4")},
        {85, "保存结果",   QColor("#795548")},
        {95, "清理资源",   QColor("#607D8B")},
    };

    painter.setFont(QFont("Arial", 10));
    for (const auto &evt : events) {
        int y = evt.time * kPixelsPerUnit;

        // 事件标记圆点
        painter.setBrush(evt.color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(kLabelWidth + 35, y), 6, 6);

        // 事件名称
        painter.setPen(evt.color);
        QRect textRect(kLabelWidth + 48, y - 10, 200, 20);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, evt.name);

        // 事件到时间轴的连线
        painter.setPen(QPen(evt.color, 1, Qt::DotLine));
        painter.drawLine(kLabelWidth, y, kLabelWidth + 29, y);
    }

    // 绘制时间轴分隔线
    painter.setPen(QPen(QColor("#90A4AE"), 1));
    painter.drawLine(kLabelWidth, 0, kLabelWidth, m_contentHeight);

    // 绘制视口底部渐变遮罩（视觉提示：还有更多内容）
    // 这里用简单的半透明矩形实现
    painter.resetTransform();
    if (scrollOffset + viewHeight < m_contentHeight) {
        QLinearGradient bottomFade(0, viewHeight - 30, 0, viewHeight);
        bottomFade.setColorAt(0.0, QColor(250, 250, 250, 0));
        bottomFade.setColorAt(1.0, QColor(250, 250, 250, 180));
        painter.fillRect(0, viewHeight - 30, viewWidth, 30, bottomFade);
    }
    if (scrollOffset > 0) {
        QLinearGradient topFade(0, 0, 0, 30);
        topFade.setColorAt(0.0, QColor(250, 250, 250, 180));
        topFade.setColorAt(1.0, QColor(250, 250, 250, 0));
        painter.fillRect(0, 0, viewWidth, 30, topFade);
    }
}

/// @brief 更新垂直滚动条的范围和 pageStep
void TimelineScrollArea::updateScrollBarRange()
{
    int visibleHeight = viewport()->height();
    int maxScroll = qMax(0, m_contentHeight - visibleHeight);
    verticalScrollBar()->setRange(0, maxScroll);
    verticalScrollBar()->setPageStep(visibleHeight);
}
