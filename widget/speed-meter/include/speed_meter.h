/**
 * @file speed_meter.h
 * @brief 速度仪表盘控件 SpeedMeter——动画指针 + 主/次刻度 + 数字读数
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QColor>
#include <QWidget>

class QPropertyAnimation;

namespace AwesomeQt {

/// 速度仪表盘：value 0..maxValue（默认 220）的指针式表盘。
///
/// 设计要点（详见成品导览 index.md）：
/// - `value` 是业务属性（用户语义），`needleAngle` 是动画属性（实际绘制角度）。
///   setValue 计算 value→角度映射后，用 QPropertyAnimation 从当前角度接力到新角度，
///   指针平滑旋转；二者解耦，避免「WRITE 指 setValue→setValue 又启动画→栈溢出」（踩坑⑦）。
/// - 角度约定：表盘弧从 225°（左下）顺时针扫 270° 到 -45°（右下），开口朝下。
///   数学角度（三角函数）与 QPainter drawArc 角度（1/16°、0°=3点、正值逆时针）分别处理，
///   换算注释见 .cpp 顶部。
/// - 动画对象为持久成员指针，stop()/重配 setStartValue(当前角度)/start() 复用，
///   不用 DeleteWhenStopped（防连切悬空）。
class SpeedMeter : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：value / needleAngle / maxValue / 三色 均可被动画/Designer 驱动 ——
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(qreal needleAngle READ needleAngle WRITE setNeedleAngle NOTIFY needleAngleChanged)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)
    Q_PROPERTY(QColor needleColor READ needleColor WRITE setNeedleColor NOTIFY needleColorChanged)
    Q_PROPERTY(QColor tickColor READ tickColor WRITE setTickColor NOTIFY tickColorChanged)
    Q_PROPERTY(QColor gaugeColor READ gaugeColor WRITE setGaugeColor NOTIFY gaugeColorChanged)

  public:
    explicit SpeedMeter(QWidget* parent = nullptr);

    /// @brief 设置速度值（业务入口）：触发指针从当前角度接力到新角度的平滑旋转。
    void setValue(int value);
    int value() const;

    /// @brief 当前指针绘制角度（度，数学角度）。Q_PROPERTY(needleAngle) 的 WRITE 回调，
    ///        供 QPropertyAnimation 每帧驱动；外部业务请用 setValue，勿直接调。
    void setNeedleAngle(qreal angle);
    qreal needleAngle() const;

    /// @brief 设置量程上限（>0）。改量程后指针角度按新映射重算。
    void setMaxValue(int maxValue);
    int maxValue() const;

    void setNeedleColor(const QColor& color);
    QColor needleColor() const;

    void setTickColor(const QColor& color);
    QColor tickColor() const;

    void setGaugeColor(const QColor& color);
    QColor gaugeColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  signals:
    void valueChanged(int newValue);
    void needleAngleChanged(qreal newAngle);
    void maxValueChanged(int newMaxValue);
    void needleColorChanged(const QColor& newColor);
    void tickColorChanged(const QColor& newColor);
    void gaugeColorChanged(const QColor& newColor);

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    void initAnimation();

    /// @brief value → 数学角度（度）映射：起始角 225° 顺时针扫 270°。
    qreal angleForValue(int value) const;

    int value_{0};
    int max_value_{220};
    qreal needle_angle_{225.0}; // 初始指向最小值（225°=左下）

    QColor needle_color_{QColor(255, 70, 70)};  // 指针：红
    QColor tick_color_{QColor(60, 60, 60)};     // 刻度：深灰
    QColor gauge_color_{QColor(220, 220, 220)}; // 背景弧：浅灰

    QPropertyAnimation* needle_anim_{nullptr}; // 指针旋转（持久，非 DeleteWhenStopped）
};

} // namespace AwesomeQt
