/// @file    styled_gauge.h
/// @brief   演示 QStylePainter + QStyleOption 风格感知绘制与 heightForWidth 宽高比约束。
///
/// 对应教程：进阶层 03-QtWidgets/05-自定义控件进阶。

#pragma once

#include <QWidget>

/// 环形仪表盘控件——展示自定义控件如何融入系统风格。
///
/// 核心知识点：
/// - 使用 QStylePainter + QStyleOption 进行风格感知的文本绘制
/// - 重写 heightForWidth() 保持 1:1 宽高比约束
/// - 重写 sizeHint() / minimumSizeHint() 向布局系统提供建议尺寸
/// - 从系统 palette 获取颜色而非硬编码
class StyledGauge : public QWidget
{
    Q_OBJECT

    /// 进度值属性，支持 QPropertyAnimation 驱动平滑动画
    Q_PROPERTY(int progress READ progress WRITE setProgress NOTIFY progressChanged)

public:
    /// @brief 构造函数，初始化界面与属性。
    /// @param[in] parent 父控件指针。
    explicit StyledGauge(QWidget* parent = nullptr);

    /// @brief 获取当前进度值。
    /// @return 进度百分比，范围 [0, 100]。
    int progress() const;

    /// @brief 设置当前进度值。
    /// @param[in] value 进度百分比，范围 [0, 100]，超出范围会被钳位。
    void setProgress(int value);

    /// @brief 控件的建议尺寸，布局系统未约束时使用此值。
    /// @return 固定返回 (120, 120)，作为默认正方形大小。
    QSize sizeHint() const override;

    /// @brief 控件的最小建议尺寸。
    /// @return 固定返回 (60, 60)，保证弧线在小尺寸下仍然可辨。
    QSize minimumSizeHint() const override;

    /// @brief 根据给定宽度计算理想高度，保持 1:1 宽高比。
    /// @param[in] w 布局系统分配的宽度。
    /// @return 与宽度相等的高度值，实现正方形约束。
    /// @note 必须配合 QSizePolicy::setHeightForWidth(true) 才能生效。
    int heightForWidth(int w) const override;

signals:
    /// @brief 进度值变化时发射。
    /// @param value 新的进度值。
    void progressChanged(int value);

protected:
    /// @brief 绘制事件——使用 QStylePainter 绘制环形进度条和文本。
    /// @param[in] event 绘制事件参数。
    void paintEvent(QPaintEvent* event) override;

private:
    int m_progress;    // 当前进度，范围 [0, 100]
};
