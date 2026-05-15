/// @file    reflective_demo.h
/// @brief   反射演示辅助函数声明。
///
/// 提供 runReflectiveDemos() 等函数，演示如何使用 QMetaObject / QMetaProperty
/// 在运行时对任意 QObject 进行属性枚举、能力检测和元数据读取。
///
/// 对应教程：进阶层 01-QtBase/01-QObject 属性系统深度拆解。

#pragma once

#include <QObject>

/// @brief 遍历对象的所有 Q_PROPERTY（包括继承的），打印属性名称、类型和能力标签。
/// @param[in] obj 要枚举属性的目标对象。
void printAllProperties(const QObject* obj);

/// @brief 只遍历本类自身声明的属性（从 propertyOffset 开始），跳过继承的。
/// @param[in] obj 要枚举属性的目标对象。
void printOwnProperties(const QObject* obj);

/// @brief 逐个检测属性能力，展示 QMetaProperty 的布尔查询函数。
/// @param[in] obj 要检测属性的目标对象。
void checkPropertyCapabilities(const QObject* obj);

/// @brief 读取 Q_CLASSINFO 元数据并打印。
/// @param[in] obj 要读取元数据的目标对象。
void printClassInfo(const QObject* obj);
