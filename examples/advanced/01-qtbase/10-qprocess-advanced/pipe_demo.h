/// @file    pipe_demo.h
/// @brief   声明 QProcess 管道连接、崩溃检测与超时管理的演示函数。
///
/// 对应教程：进阶层 01-QtBase/10-QProcess 高级用法。
/// 演示 QProcess 的高级特性：
/// - 进程管道连接（类似 shell 的 | 管道操作）
/// - 进程崩溃检测（errorOccurred + exitStatus）
/// - 超时管理（terminate -> kill 两阶段终止）

#pragma once

/// @brief 演示进程管道连接：ls | grep 模式。
/// @note setStandardOutputProcess() 将第一个进程的 stdout 直接连接到第二个进程的 stdin，
///       数据通过操作系统管道传递，无需经过用户空间拷贝。
void demoProcessPipe();

/// @brief 演示进程崩溃检测。
/// @note QProcess 提供 errorOccurred 信号和 exitStatus() 方法来检测进程异常。
///       正常退出返回 NormalExit，崩溃返回 CrashExit。
void demoCrashDetection();

/// @brief 演示进程超时管理：terminate -> kill 两阶段终止。
/// @note terminate() 发送 SIGTERM（礼貌终止请求），进程可以捕获并做清理。
///       如果进程在一定时间内没有退出，再调用 kill() 强制终止（不可捕获）。
void demoTimeoutManagement();
