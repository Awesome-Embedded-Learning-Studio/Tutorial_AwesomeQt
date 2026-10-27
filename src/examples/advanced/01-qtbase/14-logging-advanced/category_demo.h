/// @file    category_demo.h
/// @brief   分类日志演示声明。
///
/// 提供 runCategoryDemo() 入口，演示 Q_DECLARE_LOGGING_CATEGORY、qCDebug/qCWarning
/// 等分类日志宏的用法，以及 setFilterRules 运行时控制日志开关和 Release 模式策略。
///
/// 对应教程：进阶层 01-QtBase/14-日志。

#pragma once

/// @brief 综合运行分类日志、过滤规则与 Release 策略演示。
void runCategoryDemo();
