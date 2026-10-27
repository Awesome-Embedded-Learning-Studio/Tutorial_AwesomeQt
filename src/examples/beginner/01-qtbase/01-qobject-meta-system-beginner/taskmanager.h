#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QDebug>
#include <QList>
#include "taskitem.h"

class TaskManager : public QObject
{
    Q_OBJECT

public:
    explicit TaskManager(QObject *parent = nullptr) : QObject(parent)
    {
        qDebug() << "创建 TaskManager";
    }

    ~TaskManager() override
    {
        qDebug() << "销毁 TaskManager，所有子任务将自动清理";
    }

    // 添加任务：新任务的 parent 设置为 this（加入对象树）
    TaskItem* addTask(const QString &title, int priority = 0)
    {
        // 注意：不需要 delete，对象树会在 TaskManager 销毁时自动清理
        TaskItem *task = new TaskItem(title, priority, this);
        m_tasks.append(task);
        return task;
    }

    // 演示遍历子对象
    void printAllTasks() const
    {
        qDebug() << "\n--- 所有任务列表 ---";
        // findChildren 演示：查找所有 TaskItem 类型的子对象
        QList<TaskItem*> tasks = findChildren<TaskItem*>(QString(), Qt::FindDirectChildrenOnly);
        for (TaskItem *task : tasks) {
            qDebug() << "  -" << task->title()
                     << "| 优先级:" << task->priority()
                     << "| 完成:" << task->completed();
        }
        qDebug() << "-------------------\n";
    }

private:
    QList<TaskItem*> m_tasks;  // 任务列表（注意：内存管理由对象树负责）
};

#endif // TASKMANAGER_H
