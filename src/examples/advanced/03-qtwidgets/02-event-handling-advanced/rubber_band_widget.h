/// @file    rubber_band_widget.h
/// @brief   演示 grabMouse() 实现橡皮筋选区，配合 EventFilterLogger 可视化事件传播链。
///
/// 对应教程：进阶层 03-QtWidgets/02-事件处理进阶。

#pragma once

#include <QPoint>
#include <QRect>
#include <QWidget>

class QLabel;

/// 橡皮筋选区演示控件。
///
/// 用户在控件上按住左键拖拽，绘制一个半透明矩形选区。
/// 演示 grabMouse() 强制捕获鼠标事件的核心机制——即使鼠标移出控件边界，
/// mouseMoveEvent 仍能正常触发，保证拖拽操作的连续性。
/// 松开鼠标后调用 releaseMouse() 释放捕获。
class RubberBandWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化控件状态。
    /// @param[in] parent 父控件指针。
    explicit RubberBandWidget(QWidget* parent = nullptr);

protected:
    /// @brief 鼠标按下——记录起点并 grabMouse。
    /// @param[in] event 鼠标事件。
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief 鼠标移动——更新选区矩形并重绘。
    /// @param[in] event 鼠标事件。
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief 鼠标释放——完成选区并 releaseMouse。
    /// @param[in] event 鼠标事件。
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// @brief 绘制橡皮筋选区矩形。
    /// @param[in] event 绘制事件。
    void paintEvent(QPaintEvent* event) override;

private:
    /// @brief 根据起点和当前位置归一化选区矩形（处理反向拖拽）。
    /// @return 归一化后的 QRect。
    QRect normalizedSelection() const;

    QPoint m_origin;       // 鼠标按下时的起始坐标
    QPoint m_current;      // 当前鼠标位置（拖拽过程中持续更新）
    bool m_isDragging;     // 是否正在拖拽选区
    QLabel* m_infoLabel;   // 底部信息提示标签
};
