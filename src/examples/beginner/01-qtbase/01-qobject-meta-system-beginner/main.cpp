// QtBase 入门示例 01: QObject 与元对象系统
// 这个示例演示了 QObject 基础、对象树内存管理、Q_OBJECT 宏、Q_PROPERTY 属性系统

#include <QCoreApplication>      // Qt 应用程序核心类，提供事件循环
#include <QObject>              // 所有 Qt 对象的基类
#include <QDebug>               // 调试输出流

#include "simpleobject.h"
#include "taskitem.h"
#include "taskmanager.h"

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
