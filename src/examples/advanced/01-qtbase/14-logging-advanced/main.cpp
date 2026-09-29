/// @file    main.cpp
/// @brief   14-logging-advanced 示例程序入口。
///
/// 演示 Qt 日志系统的高级用法：
/// 1. qInstallMessageHandler 自定义消息处理器（格式化时间戳、级别、文件:行号）
/// 2. Q_DECLARE_LOGGING_CATEGORY + qCDebug/qCWarning/qCCritical 分类日志
/// 3. setFilterRules / QT_LOGGING_RULES 环境变量控制日志开关
/// 4. Release 模式策略：保留 qWarning，禁用 qDebug
///
/// 对应教程：进阶层 01-QtBase/14-日志。

#include <QDebug>                // 调试输出
#include <QCoreApplication>      // 核心应用类（非 GUI）
#include <QLoggingCategory>      // 日志类别

#include "category_demo.h"       // 分类日志演示
#include "custom_handler.h"      // 自定义消息处理器

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("logging-advanced-demo");
    QCoreApplication::setApplicationVersion("1.0");

    qDebug() << "========================================";
    qDebug() << "  Qt6 日志系统高级示例";
    qDebug() << "========================================";
    qDebug() << "  Qt 版本:" << QT_VERSION_STR;
    qDebug() << "";

    // 演示 1：自定义消息处理器
    qDebug() << "\n[演示 1] 自定义消息处理器";
    qDebug() << "========================================";
    demoCustomHandler();

    // 演示 2-4：分类日志与过滤规则
    runCategoryDemo();

    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << "  所有演示执行完毕";
    qDebug() << "========================================";

    return 0;
}
