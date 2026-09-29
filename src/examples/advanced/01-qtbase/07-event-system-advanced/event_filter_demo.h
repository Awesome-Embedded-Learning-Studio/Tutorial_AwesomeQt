/// @file    event_filter_demo.h
/// @brief   全局事件过滤器与局部事件过滤器的声明，演示 eventFilter 拦截机制。
///
/// 对应教程：进阶层 01-QtBase/07-事件系统进阶。

#pragma once

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>

// ---------------------------------------------------------------------------
// 全局事件过滤器
// ---------------------------------------------------------------------------
/// @brief 全局事件过滤器，可安装在 QApplication 或任何 QObject 上拦截事件。
/// @note 返回 true 表示事件已处理（不再传递），返回 false 表示继续传递。
class GlobalEventFilter : public QObject {
    Q_OBJECT
public:
    /// @brief 构造全局事件过滤器。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit GlobalEventFilter(QObject* parent = nullptr);

protected:
    /// @brief 重写 eventFilter，在事件到达目标对象之前拦截。
    /// @param[in] watched 被监视的对象指针。
    /// @param[in] event   待拦截的事件指针。
    /// @return true 表示事件已处理不再传递，false 表示继续传递。
    bool eventFilter(QObject* watched, QEvent* event) override;
};

// ---------------------------------------------------------------------------
// 带有局部事件过滤器的对象
// ---------------------------------------------------------------------------
/// @brief 在自身上安装事件过滤器的对象，演示自我过滤和 event() 重写。
class FilteredObject : public QObject {
    Q_OBJECT
public:
    /// @brief 构造过滤对象，并在构造函数中为自身安装事件过滤器。
    /// @param[in] parent 父对象指针。
    /// @note installEventFilter(this) 使自身既是监视者也是被监视者。
    explicit FilteredObject(QObject* parent = nullptr);

protected:
    /// @brief 重写 event()，在事件分发的统一入口处理键盘事件。
    /// @param[in] event 待处理的事件指针。
    /// @return true 表示事件已被处理。
    /// @note QObject 没有 keyPressEvent，需要通过 event() 或 eventFilter 处理。
    bool event(QEvent* event) override;

    /// @brief 自身的 eventFilter，先于 event() 被调用。
    /// @param[in] watched 被监视的对象（这里是自身）。
    /// @param[in] event   待拦截的事件指针。
    /// @return false 以让事件继续传递到 event()。
    bool eventFilter(QObject* watched, QEvent* event) override;
};

// ---------------------------------------------------------------------------
// 演示函数
// ---------------------------------------------------------------------------

/// @brief 演示事件过滤器的安装、拦截与移除。
/// @note 展示事件传播顺序：全局过滤器 → 局部过滤器 → event() → 具体处理器。
void demoEventFilter();
