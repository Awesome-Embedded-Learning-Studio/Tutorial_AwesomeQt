/**
 * @file fade_animation.h
 * @brief 淡入淡出容器控件 FadeWidget——QGraphicsOpacityEffect + QPropertyAnimation
 * @copyright Copyright (c) 2026 AwesomeQt
 *
 * 一个可把自身（及其内容）整体淡入/淡出的容器。用一个 QGraphicsOpacityEffect
 * 挂在自身，由 QPropertyAnimation 驱动 effect 的 "opacity" 属性在 0↔1 间过渡，
 * 实现平滑淡入淡出。常用于通知条 / 启动画面 / 视图切换过渡。
 */
#pragma once

#include <QWidget>

class QGraphicsOpacityEffect;
class QPropertyAnimation;

namespace AwesomeQt {

/// @brief 淡入淡出容器：透明度由 QGraphicsOpacityEffect 承载，动画驱动其 opacity。
///
/// 设计要点：
/// - `opacity` 是 Q_PROPERTY，WRITE 仅做纯赋值 + 触发 update，本身不调动画；
///   fadeIn/fadeOut 才是驱动动画的入口；外部也可直接 setOpacity 绕过动画瞬时设值。
/// - `fade_anim_` 为持久成员指针（parent=this 托管），stop()/setStartValue/
///   setEndValue/start() 复用，避免连发动画时悬空（同 StatusLED 教训，禁 DeleteWhenStopped）。
/// - fadeOut 完成后默认隐藏自身（连 finished 信号），便于"消失"语义。
/// - 边界：duration<1 兜底成 1，opacity 始终夹到 [0,1]。
class FadeWidget : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：opacity / fadeDuration 均可被 Designer / 外部直接驱动 ——
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(int fadeDuration READ fadeDuration WRITE setFadeDuration NOTIFY fadeDurationChanged)

  public:
    explicit FadeWidget(QWidget* parent = nullptr);

    /// @brief 淡入：opacity 从当前值（一般 0）过渡到 1，动画结束后确保可见。
    /// @param duration_ms 过渡时长（毫秒），<1 兜底成 1。
    void fadeIn(int duration_ms = 300);

    /// @brief 淡出：opacity 从当前值过渡到 0，动画结束后隐藏自身。
    /// @param duration_ms 过渡时长（毫秒），<1 兜底成 1。
    void fadeOut(int duration_ms = 300);

    /// @brief 设置默认淡入淡出时长（毫秒），供不带参的调用使用。<1 兜底成 1。
    void setFadeDuration(int ms);
    int fadeDuration() const;

    /// @brief 当前透明度（0..1，等于挂载 effect 的 opacity）。动画运行时返回实时中间值。
    qreal opacity() const;

    /// @brief 瞬时设置透明度（不启动动画）。这是 Q_PROPERTY(opacity) 的 WRITE 回调，
    ///        供外部滑块或 QPropertyAnimation 直接驱动；调它不会触发淡入淡出动画。
    /// @param value 目标透明度，夹到 [0,1]。
    void setOpacity(qreal value);

    QSize sizeHint() const override;

  signals:
    void opacityChanged(qreal opacity);
    void fadeDurationChanged(int ms);
    /// @brief 一次淡入/淡出动画走完（无论目标值）发出。fadeOut 后由此触发 hide()。
    void fadeFinished(bool faded_out);

  private:
    /// @brief 配置并启动一次动画：从当前 opacity 接力到 end，时长 duration_ms。
    void runFade(qreal end, int duration_ms);

    QGraphicsOpacityEffect* effect_{nullptr}; // 透明度承载，构造期 new + setGraphicsEffect(this)
    QPropertyAnimation* fade_anim_{
        nullptr};               // 持久动画指针，stop/重配/start 复用，非 DeleteWhenStopped
    int fade_duration_ms_{300}; // 默认淡入淡出时长
    bool fading_out_{false};    // 标记当前是否在淡出，供 finished 回调决定是否 hide
};

} // namespace AwesomeQt
