// QtWidgets 入门示例 14: QAbstractScrollArea 滚动区域基类
// 演示：horizontalScrollBar() / verticalScrollBar() 滚动条控制
//       setHorizontalScrollBarPolicy / setVerticalScrollBarPolicy
//       scrollContentsBy() 内容滚动响应
//       视口（viewport）概念与绘制注意事项

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class TimelineScrollArea;

// ============================================================================
// DemoWindow: 主演示窗口
// ============================================================================
class DemoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DemoWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新位置标签
    void updatePositionLabel();

private:
    TimelineScrollArea *m_timeline = nullptr;
    QLabel *m_positionLabel = nullptr;
    QPushButton *m_policyBtn = nullptr;
    bool m_policyToggle = false;
};
