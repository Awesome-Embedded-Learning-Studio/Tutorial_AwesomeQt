/// @file    main.cpp
/// @brief   QString 深度性能与陷阱演示的入口。
///
/// 对应教程：进阶层 01-QtBase/03-QString 进阶。
/// 演示 Qt 字符串系统的高级特性，包括编码转换、QStringView 零拷贝传参、
/// QStringBuilder (%) 拼接、QStringLiteral 编译期优化、QElapsedTimer 高精度计时。

#include "encoding_demo.h"
#include "string_perf.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // ------------------------------------------------------------------
    // 演示 1: 编码转换——fromUtf8 / fromLatin1 / fromUtf16
    // ------------------------------------------------------------------
    qDebug() << "\n[演示 1] 编码转换与隐式开销";
    qDebug() << "----------------------------------------------";

    EncodingDemo::runAll();

    // ------------------------------------------------------------------
    // 演示 2: QStringView 参数传递——零拷贝
    // ------------------------------------------------------------------
    qDebug() << "\n[演示 2] QStringView 参数传递";
    qDebug() << "----------------------------------------------";

    // QStringView 可以从任何字符串类型构造，不产生堆分配
    // 适合作为只读字符串参数的统一函数签名
    StringPerf::demoQStringViewParameter();

    // ------------------------------------------------------------------
    // 演示 3: QStringBuilder (%) vs + 拼接性能对比
    // ------------------------------------------------------------------
    qDebug() << "\n[演示 3] QStringBuilder vs + 拼接性能";
    qDebug() << "----------------------------------------------";

    // % 运算符通过表达式模板延迟计算，只分配一次内存
    // + 运算符每次拼接都分配一个临时 QString
    StringPerf::demoQStringBuilderVsPlus();

    // ------------------------------------------------------------------
    // 演示 4: QStringLiteral vs 普通字面量
    // ------------------------------------------------------------------
    qDebug() << "\n[演示 4] QStringLiteral vs 普通字面量";
    qDebug() << "----------------------------------------------";

    // QStringLiteral 在编译期生成 UTF-16，运行时零分配
    // 普通字面量每次使用都触发 fromUtf8() 转换
    StringPerf::demoQStringLiteralPerf();

    // ------------------------------------------------------------------
    // 演示 5: QElapsedTimer 综合计时对比
    // ------------------------------------------------------------------
    qDebug() << "\n[演示 5] QElapsedTimer 综合计时";
    qDebug() << "----------------------------------------------";

    const int kBenchmarkCount = 1000;
    StringPerf::demoElapsedTimerComparison(kBenchmarkCount);

    // ------------------------------------------------------------------
    // 总结
    // ------------------------------------------------------------------
    qDebug() << "========================================";
    qDebug() << "QString 高级用法演示结束";
    qDebug() << "要点回顾:";
    qDebug() << "  1. 源文件 UTF-8 编码时用 fromUtf8，勿用 fromLatin1 避免乱码";
    qDebug() << "  2. 隐式 const char* -> QString 转换每次触发堆分配";
    qDebug() << "  3. QStringView 统一函数签名，传参零拷贝，但注意生命周期";
    qDebug() << "  4. QStringBuilder % 运算符延迟计算，只分配一次内存";
    qDebug() << "  5. QStringLiteral 编译期生成 UTF-16，循环中必须使用";
    qDebug() << "  6. QLatin1String 做比较零堆分配，比 fromLatin1 更高效";
    qDebug() << "========================================";

    return 0;
}
