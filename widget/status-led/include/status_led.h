/**
 * @file status_led.h
 * @brief 状态指示灯控件 StatusLED——四态颜色平滑过渡 + 可配置闪烁(OnOff/呼吸)
 * @copyright Copyright (c) 2026
 */
#pragma once

#include <QColor>
#include <QWidget>

class QPropertyAnimation;
class QTimer;
class QVariantAnimation;

namespace AwesomeQt {

/// @brief 状态指示灯：状态切换走颜色平滑过渡，闪烁支持 None/OnOff/Breathing。
///
/// 设计要点（详见成品导览 index.md）：
/// - `color` 是 Q_PROPERTY，由 QPropertyAnimation 驱动，状态切换时 300ms OutCubic 过渡；
/// - `current_color_`（过渡产物）与 `breathing_factor_`（呼吸产物）在 paintEvent
///   经 applyDisplayTransform 解耦合成，二者正交、可并行；
/// - `color_anim_` 为持久成员指针，stop()/重配/start() 复用，避免连切悬空。
class StatusLED : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：status / color / blinkMode / ledSize 均可被动画/Designer 驱动 ——
    Q_PROPERTY(Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QColor color READ color WRITE setAnimatedColor NOTIFY colorChanged)
    Q_PROPERTY(BlinkMode blinkMode READ blinkMode WRITE setBlinkMode NOTIFY blinkModeChanged)
    Q_PROPERTY(int ledSize READ ledSize WRITE setLedSize NOTIFY ledSizeChanged)

  public:
    /// @brief 状态枚举
    enum class Status { NORMAL, WARNING, ERROR, OFFLINE };
    Q_ENUM(Status)

    /// @brief 闪烁模式：None 不闪 / OnOff 生硬明灭 / Breathing 正弦呼吸
    enum class BlinkMode { None, OnOff, Breathing };
    Q_ENUM(BlinkMode)

    explicit StatusLED(QWidget* parent = nullptr);
    StatusLED(Status default_status, QWidget* parent = nullptr);

    /// @brief 切换状态，触发颜色平滑过渡动画（从当前显示色接力到新状态色）
    void setStatus(Status new_status);
    Status status() const;

    /// @brief 当前显示色（含过渡中间值）。这是 Q_PROPERTY(color) 的 WRITE 回调，
    ///        供 QPropertyAnimation 每帧驱动；外部业务请用 setStatus，勿直接调。
    void setAnimatedColor(const QColor& color);
    QColor color() const;

    /// @brief 设置闪烁模式（切换会先停掉所有闪烁动画，再按模式启对应）
    void setBlinkMode(BlinkMode mode);
    BlinkMode blinkMode() const;

    /// @brief 兼容薄层：enabled → OnOff，!enabled → None
    void setBlinking(bool enabled);
    bool isBlinking() const;

    void setLedSize(int diameter);
    int ledSize() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  signals:
    void statusChanged(Status newStatus);
    void colorChanged(const QColor& newColor);
    void blinkModeChanged(BlinkMode newMode);
    void ledSizeChanged(int newSize);

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    void initColorAnimation();
    void initBreathingAnimation();
    void initBlinkTimer();

    /// @brief 取某状态的代表色（替代裸数组索引）
    QColor statusColor(Status s) const;
    /// @brief 把显示色按闪烁模式变换：OnOff 熄灭 / Breathing 亮度调制
    QColor applyDisplayTransform(const QColor& base) const;

    Status status_{Status::OFFLINE};
    QColor current_color_;                  // 实际绘制色（过渡产物 + 闪烁变换前的基色）
    int led_size_{20};
    BlinkMode blink_mode_{BlinkMode::None};

    QPropertyAnimation* color_anim_{nullptr};     // 颜色过渡（持久，非 DeleteWhenStopped）
    QVariantAnimation* breathing_anim_{nullptr};  // 呼吸（LoopInfinite, InOutSine）
    QTimer* onoff_timer_{nullptr};                // OnOff 生硬明灭
    bool onoff_visible_{true};
    double breathing_factor_{0.0};                // 0..1，由 breathing_anim 回调写
};

} // namespace AwesomeQt
