// QtWidgets 入门示例 14: QAbstractScrollArea 滚动区域基类
// 演示：horizontalScrollBar() / verticalScrollBar() 滚动条控制
//       setHorizontalScrollBarPolicy / setVerticalScrollBarPolicy
//       scrollContentsBy() 内容滚动响应
//       视口（viewport）概念与绘制注意事项

#pragma once

#include <QAbstractScrollArea>

// ============================================================================
// TimelineScrollArea: 继承 QAbstractScrollArea 的时间轴控件
// ============================================================================
class TimelineScrollArea : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit TimelineScrollArea(QWidget *parent = nullptr);

    /// @brief 获取当前滚动位置对应的时间单位
    [[nodiscard]] double currentTimeUnit() const;

protected:
    /// @brief 视口大小变化时更新滚动条参数
    void resizeEvent(QResizeEvent *event) override;

    /// @brief 内容滚动时的回调
    void scrollContentsBy(int dx, int dy) override;

    /// @brief 在视口上绘制时间轴内容
    void paintEvent(QPaintEvent *event) override;

private:
    /// @brief 更新垂直滚动条的范围和 pageStep
    void updateScrollBarRange();

    static constexpr int kLabelWidth = 70;
    int kPixelsPerUnit;
    int kTotalUnits;
    int m_contentHeight;
};
