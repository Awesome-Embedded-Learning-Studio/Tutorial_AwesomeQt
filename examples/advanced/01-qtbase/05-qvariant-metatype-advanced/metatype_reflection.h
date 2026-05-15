/// @file    metatype_reflection.h
/// @brief   QMetaType 反射能力与 QVariant 异构容器存储演示。
///
/// 演示 QMetaType::fromType<T>() 运行时类型查询、QVariantMap 异构存储。
/// 对应教程：进阶层 01-QtBase/05-QVariant 与 QMetaType。

#pragma once

/// @brief 演示 QMetaType 的运行时类型反射能力。
///
/// 展示 fromType<T>()、id()、name()、sizeOf()、metaObject() 等反射接口。
void demoMetaTypeReflection();

/// @brief 演示 QVariantMap 实现异构类型存储与类型安全提取。
///
/// 将不同类型的值存入 QVariantMap，再根据类型安全地逐个提取。
void demoVariantContainer();
