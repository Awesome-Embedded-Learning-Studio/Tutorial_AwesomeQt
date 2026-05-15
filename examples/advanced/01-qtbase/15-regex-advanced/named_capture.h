/// @file    named_capture.h
/// @brief   命名捕获组演示声明。
///
/// 提供 runNamedCaptureDemo() 入口，演示 QRegularExpression 的命名捕获组
/// (?P<name>...) 语法、captured("name") 按名提取、capturedView("name") 零拷贝提取，
/// 以及 PatternOptions 常用选项。
///
/// 对应教程：进阶层 01-QtBase/15-正则与文本处理。

#pragma once

/// @brief 综合运行所有命名捕获组与 PatternOptions 演示。
void runNamedCaptureDemo();
