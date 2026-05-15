/// @file    connection_demo.h
/// @brief   Qt::ConnectionType 演示函数声明。
///
/// 提供 demoDirectConnection / demoQueuedConnection / demoBlockingQueuedConnection
/// 三个函数，分别演示不同连接类型的线程行为差异。
///
/// 对应教程：进阶层 01-QtBase/02-信号与槽工程级深度剖析。

#pragma once

/// @brief 演示 DirectConnection：槽函数在信号发射的线程同步执行。
void demoDirectConnection();

/// @brief 演示 QueuedConnection：槽函数在接收者线程事件循环中异步执行。
void demoQueuedConnection();

/// @brief 演示 BlockingQueuedConnection：跨线程同步获取结果。
/// @note 要求 sender 和 receiver 在不同线程，否则会死锁。
void demoBlockingQueuedConnection();
