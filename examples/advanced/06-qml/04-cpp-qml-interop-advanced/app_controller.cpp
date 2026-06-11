/// @file    app_controller.cpp
/// @brief   AppController 的实现，提供全局工具方法。
///
/// 对应教程：进阶层 06-QML/04-C++ 与 QML 互操作。

#include "app_controller.h"

#include <QDateTime>

AppController::AppController(QObject* parent)
    : QObject(parent)
{
}

int AppController::totalTasks() const
{
    return m_totalTasks;
}

void AppController::setTotalTasks(int count)
{
    if (m_totalTasks == count)
    {
        return;
    }
    m_totalTasks = count;
    emit totalTasksChanged();
}

QString AppController::formatTime() const
{
    // 使用 Qt 的时间格式化，保证跨平台一致
    return QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
}

QString AppController::greeting(const QString& name) const
{
    // 根据当前小时判断时段，生成更自然的问候语
    const int hour = QTime::currentTime().hour();
    QString period;

    if (hour < 12)
    {
        period = QStringLiteral("Good morning");
    }
    else if (hour < 18)
    {
        period = QStringLiteral("Good afternoon");
    }
    else
    {
        period = QStringLiteral("Good evening");
    }

    if (name.isEmpty())
    {
        return period + QLatin1String(", friend!");
    }
    return period + ", " + name + QLatin1Char('!');
}
