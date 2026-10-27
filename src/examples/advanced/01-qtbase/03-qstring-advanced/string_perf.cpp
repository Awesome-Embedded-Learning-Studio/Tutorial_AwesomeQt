/// @file    string_perf.cpp
/// @brief   StringPerf 类的实现——字符串性能对比演示。
///
/// 对应教程：进阶层 01-QtBase/03-QString 进阶。

#include "string_perf.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QString>
#include <QStringBuilder>
#include <QStringView>

// ============================================================================
// StringPerf — 字符串性能对比
//
// 本文件提供以下性能对比：
// 1. QStringView vs QString 作为函数参数的传参开销
// 2. QStringBuilder (%) vs 传统 + 拼接的内存分配次数
// 3. QStringLiteral vs 普通字面量在循环中的开销
//
// 使用 QElapsedTimer 进行高精度计时。
// ============================================================================

void StringPerf::runAll()
{
    demoQStringViewParameter();
    demoQStringBuilderVsPlus();
    demoQStringLiteralPerf();
}

void StringPerf::demoQStringViewParameter()
{
    qDebug() << "=== QStringView 参数传递 ===";

    // 这个 lambda 接受 QStringView，可以从任何字符串类型构造
    // 不会产生额外的堆分配
    auto printLength = [](QStringView str) {
        return str.length();
    };

    // 从 QString 构造 QStringView——零拷贝
    QString qstr = QStringLiteral("Hello QStringView");
    qDebug() << "从 QString 构造 QStringView, length:" << printLength(qstr);

    // 从 char16_t 字面量构造 QStringView——零开销
    qDebug() << "从 u\"\" 构造 QStringView, length:"
             << printLength(QStringView(u"raw C string"));

    // 从 char16_t 字面量构造 QStringView——零开销
    qDebug() << "从 u\"\" 构造 QStringView, length:"
             << printLength(u"char16_t literal");

    // 从 QLatin1String 构造 QStringView——需要显式转换
    qDebug() << "从 QLatin1String 构造 QStringView, length:"
             << printLength(QStringView(u"Latin1 string"));

    qDebug() << "";
}

void StringPerf::demoQStringBuilderVsPlus()
{
    qDebug() << "=== QStringBuilder (%) vs + 拼接性能对比 ===";

    const int kIterations = 10000;

    // 方式 1: 传统 + 拼接
    // 每次 + 都会创建一个临时 QString，分配一次内存
    {
        QElapsedTimer timer;
        timer.start();
        QString result;
        for (int i = 0; i < kIterations; ++i) {
            // 每个 + 创建一个临时 QString 对象
            result = QString("prefix_") + QString::number(i)
                     + QString("_middle_") + QString("suffix");
        }
        qint64 plusNanos = timer.nsecsElapsed();
        qDebug() << "传统 + 拼接:" << kIterations << "次, 耗时:"
                 << plusNanos / 1000 << "us"
                 << "(每次拼接多次堆分配)";
    }

    // 方式 2: QStringBuilder % 拼接
    // % 运算符通过表达式模板延迟计算，最终只分配一次内存
    {
        QElapsedTimer timer;
        timer.start();
        QString result;
        for (int i = 0; i < kIterations; ++i) {
            // % 不会立刻构造 QString，而是积攒起来最后一次性分配
            result = QStringLiteral("prefix_") % QString::number(i)
                     % QStringLiteral("_middle_") % QStringLiteral("suffix");
        }
        qint64 builderNanos = timer.nsecsElapsed();
        qDebug() << "QStringBuilder % 拼接:" << kIterations << "次, 耗时:"
                 << builderNanos / 1000 << "us"
                 << "(每次拼接只分配一次内存)";
    }

    qDebug() << "";
}

void StringPerf::demoQStringLiteralPerf()
{
    qDebug() << "=== QStringLiteral vs 普通字面量 ===";

    const int kIterations = 50000;

    // 普通字面量：每次循环都调用 fromUtf8() 构造临时 QString
    {
        QElapsedTimer timer;
        timer.start();
        qsizetype totalLen = 0;
        for (int i = 0; i < kIterations; ++i) {
            // "test_string" 是 const char*，每次都隐式构造 QString
            QString s = "test_string";
            totalLen += s.length();
        }
        qint64 plainNanos = timer.nsecsElapsed();
        qDebug() << "普通字面量:" << kIterations << "次, 耗时:"
                 << plainNanos / 1000 << "us"
                 << ", totalLen:" << totalLen;
    }

    // QStringLiteral：编译期生成 UTF-16，运行时零分配
    {
        QElapsedTimer timer;
        timer.start();
        qsizetype totalLen = 0;
        for (int i = 0; i < kIterations; ++i) {
            // QStringLiteral 在编译期就准备好了 UTF-16 数据
            // 运行时只是构造 QString 指向静态数据
            QString s = QStringLiteral("test_string");
            totalLen += s.length();
        }
        qint64 literalNanos = timer.nsecsElapsed();
        qDebug() << "QStringLiteral:" << kIterations << "次, 耗时:"
                 << literalNanos / 1000 << "us"
                 << ", totalLen:" << totalLen;
    }

    qDebug() << "";
}

void StringPerf::demoElapsedTimerComparison(int count)
{
    qDebug() << "=== QElapsedTimer 综合计时 ===";

    // QElapsedTimer 提供纳秒级精度的计时
    // 适用于微基准测试（micro-benchmarking）
    QElapsedTimer timer;

    // 方式 A: 普通拼接
    timer.start();
    for (int i = 0; i < count; ++i) {
        QString s = "id_" + QString::number(i) + "_end";
        Q_UNUSED(s)
    }
    qint64 timeA = timer.nsecsElapsed();

    // 方式 B: QStringLiteral + %
    timer.restart();
    for (int i = 0; i < count; ++i) {
        QString s = QStringLiteral("id_") % QString::number(i)
                    % QStringLiteral("_end");
        Q_UNUSED(s)
    }
    qint64 timeB = timer.nsecsElapsed();

    // 方式 C: QString::arg()
    timer.restart();
    for (int i = 0; i < count; ++i) {
        QString s = QStringLiteral("id_%1_end").arg(i);
        Q_UNUSED(s)
    }
    qint64 timeC = timer.nsecsElapsed();

    qDebug() << "构建" << count << "个字符串的耗时对比:";
    qDebug() << "  普通 + 拼接:          " << timeA / 1000 << "us";
    qDebug() << "  QStringLiteral + %:   " << timeB / 1000 << "us";
    qDebug() << "  QStringLiteral + arg: " << timeC / 1000 << "us";
    qDebug() << "";
}
