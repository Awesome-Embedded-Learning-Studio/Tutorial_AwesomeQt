/// @file    task_model.h
/// @brief   C++ list model exposing task data to QML via QAbstractListModel.
///
/// 演示如何将自定义 C++ 数据模型注册到 QML 引擎，供 ListView 直接消费。
/// 对应教程：进阶层 06-QML/04-C++ 与 QML 互操作。

#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include <QVector>

/// 单条任务的数据结构，对应 QML delegate 中的各个角色。
struct TaskItem
{
    QString name;       ///< 任务名称
    bool done;          ///< 是否已完成
    int priority;       ///< 优先级（数值越大越紧急）
};

/// @brief 暴露给 QML 的任务列表模型，基于 QAbstractListModel。
///
/// 通过 QML_ELEMENT 宏注册到 QML 类型系统，QML 端可直接用作 ListView 的 model。
/// 支持 Q_INVOKABLE 方法在 QML 中直接调用增删改操作。
class TaskModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /// 模型角色枚举，供 QML delegate 通过 role 名读取数据。
    enum Roles
    {
        kNameRole = Qt::UserRole + 1,   ///< 任务名称
        kDoneRole,                       ///< 完成状态
        kPriorityRole                    ///< 优先级
    };

    /// @brief 构造函数，初始化模型并填充示例数据。
    /// @param[in] parent 父对象指针，Qt 对象树管理生命周期。
    explicit TaskModel(QObject* parent = nullptr);

    /// @brief 返回模型行数，即任务数量。
    /// @param[in] parent 父索引（平铺列表始终无效）。
    /// @return 任务条数。
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 按 role 返回指定索引的数据。
    /// @param[in] index 要查询的模型索引。
    /// @param[in] role  请求的数据角色。
    /// @return 对应数据，无效时返回 QVariant。
    QVariant data(const QModelIndex& index, int role) const override;

    /// @brief 返回角色名映射，让 QML 可以用字符串名访问角色。
    /// @note QML 引擎通过此映射把 role 枚举值绑定到属性名。
    QHash<int, QByteArray> roleNames() const override;

    /// @brief 在列表末尾添加一条新任务。
    /// @param[in] name     任务名称。
    /// @param[in] priority 优先级数值。
    Q_INVOKABLE void addTask(const QString& name, int priority);

    /// @brief 切换指定索引任务的完成状态。
    /// @param[in] index 任务在列表中的位置。
    Q_INVOKABLE void toggleTask(int index);

    /// @brief 移除指定索引的任务。
    /// @param[in] index 要移除的任务位置。
    Q_INVOKABLE void removeTask(int index);

    /// @brief 统计已完成的任务数量。
    /// @return 已完成任务数。
    Q_INVOKABLE int completedCount() const;

private:
    QVector<TaskItem> m_tasks;  ///< 任务数据存储
};
