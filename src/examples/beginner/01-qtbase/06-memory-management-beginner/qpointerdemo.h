#ifndef QPOINTERDEMO_H
#define QPOINTERDEMO_H

#include <QObject>
#include <QPointer>
#include <QDebug>
#include "task.h"

// 演示 QPointer 用于 QObject 的类
class QPointerDemo : public QObject
{
    Q_OBJECT

public:
    explicit QPointerDemo(QObject* parent = nullptr)
        : QObject(parent)
    {
        qDebug() << "\n=== 演示 4：QPointer QObject 弱引用 ===";
    }

    void demonstrate()
    {
        // 创建一个 Task 对象（手动管理，为了演示 QPointer）
        Task* rawTask = new Task("QPointer 演示任务");

        // 创建 QPointer 弱引用
        QPointer<Task> safePtr = rawTask;

        qDebug() << "通过 QPointer 访问任务:" << safePtr->name();

        // 检查指针是否有效
        if (safePtr) {
            qDebug() << "QPointer 指向有效对象";
            safePtr->execute();
        }

        // 手动删除原始对象
        delete rawTask;
        qDebug() << "原始对象已删除";

        // QPointer 会自动变为空
        if (safePtr) {
            qDebug() << "QPointer 仍然有效（不应该看到这条消息）";
        } else {
            qDebug() << "QPointer 已自动置空，避免悬空指针";
        }
    }
};

#endif // QPOINTERDEMO_H
