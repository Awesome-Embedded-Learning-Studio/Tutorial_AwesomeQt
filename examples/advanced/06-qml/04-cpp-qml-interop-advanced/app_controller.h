/// @file    app_controller.h
/// @brief   C++ 单例控制器，通过 QML_SINGLETON 暴露全局工具方法给 QML。
///
/// 演示 Q_PROPERTY 与 Q_INVOKABLE 两种方式将 C++ 功能暴露给 QML。
/// 对应教程：进阶层 06-QML/04-C++ 与 QML 互操作。

#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

/// @brief 全局应用控制器，以单例形式注册到 QML 引擎。
///
/// QML 端通过 AppController.formatTime() 等方式直接调用，
/// 无需在 QML 中手动实例化。
class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /// 当前模型中的总任务数量，QML 可绑定该属性实现自动刷新。
    Q_PROPERTY(int totalTasks READ totalTasks NOTIFY totalTasksChanged)

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit AppController(QObject* parent = nullptr);

    /// @brief 返回当前总任务数。
    /// @return 总任务数。
    int totalTasks() const;

    /// @brief 设置总任务数（由外部模型更新时调用）。
    /// @param[in] count 新的任务总数。
    /// @note 暴露为 Q_INVOKABLE 让 QML 也能触发刷新。
    Q_INVOKABLE void setTotalTasks(int count);

    /// @brief 返回当前时间的格式化字符串。
    /// @return 形如 "HH:mm:ss" 的时间文本。
    Q_INVOKABLE QString formatTime() const;

    /// @brief 根据用户名返回问候语。
    /// @param[in] name 用户名称。
    /// @return 个性化问候字符串。
    Q_INVOKABLE QString greeting(const QString& name) const;

signals:
    /// totalTasks 属性变化时发射，QML 绑定自动刷新。
    void totalTasksChanged();

private:
    int m_totalTasks = 0;  ///< 缓存的任务总数
};
