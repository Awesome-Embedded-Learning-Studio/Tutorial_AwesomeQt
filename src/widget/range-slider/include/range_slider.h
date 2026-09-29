/**
 * @file range_slider.h
 * @brief 双端范围滑块控件 RangeSlider——水平双柄，lower<=upper 约束，区间高亮
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QColor>
#include <QWidget>

namespace AwesomeQt {

/// 双端范围滑块：lowerValue 与 upperValue 各占一手柄，约束 lower<=upper。
///
/// 交互（照 toggle-switch 的三事件 + 拖动阈值模式）：
/// - mousePressEvent 做双手柄 hit-test，离谁近且在容差内就抓谁；
/// - mouseMoveEvent 超 kDragThreshold 才算拖（防点击被当拖），拖动即时跟手；
/// - mouseReleaseEvent 收尾重置活动手柄。
///
/// 映射：valueToX / xToValue 在 [minimum,maximum] 与轨道像素区间之间换算；
/// trackWidth<=0 时兜底，防控件过窄时除零。
class RangeSlider : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：四值 + 三配色，均可被 Designer / 动画驱动 ——
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
    Q_PROPERTY(int lowerValue READ lowerValue WRITE setLowerValue NOTIFY lowerValueChanged)
    Q_PROPERTY(int upperValue READ upperValue WRITE setUpperValue NOTIFY upperValueChanged)
    Q_PROPERTY(QColor handleColor READ handleColor WRITE setHandleColor NOTIFY handleColorChanged)
    Q_PROPERTY(QColor trackColor READ trackColor WRITE setTrackColor NOTIFY trackColorChanged)
    Q_PROPERTY(QColor rangeColor READ rangeColor WRITE setRangeColor NOTIFY rangeColorChanged)

  public:
    explicit RangeSlider(QWidget* parent = nullptr);

    int minimum() const;
    /// 设置最小值；现有值会被夹到新区间内（仍保持 lower<=upper）。
    void setMinimum(int min);

    int maximum() const;
    /// 设置最大值；现有值会被夹到新区间内（仍保持 lower<=upper）。
    void setMaximum(int max);

    /// @brief 设置区间端点 [min,max]；minimum>maximum 时自动交换。
    ///        一次性设两端，避免分别 set 时中间态非法。
    void setRange(int min, int max);

    int lowerValue() const;
    /// @brief 设置下手柄值。会夹到 [minimum, upperValue]，保持 lower<=upper。
    /// @param value 新的下手柄值。
    void setLowerValue(int value);

    int upperValue() const;
    /// @brief 设置上手柄值。会夹到 [lowerValue, maximum]，保持 lower<=upper。
    /// @param value 新的上手柄值。
    void setUpperValue(int value);

    QColor handleColor() const;
    void setHandleColor(const QColor& c);
    QColor trackColor() const;
    void setTrackColor(const QColor& c);
    QColor rangeColor() const;
    void setRangeColor(const QColor& c);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  signals:
    void minimumChanged(int min);
    void maximumChanged(int max);
    void lowerValueChanged(int value);
    void upperValueChanged(int value);
    /// 任一手柄值变动时发出（拖动过程中每步都发，便于外部即时联动）。
    void rangeChanged(int lower, int upper);
    void handleColorChanged(const QColor& c);
    void trackColorChanged(const QColor& c);
    void rangeColorChanged(const QColor& c);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

  private:
    /// 活动手柄标识：kNone 表示当前没抓任何手柄。
    enum class ActiveHandle { kNone, kLower, kUpper };

    /// 轨道可用矩形（左右各留半个手柄，让手柄圆心正好落在端点值上）。
    QRectF trackRect() const;
    /// 值 → 手柄圆心 x。
    qreal valueToX(int value) const;
    /// 鼠标 x → 值（已夹到 [minimum,maximum]）。
    int xToValue(qreal x) const;
    /// 抓取判定：返回离鼠标最近的手柄（要求在容差内，否则 kNone）。
    ActiveHandle hitTestHandle(qreal x) const;

    int minimum_{0};
    int maximum_{100};
    int lower_value_{20};
    int upper_value_{80};

    QColor handle_color_{QColor(255, 255, 255)};
    QColor track_color_{QColor(200, 200, 200)};
    QColor range_color_{QColor(0, 120, 215)};

    // 拖动状态
    ActiveHandle active_handle_{ActiveHandle::kNone};
    qreal press_x_{0}; // 按下时的鼠标 x，用于阈值判定
    bool dragging_{false};
};

} // namespace AwesomeQt
