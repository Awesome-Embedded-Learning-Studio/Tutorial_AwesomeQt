/// @file    task_model.cpp
/// @brief   TaskModel 的实现，管理任务数据的增删改查。
///
/// 对应教程：进阶层 06-QML/04-C++ 与 QML 互操作。

#include "task_model.h"

#include <QDateTime>

TaskModel::TaskModel(QObject* parent)
    : QAbstractListModel(parent)
{
    // 填充 5 条示例任务，让示例启动即有内容可交互
    m_tasks = {
        {QStringLiteral("Review pull requests"), false, 3},
        {QStringLiteral("Write unit tests"), false, 2},
        {QStringLiteral("Update documentation"), true, 1},
        {QStringLiteral("Fix memory leak in parser"), false, 5},
        {QStringLiteral("Prepare release notes"), false, 2},
    };
}

int TaskModel::rowCount(const QModelIndex& parent) const
{
    // 平铺列表模型，parent 有效时返回 0（无子项）
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_tasks.size());
}

QVariant TaskModel::data(const QModelIndex& index, int role) const
{
    // 越界检查，防止 QML 端传入无效索引导致越界访问
    if (!index.isValid()
        || index.row() < 0
        || index.row() >= static_cast<int>(m_tasks.size()))
    {
        return {};
    }

    const TaskItem& item = m_tasks.at(index.row());

    switch (role)
    {
    case kNameRole:
        return item.name;
    case kDoneRole:
        return item.done;
    case kPriorityRole:
        return item.priority;
    default:
        return {};
    }
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    // QML 通过这些字符串名访问角色，必须与 delegate 中的属性名一致
    return {
        {kNameRole, "taskName"},
        {kDoneRole, "taskDone"},
        {kPriorityRole, "taskPriority"},
    };
}

void TaskModel::addTask(const QString& name, int priority)
{
    // beginInsertRows / endInsertRows 通知 QML 视图有新行插入
    const int newRow = static_cast<int>(m_tasks.size());
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_tasks.append({name, false, priority});
    endInsertRows();
}

void TaskModel::toggleTask(int index)
{
    if (index < 0 || index >= static_cast<int>(m_tasks.size()))
    {
        return;
    }

    m_tasks[index].done = !m_tasks[index].done;

    // dataChanged 信号通知 QML 刷新指定行的 delegate
    const QModelIndex changed = this->index(index);
    emit dataChanged(changed, changed, {kDoneRole});
}

void TaskModel::removeTask(int index)
{
    if (index < 0 || index >= static_cast<int>(m_tasks.size()))
    {
        return;
    }

    // beginRemoveRows / endRemoveRows 通知 QML 视图有行被移除
    beginRemoveRows(QModelIndex(), index, index);
    m_tasks.removeAt(index);
    endRemoveRows();
}

int TaskModel::completedCount() const
{
    int count = 0;
    for (const auto& task : m_tasks)
    {
        if (task.done)
        {
            ++count;
        }
    }
    return count;
}
