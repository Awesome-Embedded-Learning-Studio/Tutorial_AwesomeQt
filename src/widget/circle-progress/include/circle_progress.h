/**
 * @file circle_progress.h
 * @brief 圆形进度环控件 CircleProgress——背景环 + 进度弧 + 中心百分比文字 + 平滑过渡
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QColor>
#include <QWidget>

class QPropertyAnimation;

namespace AwesomeQt {

/// 圆形进度环：value 0..100 的环形进度条。
///
/// 设计要点（详见成品导览 index.md）：
/// - `value` 是业务属性（0..100），`progress`（0.0..1.0）是动画属性（实际绘制进度）。
///   setValue 用 QPropertyAnimation 从当前 progress 接力到 value/100，弧平滑铺开；
///   二者解耦，避免「WRITE 指 setValue→setValue 又启动画→栈溢出」（踩坑③）。
/// - 进度弧从 12 点钟顺时针铺开。QPainter drawArc 角度是 1/16°、0°=3 点、正值逆时针，
///   故起始角 1440（=90°×16，12 点钟）、扫角取负（顺时针）。换算注释见 .cpp。
/// - 动画对象为持久成员指针，stop()/重配 setStartValue(当前 progress)/start() 复用，
///   不用 DeleteWhenStopped（防连切悬空）。
class CircleProgress : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：value / progress / strokeWidth / 两色 / showText 均可被动画/Designer 驱动 ——
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double progress READ progress WRITE setDisplayProgress NOTIFY progressChanged)
    Q_PROPERTY(int strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)
    Q_PROPERTY(
        QColor progressColor READ progressColor WRITE setProgressColor NOTIFY progressColorChanged)
    Q_PROPERTY(QColor ringColor READ ringColor WRITE setRingColor NOTIFY ringColorChanged)
    Q_PROPERTY(bool showText READ showText WRITE setShowText NOTIFY showTextChanged)

  public:
    explicit CircleProgress(QWidget* parent = nullptr);
    CircleProgress(int value, QWidget* parent = nullptr);

    /// @brief 设置进度值（业务入口）：触发从当前 progress 接力到 value/100 的平滑过渡。
    void setValue(int value);
    int value() const;

    /// @brief 当前绘制进度 [0.0, 1.0]。Q_PROPERTY(progress) 的 WRITE 回调，
    ///        供 QPropertyAnimation 每帧驱动；外部业务请用 setValue，勿直接调。
    void setDisplayProgress(double progress);
    double progress() const;

    /// @brief 设置进度环线宽（>0）。
    void setStrokeWidth(int width);
    int strokeWidth() const;

    void setProgressColor(const QColor& color);
    QColor progressColor() const;

    void setRingColor(const QColor& color);
    QColor ringColor() const;

    void setShowText(bool enabled);
    bool showText() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  signals:
    void valueChanged(int newValue);
    void progressChanged(double newProgress);
    void strokeWidthChanged(int newWidth);
    void progressColorChanged(const QColor& newColor);
    void ringColorChanged(const QColor& newColor);
    void showTextChanged(bool enabled);

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    void initAnimation();

    int value_{0};         // 业务值 0..100
    double progress_{0.0}; // 动画产物 0..1（实际绘制进度）
    int stroke_width_{10};
    QColor progress_color_{QColor(50, 160, 255)}; // 进度弧：蓝
    QColor ring_color_{QColor(230, 230, 230)};    // 背景环：浅灰
    bool show_text_{true};

    QPropertyAnimation* progress_anim_{nullptr}; // 进度过渡（持久，非 DeleteWhenStopped）
};

} // namespace AwesomeQt
