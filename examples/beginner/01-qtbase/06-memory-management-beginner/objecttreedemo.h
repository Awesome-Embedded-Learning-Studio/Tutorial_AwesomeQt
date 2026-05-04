#ifndef OBJECTTREEDEMO_H
#define OBJECTTREEDEMO_H

#include <QObject>
#include <QDebug>
#include "task.h"

// 演示对象树机制的类
class ObjectTreeDemo : public QObject
{
    Q_OBJECT

public:
    explicit ObjectTreeDemo(QObject* parent = nullptr)
        : QObject(parent)
    {
        qDebug() << "\n=== 演示 1：对象树自动清理 ===";
    }

    void demonstrate()
    {
        // 创建父对象（栈上分配，自动析构）
        QObject* parent = new QObject(this);

        // 创建多个子对象，都指定同一个父对象
        Task* task1 = new Task("任务 A", parent);
        Task* task2 = new Task("任务 B", parent);
        Task* task3 = new Task("任务 C", parent);

        Q_UNUSED(task1);
        Q_UNUSED(task2);
        Q_UNUSED(task3);

        qDebug() << "创建了 3 个子任务，都属于同一个父对象";

        // 当 ObjectTreeDemo 析构时，parent 会被自动删除
        // parent 删除时，所有子任务也会被自动删除
        qDebug() << "父对象析构时，所有子对象将自动清理";
    }
};

#endif // OBJECTTREEDEMO_H
