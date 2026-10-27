/// @file    custom_handler.h
/// @brief   自定义消息处理器声明。
///
/// 提供 demoCustomHandler() 入口，演示 qInstallMessageHandler 的完整实现，
/// 包括格式化输出（时间戳、级别、文件名:行号）、同时输出到控制台和日志文件、
/// 以及处理器的安装与恢复。
///
/// 对应教程：进阶层 01-QtBase/14-日志。

#pragma once

/// @brief 演示自定义消息处理器的安装、使用与恢复。
void demoCustomHandler();
