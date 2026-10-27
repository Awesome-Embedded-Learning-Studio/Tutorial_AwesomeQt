/// @file    concurrent_demo.h
/// @brief   演示 QtConcurrent::run() 异步执行与 QFuture/QFutureWatcher 监控。
///
/// 对应教程：进阶层 01-QtBase/09-多线程。
/// 展示 QtConcurrent 高层并发 API 的三种典型用法：
/// 异步任务提交、QFutureWatcher 信号监控、多任务并行等待。

#pragma once

#include <QElapsedTimer>
#include <QEventLoop>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

/// @brief 演示 QtConcurrent::run() 基础异步执行。
///
/// 展示无返回值任务、带返回值任务和带参数任务三种用法。
/// QFuture::result() 会阻塞直到结果就绪。
void demoConcurrentRun();

/// @brief 演示 QFutureWatcher 监控异步结果完成。
///
/// QFutureWatcher 将 QFuture 状态变化转为 Qt 信号，可在事件循环中
/// 异步接收完成通知，无需阻塞等待，适合 GUI 应用。
void demoFutureWatcher();

/// @brief 演示多个并发任务的同时执行与等待。
///
/// 同时提交多个独立任务，并行执行后逐个等待结果。
/// 总耗时接近最慢的单个任务，而非串行累加。
void demoMultipleConcurrent();
