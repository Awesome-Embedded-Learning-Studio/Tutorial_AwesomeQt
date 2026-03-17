/*
 * Qt 内存管理入门示例
 *
 * 本示例演示 Qt 中几种主要的内存管理方式：
 * 1. 对象树（Object Tree）自动清理
 * 2. QSharedPointer 共享所有权
 * 3. QWeakPointer 弱引用
 * 4. QPointer QObject 弱引用
 *
 * 编译命令：
 * mkdir build && cd build
 * cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/gcc_64
 * cmake --build .
 * ./06-memory-management-beginner
 */

#include <QCoreApplication>
#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QPointer>
#include <QDebug>
#include <QString>
#include <QTimer>

// 自定义任务类，用于演示智能指针的用法
class Task : public QObject
{
    Q_OBJECT

public:
    explicit Task(const QString& name, QObject* parent = nullptr)
        : QObject(parent), m_name(name)
    {
        qDebug() << "[Task] 创建任务:" << m_name;
    }

    ~Task()
    {
        qDebug() << "[Task] 销毁任务:" << m_name;
    }

    void execute()
    {
        qDebug() << "[Task] 执行任务:" << m_name;
    }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

private:
    QString m_name;
};

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

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt 内存管理入门示例";
    qDebug() << "========================";

    // 使用定时器延迟执行，确保看到析构顺序
    QTimer::singleShot(0, [&]() {
        // 演示 1：对象树
        {
            ObjectTreeDemo treeDemo;
            treeDemo.demonstrate();
        }
        qDebug() << "ObjectTreeDemo 离开作用域，观察子对象的析构顺序\n";

        // 演示 2：QSharedPointer
        {
            SharedPointerDemo sharedDemo;
            sharedDemo.demonstrate();
        }
        qDebug() << "SharedPointerDemo 离开作用域，观察共享对象的析构\n";

        // 演示 3：QWeakPointer
        {
            WeakPointerDemo weakDemo;
            weakDemo.demonstrate();
        }
        qDebug() << "WeakPointerDemo 演示完成\n";

        // 演示 4：QPointer
        {
            QPointerDemo qpointerDemo;
            qpointerDemo.demonstrate();
        }
        qDebug() << "QPointerDemo 演示完成\n";

        // 演示 5：循环引用
        {
            CircularReferenceDemo circularDemo;
            circularDemo.demonstrateBad();
            circularDemo.demonstrateGood();
        }
        qDebug() << "CircularReferenceDemo 演示完成\n";

        qDebug() << "\n========================";
        qDebug() << "所有演示完成！";
        qDebug() << "\n关键要点：";
        qDebug() << "1. 对象树：父子关系自动管理内存";
        qDebug() << "2. QSharedPointer：共享所有权，引用计数";
        qDebug() << "3. QWeakPointer：弱引用，不增加引用计数";
        qDebug() << "4. QPointer：QObject 专用弱指针";
        qDebug() << "5. 避免循环引用：使用弱引用打破循环";

        // 退出应用
        QCoreApplication::quit();
    });

    return app.exec();
}

#include "main.moc"  // 因为我们在 .cpp 文件中使用了 Q_OBJECT 宏
