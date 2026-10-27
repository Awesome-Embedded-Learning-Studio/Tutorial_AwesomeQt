/// @file    custom_tick_slider.h
/// @brief   演示 QSlider 进阶用法：鼠标点击直接跳转到目标值、自定义刻度标注。
///
/// 本示例展示了以下进阶知识点：
/// - 重写 mousePressEvent 实现左键点击 groove 直接跳转（默认左键是按 pageStep
/// 页步进，中键才是跳到点击处）
/// - 利用 QStyleOptionSlider 获取 groove/handle 的真实几何信息
/// - 在 groove 下方绘制百分比数值刻度标注
/// - sliderPosition vs value 的双值机制
///
/// 对应教程：进阶层 03-QtWidgets/31-QSlider 进阶。

#pragma once

#include <QSlider>

class QLabel;

/// QSlider 子类，演示点击跳转和自定义刻度标注。
///
/// 核心特性：
/// - 左键点击 groove 任意位置时，直接映射到对应数值（默认左键是按 pageStep 页步进；中键才会
///   把手柄中心移到点击位置，且映射跨度两端各留半个手柄宽的无效区域）
/// - 在 groove 下方绘制百分比刻度标注（0%、25%、50%、75%、100%）
/// - 同时显示 sliderPosition 与 value 以演示双值机制（tracking 默认开，拖拽中同步实时更新，value
/// 松手才定是 setTracking(false) 后的行为）
class CustomTickSlider : public QSlider {
    Q_OBJECT

  public:
    /// @brief 构造函数。
    /// @param[in] orientation 水平或垂直方向。
    /// @param[in] parent 父控件指针。
    explicit CustomTickSlider(Qt::Orientation orientation, QWidget* parent = nullptr);

    /// @brief 设置刻度标注的数值列表（0~1 之间的比例值）。
    /// @param[in] ratios 标注位置的比例列表，如 {0.0, 0.25, 0.5, 0.75, 1.0}。
    void setTickLabels(const QVector<double>& ratios);

  protected:
    /// @brief 重写鼠标按下事件，将点击位置直接映射为滑块值。
    ///
    /// 默认左键点击 groove 是按 pageStep 页步进；中键才是把手柄中心移到点击位置
    /// （映射范围是 groove 减去一个手柄宽的区域）。我们改为左键直接线性映射
    /// 到 [min, max]，让点击位置精确对应目标值。
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief 重写鼠标移动事件，同样使用直接映射。
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief 重写绘制事件，在基类绘制完成后额外绘制刻度标注文字。
    void paintEvent(QPaintEvent* event) override;

  private:
    /// @brief 根据鼠标位置计算对应的滑块值。
    /// @param[in] pos 鼠标在 widget 中的坐标。
    /// @return 映射后的整数值，已 clamp 到 [minimum(), maximum()]。
    int posToValue(const QPoint& pos) const;

    /// @brief 获取 groove 的有效矩形区域（通过 QStyle 查询）。
    /// @return groove 矩形，用于计算映射范围。
    QRect grooveRect() const;

    QVector<double> m_tickRatios; // 刻度标注的比例位置列表
};
