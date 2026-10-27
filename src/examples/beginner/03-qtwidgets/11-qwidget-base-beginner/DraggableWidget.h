#pragma once

#include <QWidget>

class QLabel;

// ============================================================================
// DraggableWidget: 支持无边框拖拽的自定义 QWidget
// ============================================================================
class DraggableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DraggableWidget(QWidget *parent = nullptr);

protected:
    /// @brief 绘制背景（无边框模式下需要手动绘制）
    void paintEvent(QPaintEvent *) override;

    /// @brief 无边框模式下的拖拽支持：鼠标按下
    void mousePressEvent(QMouseEvent *event) override;

    /// @brief 无边框模式下的拖拽支持：鼠标移动
    void mouseMoveEvent(QMouseEvent *event) override;

    /// @brief 无边框模式下的拖拽支持：鼠标释放
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新尺寸信息标签
    void updateSizeLabel();

private:
    QLabel *m_infoPanel = nullptr;
    QLabel *m_backLabel = nullptr;
    QLabel *m_sizeLabel = nullptr;
    QPoint m_dragPosition;
    bool m_dragging = false;
};
