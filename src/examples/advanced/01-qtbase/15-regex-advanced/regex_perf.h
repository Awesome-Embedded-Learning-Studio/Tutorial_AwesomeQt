/// @file    regex_perf.h
/// @brief   正则表达式性能与高级匹配演示声明。
///
/// 提供 runRegexPerfDemo() 入口，演示 optimize() JIT 预编译性能优化、
/// globalMatch() + QRegularExpressionMatchIterator 全局匹配，
/// 以及 setMatchTimeout() 防止灾难性回溯（ReDoS 攻击防护）。
///
/// 对应教程：进阶层 01-QtBase/15-正则与文本处理。

#pragma once

/// @brief 综合运行 JIT 优化、全局匹配与灾难性回溯防护演示。
void runRegexPerfDemo();
