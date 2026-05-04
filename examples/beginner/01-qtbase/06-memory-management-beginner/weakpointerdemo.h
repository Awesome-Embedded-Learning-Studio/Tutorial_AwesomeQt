#ifndef WEAKPOINTERDEMO_H
#define WEAKPOINTERDEMO_H

#include <QObject>
#include <QDebug>
#include <QSharedPointer>
#include <QWeakPointer>
#include "task.h"

// 演示 QWeakPointer 弱引用的类
class WeakPointerDemo : public QObject
{
    Q_OBJECT

public:
    explicit WeakPointerDemo(QObject* parent = nullptr)
        : QObject(parent)
    {
        qDebug() << "\n=== 演示 3：QWeakPointer 弱引用 ===";
    }

    void demonstrate()
    {
        // 创建共享指针
        QSharedPointer<Task> strongTask = QSharedPointer<Task>::create("弱引用演示任务");
        qDebug() << "强引用创建，对象被引用计数管理";  // 引用计数为 1

        // 创建弱引用，不增加引用计数
        QWeakPointer<Task> weakTask = strongTask;
        qDebug() << "弱引用创建，不增加引用计数";  // 引用计数仍为 1

        // 使用弱引用前，需要先转换为强引用
        if (!weakTask.isNull()) {
            QSharedPointer<Task> locked = weakTask.toStrongRef();
            if (locked) {
                qDebug() << "通过弱引用成功锁定对象:" << locked->name();
                locked->execute();
            }
        }

        // 清除强引用
        strongTask.clear();
        qDebug() << "强引用已清除";

        // 现在弱引用应该已经失效
        if (weakTask.isNull()) {
            qDebug() << "弱引用已失效（对象已被删除）";
        } else {
            qDebug() << "弱引用仍然有效";
        }
    }
};

#endif // WEAKPOINTERDEMO_H
