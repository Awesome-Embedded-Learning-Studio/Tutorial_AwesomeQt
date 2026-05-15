/// @file    localize_demo.h
/// @brief   QLocale 格式化演示函数声明。
///
/// 提供 runLocaleDemo() 入口，演示 QLocale 的数字、日期时间、货币格式化
/// 以及区域信息查询。QLocale 不依赖 .qm 翻译文件，可直接使用 Qt 内置的区域数据。
///
/// 对应教程：进阶层 01-QtBase/13-国际化。

#pragma once

/// @brief 综合运行所有 QLocale 格式化演示。
void runLocaleDemo();
