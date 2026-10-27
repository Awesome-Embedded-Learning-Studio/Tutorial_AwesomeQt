/// @file    lambda_trap.cpp
/// @brief   Lambda 捕获陷阱演示函数实现。
///
/// 对应教程：进阶层 01-QtBase/02-信号与槽工程级深度剖析。

#include "lambda_trap.h"

#include <QDebug>
#include <QPointer>
#include <QTimer>

#include <QCoreApplication>

void demoRawPointerTrap()
{
    qDebug() << "--- Lambda 裸指针捕获陷阱 ---";

    QObject* target = new QObject();
    target->setObjectName(QStringLiteral("TargetObject"));

    QTimer timer;
    int callCount = 0;

    // 危险：裸指针 target 被捕获到 Lambda 中
    // 三参数 connect 没有 context object，target 被删除后连接不会自动断开
    QObject::connect(&timer, &QTimer::timeout, [target, &callCount]() {
        callCount++;
        // 对于裸指针，delete 后指针不会自动变为 nullptr
        // 所以 if 检查是无效的——target 指向已释放的内存
        if (target) {
            qDebug() << "  裸指针 Lambda 调用次数:" << callCount
                     << ", target->objectName():" << target->objectName()
                     << "(如果 target 已被 delete，这里是未定义行为!)";
        }
    });

    timer.start(50);

    // 第一次触发：target 存活
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    qDebug() << "  第一次触发完成，target 仍然存活";

    // 删除 target——Lambda 里的裸指针不知道对象已被删除
    delete target;

    qDebug() << "  target 已被 delete，但裸指针不知道";

    // 及时停止定时器，防止后续触发导致实际崩溃
    timer.stop();

    qDebug() << "  已停止定时器，避免实际崩溃";
    qDebug() << "";
}

void demoQPointerSafeCapture()
{
    qDebug() << "--- QPointer 安全捕获 ---";

    QObject* raw = new QObject();
    raw->setObjectName(QStringLiteral("SafeTarget"));

    // QPointer 是 Qt 提供的弱引用智能指针
    // 当它指向的 QObject 被 delete 后，QPointer 自动变为 nullptr
    QPointer<QObject> safePtr = raw;

    QTimer timer;

    // 安全：QPointer 捕获，对象销毁后 safePtr 变为 nullptr
    QObject::connect(&timer, &QTimer::timeout, [safePtr]() {
        if (safePtr) {
            qDebug() << "  QPointer Lambda: 对象存活, name:"
                     << safePtr->objectName();
        } else {
            qDebug() << "  QPointer Lambda: 对象已销毁, 安全跳过";
        }
    });

    timer.start(50);

    // 第一次触发：对象存活
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    qDebug() << "  第一次触发完成，对象存活";

    // 删除对象，safePtr 自动变为 nullptr
    delete raw;

    qDebug() << "  对象已 delete, QPointer isNull:" << safePtr.isNull();

    // 第二次触发：对象已销毁
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    qDebug() << "  第二次触发完成，安全跳过";

    timer.stop();
    qDebug() << "";
}
