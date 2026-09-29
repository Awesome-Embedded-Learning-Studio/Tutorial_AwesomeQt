#ifndef CIRCULARREFERENCEDEMO_H
#define CIRCULARREFERENCEDEMO_H

#include <QObject>
#include <QDebug>
#include <QSharedPointer>
#include <QWeakPointer>

// 演示循环引用问题的类
class CircularReferenceDemo : public QObject
{
    Q_OBJECT

    // 前向声明
    class NodeA;
    class NodeB;

    // 节点 A 类
    class NodeA : public QObject
    {
    public:
        QSharedPointer<NodeB> partner;  // 持有 B 的共享指针
        ~NodeA() { qDebug() << "[NodeA] 析构"; }
    };

    // 节点 B 类
    class NodeB : public QObject
    {
    public:
        QSharedPointer<NodeA> partner;  // 持有 A 的共享指针 - 这会导致循环引用！
        ~NodeB() { qDebug() << "[NodeB] 析构"; }
    };

public:
    explicit CircularReferenceDemo(QObject* parent = nullptr)
        : QObject(parent)
    {
        qDebug() << "\n=== 演示 5：避免循环引用 ===";
    }

    void demonstrateBad()
    {
        qDebug() << "\n错误示例：循环引用导致内存泄漏";

        // 创建两个互相引用的节点
        QSharedPointer<NodeA> nodeA = QSharedPointer<NodeA>::create();
        QSharedPointer<NodeB> nodeB = QSharedPointer<NodeB>::create();

        // 互相持有对方的共享指针 - 循环引用！
        nodeA->partner = nodeB;
        nodeB->partner = nodeA;

        qDebug() << "nodeA 被两个强引用持有";  // nodeA 自身 + nodeB->partner
        qDebug() << "nodeB 被两个强引用持有";  // nodeB 自身 + nodeA->partner

        // 当 nodeA 和 nodeB 离开作用域时
        // 它们的引用计数只会减 1，变成 1，不会归零
        // 因此对象永远不会被删除 - 内存泄漏！
        qDebug() << "函数结束时，两个对象的引用计数不会归零，内存泄漏！";
    }

    void demonstrateGood()
    {
        qDebug() << "\n正确示例：使用弱引用打破循环";

        // 定义正确版本的节点类（在函数外定义避免类型问题）
        struct NodeACorrected : public QObject
        {
            QSharedPointer<QObject> partner;  // 使用 QObject 基类避免类型循环
            ~NodeACorrected() { qDebug() << "[NodeACorrected] 析构"; }
        };

        struct NodeBCorrected : public QObject
        {
            QWeakPointer<QObject> partner;  // 使用弱引用打破循环
            ~NodeBCorrected() { qDebug() << "[NodeBCorrected] 析构"; }
        };

        QSharedPointer<NodeACorrected> nodeA = QSharedPointer<NodeACorrected>::create();
        QSharedPointer<NodeBCorrected> nodeB = QSharedPointer<NodeBCorrected>::create();

        // nodeA 强引用 nodeB
        nodeA->partner = nodeB;
        // nodeB 弱引用 nodeA，不增加引用计数
        nodeB->partner = nodeA;

        qDebug() << "正确设置：nodeA 强引用 nodeB，nodeB 弱引用 nodeA";

        // 现在引用计数正确，对象会被正常删除
        qDebug() << "函数结束时，对象将被正确删除";
    }
};

#endif // CIRCULARREFERENCEDEMO_H
