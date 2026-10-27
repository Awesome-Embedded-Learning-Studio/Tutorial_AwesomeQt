#ifndef SHAREDPOINTERDEMO_H
#define SHAREDPOINTERDEMO_H

#include <QObject>
#include <QDebug>
#include <QSharedPointer>
#include "task.h"

// 演示 QSharedPointer 共享所有权的类
class SharedPointerDemo : public QObject
{
    Q_OBJECT

public:
    explicit SharedPointerDemo(QObject* parent = nullptr)
        : QObject(parent)
    {
        qDebug() << "\n=== 演示 2：QSharedPointer 共享所有权 ===";
    }

    void demonstrate()
    {
        // 创建一个共享指针管理的任务
        QSharedPointer<Task> task1 = QSharedPointer<Task>::create("共享任务 A");
        qDebug() << "创建共享任务 A";  // 引用计数为 1

        {
            // 复制共享指针，引用计数增加
            QSharedPointer<Task> task2 = task1;
            qDebug() << "复制后，两个共享指针指向同一对象";  // 引用计数为 2

            // 通过不同的指针访问同一个对象
            task2->execute();
        }
        // task2 离开作用域，引用计数减少
        qDebug() << "task2 离开作用域，只剩下 task1 持有引用";  // 引用计数回到 1

        // 创建另一个独立的共享任务
        QSharedPointer<Task> task3 = QSharedPointer<Task>::create("共享任务 B");
        qDebug() << "创建共享任务 B";

        // 当函数结束时，task1 和 task3 离开作用域
        // 如果引用计数归零，对象会被自动删除
        qDebug() << "函数结束时，引用计数归零的对象将被自动删除";
    }
};

#endif // SHAREDPOINTERDEMO_H
