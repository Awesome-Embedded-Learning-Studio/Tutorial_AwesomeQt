/// @file    main.cpp
/// @brief   程序入口，依次调用各演示函数并输出要点总结。
///
/// 对应教程：进阶层 01-QtBase/06-内存管理。
/// 串联智能指针、循环引用、QPointer 三个子示例。

#include <QCoreApplication>
#include <QDebug>

#include "circular_fix.h"
#include "qpointer_demo.h"
#include "smart_pointer_demo.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========== Qt 内存管理进阶示例 ==========\n";

    // 演示 1: QSharedPointer 引用计数追踪
    qDebug() << "[演示 1] QSharedPointer 引用计数追踪";
    demoSharedPointer();

    // 演示 2: 循环引用与 QWeakPointer 解决方案
    qDebug() << "\n[演示 2] 循环引用与 QWeakPointer 解决方案";
    demoCircularReference();

    // 演示 3: QPointer 自动置空
    qDebug() << "\n[演示 3] QPointer 自动置空";
    demoQPointerAutoNull();

    // 演示 4: QScopedPointer vs std::unique_ptr
    qDebug() << "\n[演示 4] QScopedPointer vs std::unique_ptr";
    demoScopedPointer();

    // 演示 5: 内存管理策略选择指南
    qDebug() << "\n[演示 5] 内存管理策略选择指南";
    demoMemoryManagementGuide();

    // 结束
    qDebug() << "\n===========================================";
    qDebug() << "要点总结:";
    qDebug() << "  - QSharedPointer 提供引用计数的共享所有权";
    qDebug() << "  - QWeakPointer 打破循环引用，不增加引用计数";
    qDebug() << "  - QPointer 监视 QObject 生命周期，删除后自动置空";
    qDebug() << "  - 根据场景选择合适的内存管理策略";

    return 0;
}
