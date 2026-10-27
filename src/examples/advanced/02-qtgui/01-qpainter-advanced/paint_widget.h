/// @file    paint_widget.h
/// @brief   演示 QPainter 双缓冲、合成模式和抗锯齿的自定义绘制控件。
///
/// 对应教程：进阶层 02-QtGui/01-QPainter 进阶：双缓冲、合成模式、抗锯齿。

#pragma once

#include <QWidget>

/// 双缓冲绘制演示控件。
///
/// 展示 QPainter 的三种进阶特性：
/// - 双缓冲绘制（在 QPixmap 上绘制后一次性 blit 到屏幕）
/// - CompositionMode 合成模式叠加效果
/// - RenderHint 抗锯齿与平滑变换选项
class PaintWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化双缓冲画布。
    /// @param[in] parent 父控件指针。
    explicit PaintWidget(QWidget* parent = nullptr);

protected:
    /// @brief 双缓冲绘制入口：先在 m_buffer 上绘制，再 blit 到屏幕。
    void paintEvent(QPaintEvent* event) override;

    /// @brief 窗口大小变化时重建画布。
    void resizeEvent(QResizeEvent* event) override;

private:
    /// @brief 在离屏画布上绘制所有演示内容。
    void renderToBuffer();

    /// @brief 绘制抗锯齿对比演示。
    void drawAntialiasDemo(QPainter& painter);

    /// @brief 绘制合成模式演示。
    void drawCompositionDemo(QPainter& painter);

    /// @brief 绘制渐变填充演示。
    void drawGradientDemo(QPainter& painter);

    QPixmap m_buffer;    // 双缓冲离屏画布
};
