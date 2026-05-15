/// @file    event_filter_demo.cpp
/// @brief   GlobalEventFilter、FilteredObject 的实现，以及事件过滤器演示函数。
///
/// 对应教程：进阶层 01-QtBase/07-事件系统进阶。

#include "event_filter_demo.h"

#include <QCoreApplication>

// ===========================================================================
// GlobalEventFilter
// ===========================================================================

GlobalEventFilter::GlobalEventFilter(QObject* parent) : QObject(parent)
{
}

bool GlobalEventFilter::eventFilter(QObject* watched, QEvent* event)
{
    // 拦截键盘事件
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        qDebug() << "  [eventFilter] 拦截键盘事件:"
                 << "key=" << keyEvent->key()
                 << "text=" << keyEvent->text();

        // 拦截 Escape 键，阻止传递
        if (keyEvent->key() == Qt::Key_Escape) {
            qDebug() << "  Escape 键被拦截！不传递给目标对象";
            return true;
        }
    }

    // 拦截鼠标事件
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << "  [eventFilter] 拦截鼠标事件:"
                 << "pos=" << mouseEvent->position()
                 << "button=" << mouseEvent->button();
    }

    // 其他事件正常传递
    return QObject::eventFilter(watched, event);
}

// ===========================================================================
// FilteredObject
// ===========================================================================

FilteredObject::FilteredObject(QObject* parent) : QObject(parent)
{
    // 给自身安装事件过滤器
    installEventFilter(this);
}

bool FilteredObject::event(QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        qDebug() << "  [event] 收到按键:" << keyEvent->key();
    }
    return QObject::event(event);
}

bool FilteredObject::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        qDebug() << "  [自身过滤器] 按键:" << keyEvent->text();
        // 返回 false 让事件继续传递到 event()
    }
    return QObject::eventFilter(watched, event);
}

// ===========================================================================
// 演示函数
// ===========================================================================

void demoEventFilter()
{
    qDebug() << "\n=== 事件过滤器演示 ===";

    // 注意: 在控制台应用中无法真正生成键盘/鼠标事件
    // 这里演示 API 用法和事件传播机制
    qDebug() << "事件过滤器可以安装在:";
    qDebug() << "  1. QApplication 上 -> 拦截所有事件（全局）";
    qDebug() << "  2. 特定 QObject 上 -> 拦截该对象的事件";
    qDebug() << "  3. 自身 -> 自我过滤";

    qDebug() << "\n事件传播顺序:";
    qDebug() << "  1. QApplication 的事件过滤器";
    qDebug() << "  2. 目标对象上安装的事件过滤器";
    qDebug() << "  3. 目标对象的 event() 方法";
    qDebug() << "  4. 目标对象的特定事件处理器（如 keyPressEvent）";

    // 手动发送一个键盘事件来演示过滤
    GlobalEventFilter filter;
    FilteredObject obj;

    // 给对象安装过滤器
    obj.installEventFilter(&filter);

    qDebug() << "\n--- 发送键盘事件 ---";
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "A");
    QCoreApplication::sendEvent(&obj, &keyEvent);

    qDebug() << "\n--- 发送 Escape 键（被过滤器拦截）---";
    QKeyEvent escEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(&obj, &escEvent);

    // 移除过滤器
    obj.removeEventFilter(&filter);
    qDebug() << "\n过滤器已移除";
}
