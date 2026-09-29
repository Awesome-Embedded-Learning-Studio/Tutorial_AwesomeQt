/// @file    event_filter_logger.h
/// @brief   事件过滤器日志记录器——可视化 eventFilter 在事件传播链中的优先级。
///
/// 对应教程：进阶层 03-QtWidgets/02-事件处理进阶。

#pragma once

#include <QObject>

class QEvent;
class QPlainTextEdit;
class QWidget;

/// 事件过滤器日志记录器。
///
/// 作为 QObject 安装到目标控件上，拦截并记录经过的事件类型。
/// 演示事件传播链的优先级：eventFilter > event() > specific handler。
/// 日志输出到 QPlainTextEdit，方便实时观察事件到达顺序。
class EventFilterLogger : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] logOutput 日志输出控件指针（不持有所有权）。
    /// @param[in] parent 父对象指针。
    explicit EventFilterLogger(QPlainTextEdit* logOutput, QObject* parent = nullptr);

    /// @brief 将此过滤器安装到指定控件，同时开始记录。
    /// @param[in] target 目标控件。
    void installOn(QWidget* target);

protected:
    /// @brief 事件过滤器——拦截并记录事件类型。
    /// @param[in] watched 被监视的对象。
    /// @param[in] event 事件对象。
    /// @return false 始终放行，不消费任何事件。
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /// @brief 将一条事件日志追加到输出控件。
    /// @param[in] message 日志文本。
    void appendLog(const QString& message);

    QPlainTextEdit* m_logOutput;  // 日志输出目标（不持有）
    QWidget* m_target;            // 被监视的控件
};
