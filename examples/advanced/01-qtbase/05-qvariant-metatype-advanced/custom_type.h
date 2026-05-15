/// @file    custom_type.h
/// @brief   自定义传感器数据类型及其 QVariant 注册与存取演示。
///
/// 演示 Q_DECLARE_METATYPE、qRegisterMetaType、qvariant_cast<T> 等进阶用法。
/// 对应教程：进阶层 01-QtBase/05-QVariant 与 QMetaType。

#pragma once

#include <QDateTime>
#include <QDebug>
#include <QMetaType>
#include <QString>
#include <QVariant>

/// 自定义传感器数据结构体，用于演示 QVariant 包装自定义类型。
struct SensorData
{
    int m_sensorId;          ///< 传感器编号
    double m_value;          ///< 传感器读数
    QDateTime m_timestamp;   ///< 采样时间
    QString m_unit;          ///< 单位（如 "°C"、"m"）

    /// @brief 默认构造函数（QMetaType 要求类型可默认构造）。
    SensorData();

    /// @brief 带参构造函数。
    /// @param[in] id   传感器编号。
    /// @param[in] val  传感器读数。
    /// @param[in] ts   采样时间。
    /// @param[in] u    单位。
    SensorData(int id, double val, const QDateTime& ts, const QString& u);

    /// @brief 相等比较运算符，用于 QVariant 中的值比对。
    /// @param[in] other 另一个 SensorData 对象。
    /// @return 两对象所有字段是否相等（浮点值使用 qFuzzyCompare）。
    bool operator==(const SensorData& other) const;
};

// 必须放在全局命名空间中，类型声明之后
Q_DECLARE_METATYPE(SensorData)

// ---------------------------------------------------------------------------
// 演示函数声明
// ---------------------------------------------------------------------------

/// @brief 演示自定义类型的元类型注册与运行时信息查询。
void demoCustomTypeRegistration();

/// @brief 演示 QVariant 存取自定义类型的多种方式。
void demoVariantStoreAndRetrieve();

/// @brief 演示 QVariant 内置类型间的转换能力与 canConvert/value<T> 用法。
void demoTypeConversion();
