/// @file    main.cpp
/// @brief   QTimer 高级示例的程序入口，依次调用各演示函数。
///
/// 对应教程：进阶层 01-QtBase/11-QTimer 高级。
///
/// 演示内容：
/// 1. QTimer 三种精度类型对比（PreciseTimer/CoarseTimer/VeryCoarseTimer）
/// 2. QElapsedTimer 纳秒级性能计时
/// 3. QDeadlineTimer 截止时间超时控制
/// 4. 0ms QTimer 零定时器延迟执行模式
/// 5. QBasicTimer 轻量级定时器

#include "elapsed_timer_demo.h"
#include "precision_demo.h"

#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QCoreApplication>

/// @brief 演示 0ms QTimer 零定时器延迟执行。
///
/// 将 QTimer 间隔设为 0ms 时，定时器会在下一次事件循环迭代时
/// 立即触发。这种"零定时器"模式常用于：
/// - 延迟初始化：在事件循环启动后再执行初始化操作
/// - 分批处理：将大量工作分成小块，每块之间处理事件
/// - 确保 UI 更新后再执行操作
///
/// 0ms 定时器的优先级低于普通事件，因此不会阻塞其他事件处理。
static void demo4_zeroTimer()
{
    qDebug() << "\n[演示 4] 0ms QTimer 零定时器延迟执行";

    // 零定时器：立即返回，在下一次事件循环迭代时触发
    int triggerCount = 0;
    QEventLoop loop;

    qDebug() << "[零定时器] 设置 0ms 定时器（立即返回当前函数）";

    QTimer::singleShot(0, [&]() {
        triggerCount++;
        qDebug() << "[零定时器] 第 1 次触发 - 这是在事件循环的下一次迭代中执行的";

        // 链式零定时器：每次触发后再注册一个零定时器
        QTimer::singleShot(0, [&]() {
            triggerCount++;
            qDebug() << "[零定时器] 第 2 次触发 - 链式延迟执行";

            QTimer::singleShot(0, [&]() {
                triggerCount++;
                qDebug() << "[零定时器] 第 3 次触发 - 三级链式延迟";
                qDebug() << "[零定时器] 总共触发" << triggerCount << "次";
                loop.quit();
            });
        });
    });

    qDebug() << "[零定时器] 进入事件循环...";
    loop.exec();
    qDebug() << "[零定时器] 演示结束";

    // 演示重复零定时器：实现协作式分批处理
    qDebug() << "\n[零定时器] 分批处理模式演示";
    int totalItems = 10;
    int processed = 0;

    QTimer batchTimer;
    batchTimer.setInterval(0);  // 0ms = 每次事件循环迭代都触发

    QObject::connect(&batchTimer, &QTimer::timeout, &batchTimer, [&]() {
        // 每次处理 3 个项目
        int batchSize = qMin(3, totalItems - processed);
        for (int i = 0; i < batchSize; ++i) {
            processed++;
            qDebug() << "[零定时器] 处理项目" << processed << "/" << totalItems;
        }

        if (processed >= totalItems) {
            batchTimer.stop();
            qDebug() << "[零定时器] 分批处理完成";
        }
    });

    batchTimer.start();

    // 等待处理完成
    QEventLoop loop2;
    QObject::connect(&batchTimer, &QTimer::timeout, &loop2, [&]() {
        if (!batchTimer.isActive()) {
            loop2.quit();
        }
    });
    loop2.exec();
}

/// @brief 程序入口，依次运行所有定时器高级演示。
/// @param[in] argc 命令行参数个数。
/// @param[in] argv 命令行参数数组。
/// @return 程序退出码。
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========== QTimer 高级示例 ==========";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;

    // ================================================================
    // [演示 1] QTimer 三种精度类型对比
    // ================================================================
    qDebug() << "\n[演示 1] QTimer 三种精度类型对比";
    demoTimerPrecisionComparison();

    // ================================================================
    // [演示 2] QElapsedTimer 纳秒级性能计时
    // ================================================================
    qDebug() << "\n[演示 2] QElapsedTimer 纳秒级性能计时";
    demoElapsedTimer();

    // ================================================================
    // [演示 3] QDeadlineTimer 截止时间超时控制
    // ================================================================
    qDebug() << "\n[演示 3] QDeadlineTimer 截止时间超时控制";
    demoDeadlineTimer();

    // ================================================================
    // [演示 4] 0ms QTimer 零定时器延迟执行
    // ================================================================
    demo4_zeroTimer();

    // ================================================================
    // [演示 5] QBasicTimer 轻量级定时器
    // ================================================================
    qDebug() << "\n[演示 5] QBasicTimer 轻量级定时器";
    demoBasicTimer();

    qDebug() << "\n========== 所有演示执行完毕 ==========";

    // 使用 QTimer::singleShot 延迟退出
    QTimer::singleShot(0, &app, &QCoreApplication::quit);

    return app.exec();
}
