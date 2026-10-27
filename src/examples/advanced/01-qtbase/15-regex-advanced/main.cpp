/// @file    main.cpp
/// @brief   15-regex-advanced 示例程序入口。
///
/// 演示 QRegularExpression 的高级功能：
/// 1. 命名捕获组 (?P<name>...) + captured("name") 按名提取
/// 2. optimize() JIT 预编译性能对比
/// 3. globalMatch() + QRegularExpressionMatchIterator 全局匹配
/// 4. setMatchTimeout() 防止灾难性回溯（ReDoS）
/// 5. PatternOptions：CaseInsensitive, Multiline, DotMatchesEverything 等
///
/// 对应教程：进阶层 01-QtBase/15-正则与文本处理。

#include <QDebug>                        // 调试输出
#include <QCoreApplication>              // 核心应用类（非 GUI）
#include <QElapsedTimer>                 // 高精度计时器
#include <QRegularExpression>            // 正则表达式

#include "named_capture.h"               // 命名捕获组演示
#include "regex_perf.h"                  // 性能与全局匹配演示

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("regex-advanced-demo");
    QCoreApplication::setApplicationVersion("1.0");

    qDebug() << "========================================";
    qDebug() << "  Qt6 正则表达式高级示例";
    qDebug() << "========================================";
    qDebug() << "  Qt 版本:" << QT_VERSION_STR;
    qDebug() << "  正则引擎: PCRE2 (Perl Compatible Regular Expressions)";
    qDebug() << "";

    // 演示 1：命名捕获组
    qDebug() << "\n[演示 1] 命名捕获组 (?P<name>...)";
    qDebug() << "========================================";
    runNamedCaptureDemo();

    // 演示 2-3：性能优化与全局匹配
    qDebug() << "\n[演示 2] JIT 优化与全局匹配";
    qDebug() << "========================================";
    runRegexPerfDemo();

    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << "  所有演示执行完毕";
    qDebug() << "========================================";

    return 0;
}
