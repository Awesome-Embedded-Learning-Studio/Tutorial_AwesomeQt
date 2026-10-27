/// @file    event_filter_logger.cpp
/// @brief   EventFilterLogger 类实现——事件过滤器日志记录。
///
/// 对应教程：进阶层 03-QtWidgets/02-事件处理进阶。

#include "event_filter_logger.h"

#include <QEvent>
#include <QPlainTextEdit>
#include <QWidget>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

EventFilterLogger::EventFilterLogger(QPlainTextEdit* logOutput, QObject* parent)
    : QObject(parent)
    , m_logOutput(logOutput)
    , m_target(nullptr)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// 安装过滤器
// ─────────────────────────────────────────────────────────────────────────────

void EventFilterLogger::installOn(QWidget* target)
{
    m_target = target;
    target->installEventFilter(this);

    appendLog(QStringLiteral("[EventFilterLogger] 已安装到 %1，开始监听事件。")
                  .arg(target->metaObject()->className()));
}

// ─────────────────────────────────────────────────────────────────────────────
// 事件过滤器实现
// ─────────────────────────────────────────────────────────────────────────────

bool EventFilterLogger::eventFilter(QObject* watched, QEvent* event)
{
    // 只关注鼠标和键盘相关事件，避免日志量过大
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        appendLog(QStringLiteral("[eventFilter] MouseButtonPress → 即将进入 event() → "
                                 "mousePressEvent 调用链"));
        break;
    case QEvent::MouseButtonRelease:
        appendLog(QStringLiteral("[eventFilter] MouseButtonRelease"));
        break;
    case QEvent::MouseMove:
        appendLog(QStringLiteral("[eventFilter] MouseMove"));
        break;
    case QEvent::KeyPress: {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        appendLog(QStringLiteral("[eventFilter] KeyPress: key=0x%1, modifiers=0x%2")
                      .arg(keyEvent->key(), 0, 16)
                      .arg(static_cast<int>(keyEvent->modifiers()), 0, 16));
        break;
    }
    case QEvent::KeyRelease:
        appendLog(QStringLiteral("[eventFilter] KeyRelease"));
        break;
    case QEvent::Enter:
        appendLog(QStringLiteral("[eventFilter] Enter —— 鼠标进入控件"));
        break;
    case QEvent::Leave:
        appendLog(QStringLiteral("[eventFilter] Leave —— 鼠标离开控件"));
        break;
    default:
        // 其他事件类型不记录，避免刷屏
        break;
    }

    // 始终返回 false：不消费事件，让事件继续沿传播链传递
    return QObject::eventFilter(watched, event);
}

// ─────────────────────────────────────────────────────────────────────────────
// 日志输出
// ─────────────────────────────────────────────────────────────────────────────

void EventFilterLogger::appendLog(const QString& message)
{
    if (m_logOutput) {
        m_logOutput->appendPlainText(message);
    }
}
