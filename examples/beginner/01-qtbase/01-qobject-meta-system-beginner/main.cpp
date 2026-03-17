// QtBase 入门示例 01: QObject 与元对象系统
// 这个示例演示了 QObject 基础、对象树内存管理、Q_OBJECT 宏、Q_PROPERTY 属性系统

#include <QCoreApplication>      // Qt 应用程序核心类，提供事件循环
#include <QObject>              // 所有 Qt 对象的基类
#include <QDebug>               // 调试输出流
#include <QString>              // Qt 字符串类
#include <QMetaObject>          // 元对象系统
#include <QMetaProperty>        // 属性系统

// ============================================================================
// 第一步：定义一个简单的 QObject 派生类
// ============================================================================
class SimpleObject : public QObject
{
    Q_OBJECT  // 必须的宏，启用元对象系统（信号槽、属性、反射等）

public:
    // 构造函数接受 parent 参数，用于建立对象树关系
    explicit SimpleObject(const QString &name, QObject *parent = nullptr)
        : QObject(parent), m_name(name)  // 初始化基类和成员变量
    {
        qDebug() << "创建 SimpleObject:" << m_name;
    }

    // 析构函数演示对象树销毁顺序
    ~SimpleObject() override
    {
        qDebug() << "销毁 SimpleObject:" << m_name;
    }

    // 使用 Q_PROPERTY 声明属性，让属性可被元对象系统识别
    // READ 指定读取函数，WRITE 指定写入函数，NOTIFY 指定变更信号
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    // name 属性的 getter
    QString name() const { return m_name; }

    // name 属性的 setter
    void setName(const QString &name)
    {
        if (m_name != name) {  // 只有值真正改变时才发出信号
            m_name = name;
            emit nameChanged();  // 发出属性变更信号
        }
    }

signals:  // signals: 关键字声明信号函数
    void nameChanged();  // 属性变更信号（Q_PROPERTY 的 NOTIFY）

private:
    QString m_name;  // 私有成员变量存储属性值
};

// ============================================================================
// 第二步：定义一个更复杂的类，演示多个属性和对象树
// ============================================================================
class TaskItem : public QObject
{
    Q_OBJECT

public:
    explicit TaskItem(const QString &title, int priority, QObject *parent = nullptr)
        : QObject(parent), m_title(title), m_priority(priority), m_completed(false)
    {
        qDebug() << "创建 TaskItem:" << m_title << "(优先级:" << m_priority << ")";
    }

    ~TaskItem() override
    {
        qDebug() << "销毁 TaskItem:" << m_title;
    }

    // 声明多个属性，演示完整的属性系统
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged)

    // Getter 和 Setter 函数
    QString title() const { return m_title; }
    void setTitle(const QString &title)
    {
        if (m_title != title) {
            m_title = title;
            emit titleChanged();
        }
    }

    int priority() const { return m_priority; }
    void setPriority(int priority)
    {
        if (m_priority != priority) {
            m_priority = priority;
            emit priorityChanged();
        }
    }

    bool completed() const { return m_completed; }
    void setCompleted(bool completed)
    {
        if (m_completed != completed) {
            m_completed = completed;
            emit completedChanged();
        }
    }

    // 演示如何通过元对象系统打印属性信息
    void printProperties() const
    {
        const QMetaObject *meta = metaObject();  // 获取元对象
        qDebug() << "=== " << m_title << " 的属性 ===";
        // 遍历所有属性
        for (int i = 0; i < meta->propertyCount(); ++i) {
            QMetaProperty prop = meta->property(i);
            if (prop.isReadable()) {  // 跳过不可读属性
                qDebug() << prop.name() << "=" << prop.read(this);
            }
        }
        qDebug() << "====================";
    }

signals:
    void titleChanged();
    void priorityChanged();
    void completedChanged();

private:
    QString m_title;
    int m_priority;
    bool m_completed;
};

// ============================================================================
// 第三步：定义任务管理器类，演示对象树管理
// ============================================================================
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

