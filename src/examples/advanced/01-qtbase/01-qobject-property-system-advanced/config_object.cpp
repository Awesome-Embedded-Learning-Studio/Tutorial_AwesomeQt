/// @file    config_object.cpp
/// @brief   ConfigObject 类实现——Q_PROPERTY 高级特性演示。
///
/// 对应教程：进阶层 01-QtBase/01-QObject 属性系统深度拆解。

#include "config_object.h"

ConfigObject::ConfigObject(QObject* parent)
    : QObject(parent)
    , m_appVersion(QStringLiteral("1.0.0"))
    , m_debugMode(false)
    , m_logLevel(kInfo)
{
}

QString ConfigObject::appVersion() const
{
    return m_appVersion;
}

bool ConfigObject::debugMode() const
{
    return m_debugMode;
}

void ConfigObject::setDebugMode(bool mode)
{
    // 变更守卫：值没变就不赋值不发射信号
    // 这是 Q_PROPERTY 的黄金法则——防止无意义的信号发射
    if (m_debugMode == mode) {
        return;
    }
    m_debugMode = mode;
    emit debugModeChanged();
}

void ConfigObject::resetDebugMode()
{
    setDebugMode(false);
}

int ConfigObject::logLevel() const
{
    return m_logLevel;
}

void ConfigObject::setLogLevel(int level)
{
    // 范围检查 + 变更守卫
    if (level < kTrace) {
        level = kTrace;
    }
    if (level > kError) {
        level = kError;
    }
    if (m_logLevel == level) {
        return;
    }
    m_logLevel = level;
    emit logLevelChanged();
}
