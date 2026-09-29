/// @file    cow_demo.h
/// @brief   演示 Qt 容器的 COW（隐式共享）机制与 STL 算法互操作。
///
/// 对应教程：进阶层 01-QtBase/04-容器（高级）。
/// 涵盖 detach 触发条件、std::as_const() 防深拷贝、STL 算法在 QList 上的使用，
/// 以及 QList/QVector 互操作。

#pragma once

#include <QList>
#include <QVector>

// 演示 const vs 非 const 访问触发/避免 detach
void demoDetachBehavior();

// 演示 std::as_const() 防止意外深拷贝
void demoStdAsConst();

// 演示 STL 算法在 QList 上的使用
void demoStlAlgorithms();

// 演示 QList 和 QVector 的互操作
void demoQListQVectorInterop();
