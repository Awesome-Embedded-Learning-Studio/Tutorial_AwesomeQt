/**
 * QtWebChannel 交互面板示例
 *
 * 本示例演示 QtWebChannel 模块的核心功能：
 * - QWebChannel 将 C++ 对象暴露给 JavaScript
 * - qwebchannel.js 前端侧配置
 * - 双向通信：JS 调用 Qt 方法 / Qt 发信号到 JS
 * - 与 QWebEngineView 集成
 *
 * 启动后弹出一个 WebEngineView 窗口，显示交互面板：
 *   - 获取时间：JS 调用 C++ getCurrentTime() 方法
 *   - 获取主机名：JS 调用 C++ getHostName() 方法
 *   - 系统信息：JS 调用 C++ getSystemInfo() 方法
 *   - 推送通知：JS 触发 C++ emit signal 回传 JS
 */

#include "backendobject.h"

#include <QDateTime>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>

/// @brief 暴露给 JavaScript 的后端交互对象
/// 演示 Q_PROPERTY 属性暴露、Q_INVOKABLE 方法调用、信号推送

BackendObject::BackendObject(QObject *parent)
    : QObject(parent), m_callCount(0)
{
}

QString BackendObject::platformInfo() const
{
    return QStringLiteral("%1 / %2 / %3")
        .arg(QSysInfo::prettyProductName())
        .arg(QSysInfo::currentCpuArchitecture())
        .arg(QSysInfo::kernelVersion());
}

int BackendObject::callCount() const { return m_callCount; }

/// @brief 获取当前时间字符串（ISO 格式）
QString BackendObject::getCurrentTime()
{
    incrementCount();
    return QDateTime::currentDateTime().toString(Qt::ISODate);
}

/// @brief 获取本机主机名
QString BackendObject::getHostName()
{
    incrementCount();
    return QHostInfo::localHostName();
}

/// @brief 获取系统信息（JSON 格式）
QString BackendObject::getSystemInfo()
{
    incrementCount();
    QJsonObject info;
    info[QStringLiteral("hostname")] = QHostInfo::localHostName();
    info[QStringLiteral("platform")] = QSysInfo::prettyProductName();
    info[QStringLiteral("arch")] = QSysInfo::currentCpuArchitecture();
    info[QStringLiteral("kernel")] = QSysInfo::kernelVersion();
    info[QStringLiteral("machineId")] = QString::fromUtf8(QSysInfo::machineUniqueId());
    info[QStringLiteral("timestamp")]
        = QDateTime::currentDateTime().toString(Qt::ISODate);
    return QJsonDocument(info).toJson(QJsonDocument::Compact);
}

/// @brief 从 C++ 侧推送通知到 JS（演示信号回调）
void BackendObject::sendNotification(const QString &text)
{
    incrementCount();
    emit notificationPushed(text);
}

void BackendObject::incrementCount()
{
    m_callCount++;
    emit callCountChanged(m_callCount);
}