// ============================================================================
// 主函数：演示各种功能
// ============================================================================
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);  // 创建应用程序对象（无需 GUI）

    qDebug() << "========== Qt 对象树与元对象系统示例 ==========\n";

    // ----------------------------------------------------------------------
    // 演示 1: 基本 QObject 和 Q_OBJECT
    // ----------------------------------------------------------------------
    qDebug() << "[演示 1] 基本 QObject 用法";
    SimpleObject simple("简单对象");
    qDebug() << "对象名称:" << simple.name();
    qDebug() << "类名称:" << simple.metaObject()->className();  // 元对象系统获取类名

    // ----------------------------------------------------------------------
    // 演示 2: 属性系统运行时读写
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 2] 属性系统动态读写";
    simple.setProperty("name", "修改后的名称");  // 动态设置属性
    qDebug() << "修改后的名称:" << simple.property("name").toString();  // 动态读取属性

    // ----------------------------------------------------------------------
    // 演示 3: 对象树与内存管理
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 3] 对象树自动内存管理";
    {
        // parent 在栈上，离开作用域会自动销毁
        QObject parent;
        parent.setObjectName("父对象");

        // 创建子对象，指定 parent
        SimpleObject *child1 = new SimpleObject("子对象1", &parent);
        SimpleObject *child2 = new SimpleObject("子对象2", &parent);

        qDebug() << "父对象有" << parent.children().size() << "个子对象";

        // parent 离开作用域时会自动删除 child1 和 child2
        qDebug() << "父对象即将离开作用域，子对象会被自动删除...";
    }
    qDebug() << "父对象已销毁，所有子对象也被清理\n";

    // ----------------------------------------------------------------------
    // 演示 4: qobject_cast 类型转换
    // ----------------------------------------------------------------------
    qDebug() << "[演示 4] qobject_cast 类型转换";
    QObject *obj = new SimpleObject("转换测试");
    SimpleObject *simpleCast = qobject_cast<SimpleObject*>(obj);
    if (simpleCast) {
        qDebug() << "qobject_cast 转换成功:" << simpleCast->name();
    }

    // 对不兼容类型转换会返回 nullptr
    TaskItem *taskCast = qobject_cast<TaskItem*>(obj);
    if (!taskCast) {
        qDebug() << "qobject_cast 转换失败（类型不兼容），返回 nullptr";
    }

    delete obj;  // 没有 parent 的对象需要手动删除

    // ----------------------------------------------------------------------
    // 演示 5: 完整的任务管理器示例
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 5] 任务管理器（对象树实战）";
    TaskManager manager;

    // 添加任务（新任务自动加入 manager 的对象树）
    TaskItem *task1 = manager.addTask("学习 Qt 基础", 1);
    TaskItem *task2 = manager.addTask("完成练习项目", 2);
    TaskItem *task3 = manager.addTask("阅读官方文档", 3);

    // 打印所有任务
    manager.printAllTasks();

    // 修改任务状态，触发属性变更信号
    task1->setCompleted(true);
    task2->setPriority(0);  // 提高优先级

    // 使用元对象系统打印属性
    task1->printProperties();
    task2->printProperties();

    // ----------------------------------------------------------------------
    // 演示 6: 动态属性（未在 Q_PROPERTY 声明）
    // ----------------------------------------------------------------------
    qDebug() << "\n[演示 6] 动态属性";
    task3->setProperty("assignee", "张三");  // 添加动态属性
    if (task3->dynamicPropertyNames().contains("assignee")) {
        qDebug() << "动态属性 assignee:" << task3->property("assignee").toString();
    }

    // ----------------------------------------------------------------------
    // 结束演示
    // ----------------------------------------------------------------------
    qDebug() << "\n程序即将结束，TaskManager 销毁时会自动清理所有任务...";
    qDebug() << "观察析构顺序：子对象在父对象之前销毁";
    qDebug() << "===========================================";

    return 0;  // app 销毁时所有对象都会被清理
}

// 注意：这个文件需要 moc 处理 Q_OBJECT 宏
// CMake 的 CMAKE_AUTOMOC ON 会自动处理，无需手动运行 moc
// 但由于 main.cpp 中包含 Q_OBJECT 宏的类定义，需要显式包含 moc 文件
#include "main.moc"
