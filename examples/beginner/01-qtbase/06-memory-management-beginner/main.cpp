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
#include <QDebug>
#include <QTimer>

#include "objecttreedemo.h"
#include "sharedpointerdemo.h"
#include "weakpointerdemo.h"
#include "qpointerdemo.h"
#include "circularreferencedemo.h"

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
