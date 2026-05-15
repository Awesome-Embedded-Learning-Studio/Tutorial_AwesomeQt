/// @file    file_ops_demo.h
/// @brief   演示 QSaveFile 原子写入、QFile::map() 内存映射、QFileSystemWatcher 文件监控。
///
/// 对应教程：进阶层 01-QtBase/08-文件与 IO 进阶。

#pragma once

#include <QString>

// 前向声明，避免在头文件中引入大量 Qt 内部头文件
class QFileSystemWatcher;

// ---------------------------------------------------------------------------
// 演示 QSaveFile 原子写入
// ---------------------------------------------------------------------------

/// @brief 演示 QSaveFile 的原子写入和取消写入机制。
/// @param[in] dirPath 用于存放测试文件的目录路径。
void demoAtomicWrite(const QString& dirPath);

// ---------------------------------------------------------------------------
// 演示 QFile::map() 内存映射
// ---------------------------------------------------------------------------

/// @brief 演示 QFile::map() 将文件映射到内存进行高效读取。
/// @param[in] dirPath 用于存放测试文件的目录路径。
void demoMemoryMappedFile(const QString& dirPath);

// ---------------------------------------------------------------------------
// 演示 QFileSystemWatcher 文件监控
// ---------------------------------------------------------------------------

/// @brief 演示 QFileSystemWatcher 监控文件和目录的变化事件。
/// @param[in] dirPath 需要监控的目录路径。
void demoFileSystemWatcher(const QString& dirPath);
