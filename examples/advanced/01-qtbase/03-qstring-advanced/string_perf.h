/// @file    string_perf.h
/// @brief   演示 QStringView 参数传递、QStringBuilder (%) 拼接、QStringLiteral 性能。
///
/// 对应教程：进阶层 01-QtBase/03-QString 进阶。
/// 使用 QElapsedTimer 进行高精度计时，对比各种字符串操作的运行时开销。

#pragma once

class StringPerf
{
public:
    /// @brief 运行全部字符串性能演示。
    static void runAll();

    /// @brief 演示 QStringView 作为只读函数参数，避免不必要的 QString 构造。
    static void demoQStringViewParameter();

    /// @brief 演示 QStringBuilder (%) vs 传统 + 拼接的内存分配次数差异。
    static void demoQStringBuilderVsPlus();

    /// @brief 演示 QStringLiteral vs 普通字面量在循环中的开销差异。
    static void demoQStringLiteralPerf();

    /// @brief 使用 QElapsedTimer 对三种字符串构建方式做综合计时对比。
    /// @param[in] count 每种方式循环构建的字符串数量。
    static void demoElapsedTimerComparison(int count);
};
