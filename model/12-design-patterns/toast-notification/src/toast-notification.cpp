/**
 * @file toast-notification.cpp
 * @brief ToastNotification 实现——无边框置顶 + 淡入淡出 + 单例堆叠管理
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "toast-notification.h"

#include <QApplication>
#include <QColor>
#include <QGuiApplication>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

namespace AwesomeQt {

// ============================================================
// Toast
// ============================================================

Toast::Toast(const QString& text, ToastType type, int duration)
    : QWidget(nullptr), text_(text), type_(type),
      duration_(duration < kMinDuration ? kMinDuration : duration) {
    // 无边框 + 工具窗口（不在任务栏占位）+ 置顶 + 不抢焦点。
    // Qt::Tool 让它不出现在任务栏；Qt::WindowDoesNotAcceptFocus 不抢键盘焦点。
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint |
                            Qt::WindowDoesNotAcceptFocus;
    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground); // 让圆角外的区域真透明
    setAttribute(Qt::WA_ShowWithoutActivating); // show 时不抢激活
    setAttribute(Qt::WA_DeleteOnClose);

    // 固定宽度，高度由 label wordWrap 撑开
    setFixedWidth(kPreferredWidth);

    label_ = new QLabel(text_, this);
    label_->setWordWrap(true);
    label_->setTextFormat(Qt::PlainText); // 防 rich text 注入
    QFont f = label_->font();
    f.setPointSize(10);
    label_->setFont(f);
    label_->setStyleSheet("QLabel { color: white; padding: 14px 18px 14px 22px; }");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label_);

    // 持久动画指针，parent=this 让对象树托管生命周期
    opacity_anim_ = new QPropertyAnimation(this, "windowOpacity", this);
    opacity_anim_->setDuration(kFadeDuration);

    display_timer_ = new QTimer(this);
    display_timer_->setSingleShot(true);
    // 到点触发淡出（淡出动画结束再 deleteLater，不在这里直接删）
    connect(display_timer_, &QTimer::timeout, this, &Toast::fadeOut);

    setWindowOpacity(0.0); // 初始透明，fadeIn 把它拉到 1
}

void Toast::fadeIn() {
    if (fading_out_) {
        return; // 已在淡出，别打断
    }
    opacity_anim_->stop();
    opacity_anim_->setStartValue(windowOpacity()); // 从当前值接力（通常是 0）
    opacity_anim_->setEndValue(1.0);
    opacity_anim_->start();
    display_timer_->start(duration_);
}

void Toast::fadeOut() {
    if (fading_out_) {
        return; // 防重入（displayTimer 超时 + 被管理器踢出 可能同时触发）
    }
    fading_out_ = true;
    display_timer_->stop();
    opacity_anim_->stop();
    opacity_anim_->setStartValue(windowOpacity());
    opacity_anim_->setEndValue(0.0);
    // 淡出结束自毁：close() 触发 WA_DeleteOnClose → deleteLater
    disconnect(opacity_anim_, &QPropertyAnimation::finished, nullptr, nullptr);
    connect(opacity_anim_, &QPropertyAnimation::finished, this, [this]() { close(); });
    opacity_anim_->start();
}

QColor Toast::baseColor() const {
    switch (type_) {
        case ToastType::kSuccess:
            return QColor(46, 125, 50); // 绿
        case ToastType::kWarning:
            return QColor(245, 124, 0); // 琥珀
        case ToastType::kError:
            return QColor(198, 40, 40); // 红
        case ToastType::kInfo:
        default:
            return QColor(55, 58, 64); // 深灰
    }
}

QColor Toast::accentColor() const {
    switch (type_) {
        case ToastType::kSuccess:
            return QColor(129, 199, 132);
        case ToastType::kWarning:
            return QColor(255, 183, 77);
        case ToastType::kError:
            return QColor(229, 115, 115);
        case ToastType::kInfo:
        default:
            return QColor(200, 203, 208);
    }
}

void Toast::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 圆角底
    QPainterPath path;
    const QRectF r = rect().adjusted(0.5, 0.5, -0.5, -0.5); // 避免 1px 抗锯齿裁切
    path.addRoundedRect(r, 8, 8);
    painter.fillPath(path, baseColor());

    // 左侧强调色条（类型标识）
    QPainterPath accent;
    QRectF accent_rect(0, 0, 5, height());
    accent.addRoundedRect(accent_rect, 2, 2);
    painter.fillPath(accent, accentColor());
}

// ============================================================
// ToastManager
// ============================================================

ToastManager* ToastManager::getInstance() {
    // 惰性单例，挂在 QCoreApplication 上，进程退出随主对象销毁
    static ToastManager* instance = []() {
        auto* m = new ToastManager();
        m->setParent(qApp);
        return m;
    }();
    return instance;
}

ToastManager::ToastManager() : QObject(nullptr) {}

void ToastManager::showToast(const QString& text, ToastType type, int duration, ToastCorner corner,
                             QWidget* parent) {
    // 屏幕可用区（排除任务栏）：优先用 parent 所在屏幕，否则主屏
    QRect available;
    QScreen* screen = nullptr;
    if (parent != nullptr && parent->windowHandle() != nullptr) {
        screen = parent->windowHandle()->screen();
    }
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen != nullptr) {
        available = screen->availableGeometry();
    } else {
        available = QRect(0, 0, 1920, 1080); // 兜底（无屏环境）
    }

    auto* toast = new Toast(text, type, duration);
    toast->adjustSize(); // 让 wordWrap 把高度撑到位

    // 入队 + 定位 + 显示
    toasts_.append(toast);
    placeToast(toast, corner, available);
    toast->show();
    toast->fadeIn();

    // Toast 自毁前回调：移出队列 + 重排同角落余下。
    // destroyed 发出时 Toast 析构已在进行，不再安全访问其成员——这里只比指针移出队列，
    // relayout 遍历的是仍存活（未进入析构）的其余 Toast，安全。
    connect(toast, &QObject::destroyed, this,
            [this, corner, toast]() { onToastDestroyed(toast, corner); });
}

void ToastManager::placeToast(Toast* toast, ToastCorner corner, const QRect& available) const {
    const int w = toast->width();
    const int h = toast->height();

    // 先算「不堆叠时」的基准锚点（屏幕角落内缩 margin）
    int x = 0;
    int y = 0;
    switch (corner) {
        case ToastCorner::kBottomRight:
            x = available.right() - w - margin_;
            y = available.bottom() - h - margin_;
            break;
        case ToastCorner::kBottomLeft:
            x = available.left() + margin_;
            y = available.bottom() - h - margin_;
            break;
        case ToastCorner::kTopRight:
            x = available.right() - w - margin_;
            y = available.top() + margin_;
            break;
        case ToastCorner::kTopLeft:
            x = available.left() + margin_;
            y = available.top() + margin_;
            break;
    }

    // 堆叠：在队列里位于本 toast 之前的同角落 toast，累计占用的 y 偏移
    // （队列顺序 = 屏幕从外到内，先入的靠边）
    int offset = 0;
    for (Toast* prev : toasts_) {
        if (prev == toast) {
            break; // 轮到自己，停
        }
        if (prev == nullptr) {
            continue;
        }
        // 简化：本实现只跟踪单一 corner 的视觉，按队列整体累计偏移
        offset += prev->height() + spacing_;
    }

    switch (corner) {
        case ToastCorner::kBottomRight:
        case ToastCorner::kBottomLeft:
            y -= offset; // 向上堆叠
            break;
        case ToastCorner::kTopRight:
        case ToastCorner::kTopLeft:
            y += offset; // 向下堆叠
            break;
    }

    toast->move(x, y);
}

void ToastManager::onToastDestroyed(Toast* toast, ToastCorner corner) {
    toasts_.removeAll(toast);
    // 重排剩余 toast（若有屏信息）
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect available = screen != nullptr ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);
    relayout(corner, available);
}

void ToastManager::relayout(ToastCorner corner, const QRect& available) const {
    // 按队列顺序（从外到内）重算每条位置；队列为空时无事可做
    int offset = 0;
    for (Toast* t : toasts_) {
        if (t == nullptr) {
            continue;
        }
        const int w = t->width();
        const int h = t->height();
        int x = 0;
        int y = 0;
        switch (corner) {
            case ToastCorner::kBottomRight:
                x = available.right() - w - margin_;
                y = available.bottom() - h - margin_ - offset;
                break;
            case ToastCorner::kBottomLeft:
                x = available.left() + margin_;
                y = available.bottom() - h - margin_ - offset;
                break;
            case ToastCorner::kTopRight:
                x = available.right() - w - margin_;
                y = available.top() + margin_ + offset;
                break;
            case ToastCorner::kTopLeft:
                x = available.left() + margin_;
                y = available.top() + margin_ + offset;
                break;
        }
        t->move(x, y);
        offset += h + spacing_;
    }
}

void ToastManager::setSpacing(int spacing) {
    spacing_ = spacing < 0 ? 0 : spacing;
}

int ToastManager::spacing() const {
    return spacing_;
}

void ToastManager::setMargin(int margin) {
    margin_ = margin < 0 ? 0 : margin;
}

int ToastManager::margin() const {
    return margin_;
}

} // namespace AwesomeQt
