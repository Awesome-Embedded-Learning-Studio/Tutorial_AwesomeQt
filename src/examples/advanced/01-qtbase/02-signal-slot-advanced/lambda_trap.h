/// @file    lambda_trap.h
/// @brief   Lambda 捕获陷阱演示函数声明。
///
/// 演示裸指针捕获 vs QPointer 安全捕获两种方式，
/// 以及四参数 connect 让 context 对象析构时自动断开连接。
///
/// 对应教程：进阶层 01-QtBase/02-信号与槽工程级深度剖析。

#pragma once

/// @brief 演示裸指针捕获的危险——对象 delete 后信号触发野指针访问。
/// @note 安全版本，定时器及时停止，不会实际崩溃。
void demoRawPointerTrap();

/// @brief 演示 QPointer 安全捕获——对象销毁后自动置 nullptr。
void demoQPointerSafeCapture();
