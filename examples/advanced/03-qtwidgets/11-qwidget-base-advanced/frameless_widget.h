/// @file    frameless_widget.h
/// @brief   无边框半透明窗口控件声明——演示 WA_TranslucentBackground 与手动拖动。
///
/// 对应教程：进阶层 03-QtWidgets/11-QWidget 基类进阶。

#pragma once

#include <QWidget>

/// 无边框半透明窗口，演示 QWidget 的 WA_* 属性三角与手动鼠标拖动。
///
/// 核心知识点：
/// - WA_TranslucentBackground + WA_NoSystemBackground 实现半透明背景
/// - Qt::FramelessWindowHint 去掉系统边框和标题栏
/// - mousePressEvent + mouseMoveEvent 手动实现窗口拖动
/// - paintEvent 自定义绘制圆角半透明背景
class FramelessWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，配置无边框窗口属性与界面布局。
    /// @param[in] parent 父控件指针。
    explicit FramelessWidget(QWidget* parent = nullptr);

protected:
    /// @brief 重写绘制——用 QPainter 绘制圆角半透明背景。
    /// @param[in] event 绘制事件。
    void paintEvent(QPaintEvent* event) override;

    /// @brief 记录鼠标按下时的偏移量，用于后续拖动计算。
    /// @param[in] event 鼠标事件。
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief 根据鼠标移动偏移量更新窗口位置。
    /// @param[in] event 鼠标事件。
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPoint m_dragPosition;  // 拖动偏移量：鼠标全局坐标 - 窗口左上角坐标
};
