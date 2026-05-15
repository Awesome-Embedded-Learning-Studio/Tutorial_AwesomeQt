/// @file    config_object.h
/// @brief   演示 Q_PROPERTY 高级特性：CONSTANT / RESET / FINAL / Q_ENUM。
///
/// 对应教程：进阶层 01-QtBase/01-QObject 属性系统深度拆解。

#pragma once

#include <QObject>
#include <QString>

/// 演示 Q_PROPERTY 高级修饰符用法的配置对象。
///
/// 展示 CONSTANT（不可变属性）、RESET（恢复默认值）、
/// FINAL（禁止子类覆盖）、Q_ENUM（枚举注册到元对象系统）等特性。
class ConfigObject : public QObject
{
    Q_OBJECT

    // 元数据：可通过 metaObject()->classInfo() 在运行时读取
    Q_CLASSINFO("Author", "QtDemo")
    Q_CLASSINFO("Version", "2.0")

    // CONSTANT 属性：只在构造时设置一次，不能有 WRITE 和 NOTIFY
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

    // 完整属性：READ / WRITE / NOTIFY / RESET 四件套
    // RESET 函数用于将属性恢复为默认值，常见于 Designer 的「重置属性」操作
    Q_PROPERTY(bool debugMode READ debugMode WRITE setDebugMode
               RESET resetDebugMode NOTIFY debugModeChanged)

    // FINAL 属性：禁止子类用 Q_PROPERTY 覆盖
    // QML 引擎对 FINAL 属性可以做更激进的优化
    Q_PROPERTY(int logLevel READ logLevel WRITE setLogLevel
               NOTIFY logLevelChanged FINAL)

public:
    /// 日志级别枚举，通过 Q_ENUM 注册到元对象系统。
    enum LogLevel
    {
        kTrace = 0,
        kDebug = 1,
        kInfo  = 2,
        kWarn  = 3,
        kError = 4
    };
    Q_ENUM(LogLevel)

    /// @brief 构造函数，初始化属性默认值。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit ConfigObject(QObject* parent = nullptr);

    /// @brief 获取应用程序版本号（CONSTANT 属性）。
    /// @return 版本号字符串。
    QString appVersion() const;

    /// @brief 获取调试模式状态。
    /// @return true 表示调试模式已开启。
    bool debugMode() const;

    /// @brief 设置调试模式。
    /// @param[in] mode 新的调试模式状态。
    void setDebugMode(bool mode);

    /// @brief 将 debugMode 恢复为默认值 false。
    /// @note QMetaProperty::reset() 会调用这个函数。
    void resetDebugMode();

    /// @brief 获取日志级别。
    /// @return 当前日志级别的整数值。
    int logLevel() const;

    /// @brief 设置日志级别，自动裁剪到合法范围 [kTrace, kError]。
    /// @param[in] level 新的日志级别。
    void setLogLevel(int level);

signals:
    /// debugMode 属性变更通知信号。
    void debugModeChanged();

    /// logLevel 属性变更通知信号。
    void logLevelChanged();

private:
    QString m_appVersion;    // 应用版本号，CONSTANT，构造后不再变
    bool    m_debugMode;     // 调试模式开关
    int     m_logLevel;      // 当前日志级别
};
