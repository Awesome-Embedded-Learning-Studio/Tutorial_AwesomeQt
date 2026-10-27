/**
 * @file toggle_switch.h
 * @brief 自绘滑动开关控件：点击或拖动切换，滑块滑动 + 轨道变色过渡
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QWidget>

class QPropertyAnimation;

namespace AwesomeQt {

/// 自绘滑动开关。
/// `checked` 是逻辑状态，`handlePos`（0=关/左，1=开/右）是动画属性，
/// 由 QPropertyAnimation 驱动——切换时滑块滑动，轨道颜色随之过渡。
/// 支持点击切换，也支持按住拖动。
class ToggleSwitch : public QWidget {
    Q_OBJECT
    // —— Q_PROPERTY：checked / handlePos / 轨道两色 均可被动画/Designer 驱动 ——
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY toggled)
    Q_PROPERTY(qreal handlePos READ handlePos WRITE setHandlePos NOTIFY handlePosChanged)
    Q_PROPERTY(QColor trackColorOn READ trackColorOn WRITE setTrackColorOn NOTIFY trackColorOnChanged)
    Q_PROPERTY(QColor trackColorOff READ trackColorOff WRITE setTrackColorOff NOTIFY trackColorOffChanged)

  public:
    explicit ToggleSwitch(QWidget* parent = nullptr);
    ToggleSwitch(bool checked, QWidget* parent = nullptr);

    bool checked() const;
    /// 设置开关状态，会触发滑块滑动动画。
    void setChecked(bool checked);

    /// @brief 滑块位置 [0,1]（0=关，1=开）。Q_PROPERTY(handlePos) 的 WRITE 回调，
    ///        供 QPropertyAnimation 每帧驱动；外部业务请用 setChecked，勿直接调。
    qreal handlePos() const;
    void setHandlePos(qreal pos);

    QColor trackColorOn() const;
    void setTrackColorOn(const QColor& c);
    QColor trackColorOff() const;
    void setTrackColorOff(const QColor& c);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  signals:
    void toggled(bool checked);
    void handlePosChanged(qreal pos);
    void trackColorOnChanged(const QColor& c);
    void trackColorOffChanged(const QColor& c);

  protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

  private:
    void init_animation();
    void animate_to(bool checked);
    QRectF track_rect() const;
    qreal handle_range() const;   // 滑块可移动的水平行程（像素），可能为 0 或负
    qreal x_to_pos(qreal x) const;

    bool checked_{false};
    qreal handle_pos_{0.0};            // 实际绘制位置（动画产物），0=关 1=开
    QColor track_on_{QColor(0, 170, 0)};     // 开：绿
    QColor track_off_{QColor(190, 190, 190)}; // 关：灰

    QPropertyAnimation* handle_anim_{nullptr};

    // 拖动状态
    bool dragging_{false};
    qreal drag_start_pos_{0.0};
    qreal press_x_{0};
};

} // namespace AwesomeQt
