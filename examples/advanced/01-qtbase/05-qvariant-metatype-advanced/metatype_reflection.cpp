/// @file    metatype_reflection.cpp
/// @brief   QMetaType 反射与 QVariant 异构容器演示的实现。
///
/// 对应教程：进阶层 01-QtBase/05-QVariant 与 QMetaType。

#include "metatype_reflection.h"

#include "custom_type.h"

#include <QDebug>
#include <QMetaObject>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QVariant>

// ---------------------------------------------------------------------------
// 演示 4: QMetaType 反射能力
// ---------------------------------------------------------------------------
void demoMetaTypeReflection()
{
    qDebug() << "\n=== QMetaType 反射能力 ===";

    // Qt6 推荐使用 fromType<T>() 获取 QMetaType 实例
    QMetaType intMeta = QMetaType::fromType<int>();
    QMetaType strMeta = QMetaType::fromType<QString>();

    qDebug() << "int 的类型 ID:" << intMeta.id();
    qDebug() << "QString 的类型 ID:" << strMeta.id();

    qDebug() << "int 是否有效:" << intMeta.isValid();
    qDebug() << "QString 是否有效:" << strMeta.isValid();

    qDebug() << "int 类型名:" << intMeta.name();
    qDebug() << "QString 类型名:" << strMeta.name();

    qDebug() << "int 大小:" << intMeta.sizeOf() << "字节";
    qDebug() << "QString 大小:" << strMeta.sizeOf() << "字节";

    // 判断是否为内置类型（ID 小于 QMetaType::User）
    qDebug() << "int 是内置类型:" << (intMeta.id() < QMetaType::User);
    qDebug() << "用户类型起始 ID:" << QMetaType::User;

    // QMetaType 可以检查是否关联了 QMetaObject（仅 QObject 派生类型）
    QMetaType objMeta = QMetaType::fromType<QObject*>();
    if (const QMetaObject* mo = objMeta.metaObject())
    {
        qDebug() << "QObject* 的元对象类名:" << mo->className();
    }
}

// ---------------------------------------------------------------------------
// 演示 5: QVariant 容器与异构存储
// ---------------------------------------------------------------------------
void demoVariantContainer()
{
    qDebug() << "\n=== QVariant 容器与异构存储 ===";

    QVariantMap config;
    config["appName"] = "MyApp";
    config["version"] = 2.0;
    config["debug"] = true;
    config["maxRetries"] = 3;

    SensorData sensor(1, 36.5, QDateTime::currentDateTime(), QString::fromUtf8("°C"));
    config["lastSensorReading"] = QVariant::fromValue(sensor);

    // 遍历 QVariantMap，展示异构存储效果
    qDebug() << "配置项列表:";
    for (auto it = config.constBegin(); it != config.constEnd(); ++it)
    {
        qDebug() << "  " << it.key() << "=" << it.value()
                 << "(类型:" << it.value().typeName() << ")";
    }

    // 类型安全的提取
    if (config["debug"].canConvert<bool>())
    {
        bool debug = config["debug"].toBool();
        qDebug() << "\ndebug 模式:" << debug;
    }

    if (config["lastSensorReading"].canConvert<SensorData>())
    {
        SensorData s = config["lastSensorReading"].value<SensorData>();
        qDebug() << "传感器读数:" << s.m_value << s.m_unit;
    }
}
