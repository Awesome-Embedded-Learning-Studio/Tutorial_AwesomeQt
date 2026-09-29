// QtWidgets 入门示例 32: QScrollBar 滚动条
// 演示：独立使用滚动条驱动自定义控件
//       setRange / setPageStep / setSingleStep 配置
//       valueChanged 驱动内容偏移
//       QPainter::translate 偏移绘制 + 可见区域裁剪

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QColor>
#include <QString>
#include <QVector>
#include <QWidget>

// ============================================================================
// 事件数据结构
// ============================================================================
struct TimelineEvent
{
    int dayOffset;         // 相对于起始日期的天数偏移
    QString title;         // 事件标题
    QColor color;          // 卡片颜色
    int duration;          // 持续天数
};

// ============================================================================
// TimelineWidget: 自定义时间轴绘制控件
// 由外部 QScrollBar 驱动水平偏移，内部管理垂直缩放
// ============================================================================
class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget *parent = nullptr);

    /// @brief 设置水平偏移量（由外部 ScrollBar 驱动）
    void setHorizontalOffset(int offset);

    /// @brief 获取当前内容逻辑总宽度
    int contentWidth() const;

    /// @brief 获取当前缩放系数
    double zoomFactor() const;

    /// @brief 设置缩放系数，由外部 ScrollBar 驱动
    void setZoomFactor(double factor);

protected:
    void paintEvent(QPaintEvent *) override;

    /// @brief 滚轮事件转发给外部处理
    void wheelEvent(QWheelEvent *event) override;

private:
    /// @brief 初始化示例事件数据
    void initEvents();

    static constexpr int kBaseDayWidth = 30;
    static constexpr int kBaseCardHeight = 36;
    static constexpr int kPadding = 40;

    QVector<TimelineEvent> m_events;
    int m_offsetX = 0;
    double m_zoomFactor = 1.0;
    int m_dayWidth = kBaseDayWidth;
    int m_cardHeight = kBaseCardHeight;
};

#endif // TIMELINEWIDGET_H
