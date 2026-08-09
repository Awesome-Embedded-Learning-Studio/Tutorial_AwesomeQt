/**
 * @file toast-notification.h
 * @brief ToastNotification——仿 Material Toast 临时提示气泡（无边框置顶 + 淡入淡出 + 多条堆叠）
 * @copyright Copyright (c) 2026 AwesomeQt
 *
 * 教学要点（model 栏设计模式实例，集中体现 5 个 Qt 能力）：
 * - 无边框置顶窗口（Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint），不抢焦点
 * - QPropertyAnimation 淡入淡出（驱动 windowOpacity）
 * - QTimer::singleShot 定时后 deleteLater 自毁
 * - 单例 ToastManager 维护活动队列，按屏幕右下角向上堆叠不重叠
 * - 类型分级（kSuccess / kWarning / kError）+ 配色
 */
#pragma once

#include <QList>
#include <QObject>
#include <QRect>
#include <QString>
#include <QWidget>

class QColor;
class QLabel;
class QPropertyAnimation;
class QTimer;

namespace AwesomeQt {

/// @brief Toast 类型分级，决定配色与左侧色条。
enum class ToastType {
    kInfo,    ///< 普通：深灰底
    kSuccess, ///< 成功：绿
    kWarning, ///< 警告：琥珀
    kError    ///< 错误：红
};

/// @brief Toast 出现的屏幕角落（堆叠方向相对它）。
enum class ToastCorner {
    kBottomRight, ///< 右下角，新条向上堆叠（默认，最常见）
    kBottomLeft,  ///< 左下角，新条向上堆叠
    kTopRight,    ///< 右上角，新条向下堆叠
    kTopLeft      ///< 左上角，新条向下堆叠
};

class ToastManager;

/// @brief 单条 Toast 气泡窗口：无边框置顶、淡入淡出、定时自毁。
///
/// 范式要点：
/// - 继承 QWidget，构造期用窗口标志位组合实现「无边框 + 工具窗口 + 置顶 + 不抢焦点」；
/// - `windowOpacity` 是 QWidget 自带属性，QPropertyAnimation 直接驱动它做淡入淡出，
///   无需 QGraphicsOpacityEffect（windowOpacity 作用于整个顶层窗口，含圆角与半透明）；
/// - 生命周期由 QTimer::singleShot + deleteLater 闭环，**无父对象**（独立顶层窗口），
///   自毁前通知 ToastManager 释放占位、重排队列；
/// - 不要直接 new Toast 用——走 ToastManager::showToast，否则堆叠定位无人管。
class Toast : public QWidget {
    Q_OBJECT

  public:
    /// @brief 构造一条 Toast（位置由 ToastManager 在 show 前定位）。
    /// @param[in] text     正文文本。
    /// @param[in] type     类型分级，决定底色与左侧色条。
    /// @param[in] duration 显示时长（毫秒），到点后淡出自毁。<kMinDuration 兜底成 kMinDuration。
    explicit Toast(const QString& text, ToastType type = ToastType::kInfo, int duration = 3000);

    /// @brief 显示时长下限（毫秒）。低于它会出现「闪一下就消失」的坏体验，构造期兜底。
    static constexpr int kMinDuration = 500;

    /// @brief 淡入淡出动画时长（毫秒），固定常量，不对外暴露配置。
    static constexpr int kFadeDuration = 250;

    /// @brief 期望宽度（像素），超出则按 wordWrap 自动增高。
    static constexpr int kPreferredWidth = 320;

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    friend class ToastManager; // 管理器需读 私有几何/动画 入口做堆叠

    /// @brief 启动淡入动画（构造 + show 后由 ToastManager 调）。
    void fadeIn();

    /// @brief 启动淡出动画，结束后 deleteLater 自毁。displayTimer 超时或被踢出队列时调。
    void fadeOut();

    /// @brief 当前类型对应的底色。
    QColor baseColor() const;

    /// @brief 当前类型对应的左侧强调色条颜色。
    QColor accentColor() const;

    QString text_;
    ToastType type_;
    int duration_;
    QLabel* label_{nullptr};
    QPropertyAnimation* opacity_anim_{nullptr}; // 持久指针，stop/重配/start 复用
    QTimer* display_timer_{nullptr};            // 到点触发 fadeOut
    bool fading_out_{false};                    // 防重复进入 fadeOut
};

/// @brief Toast 单例管理器：维护活动队列，按屏幕角落堆叠定位新 Toast。
///
/// 设计要点：
/// - 单例（getInstance），全局唯一队列，保证多条 Toast 位置不重叠；
/// - 每次 showToast 算出新 Toast 的目标矩形（基于活动队列 + 屏幕可用区 + 角落 + 间距），
///   调 move/show/fadeIn；Toast 自毁前回调 onToastDestroyed 把它移出队列并重排余下；
/// - 队列顺序 = 屏幕从外（先入）到内（后入）的视觉顺序，重排时按此顺序重算 y。
class ToastManager : public QObject {
    Q_OBJECT

  public:
    /// @brief 获取单例（首次调用惰性创建，挂在 QCoreApplication 上随进程退出销毁）。
    static ToastManager* getInstance();

    /// @brief 显示一条 Toast（自动堆叠定位 + 淡入）。
    /// @param[in] text     正文。
    /// @param[in] type     类型分级。
    /// @param[in] duration 显示时长（毫秒）。
    /// @param[in] corner   出现角落，默认右下角。
    /// @param[in] parent   用作屏幕可用区参照的窗口（nullptr 则取主屏 availableGeometry）。
    void showToast(const QString& text, ToastType type = ToastType::kInfo, int duration = 3000,
                   ToastCorner corner = ToastCorner::kBottomRight, QWidget* parent = nullptr);

    /// @brief 配置项：Toast 之间的垂直间距（像素）。
    void setSpacing(int spacing);
    int spacing() const;

    /// @brief 配置项：Toast 离屏幕边缘的留白（像素）。
    void setMargin(int margin);
    int margin() const;

  private:
    ToastManager();

    /// @brief 计算并应用一条新 Toast 的目标位置（基于当前活动队列）。
    void placeToast(Toast* toast, ToastCorner corner, const QRect& available) const;

    /// @brief Toast 自毁前回调：移出队列，重排同角落余下 Toast。
    void onToastDestroyed(Toast* toast, ToastCorner corner);

    /// @brief 重排某角落的所有活动 Toast（按队列顺序重算位置）。
    void relayout(ToastCorner corner, const QRect& available) const;

    QList<Toast*> toasts_; // 活动队列（按入队顺序，即屏幕从外到内的视觉顺序）
    int spacing_{12};
    int margin_{24};

    friend class Toast; // Toast 自毁时回调本类的私有 onToastDestroyed
};

} // namespace AwesomeQt
