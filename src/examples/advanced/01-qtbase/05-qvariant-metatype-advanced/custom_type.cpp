/// @file    custom_type.cpp
/// @brief   SensorData 结构体实现及自定义类型注册/存取/转换演示。
///
/// 对应教程：进阶层 01-QtBase/05-QVariant 与 QMetaType。

#include "custom_type.h"

// ---------------------------------------------------------------------------
// SensorData 实现
// ---------------------------------------------------------------------------

SensorData::SensorData()
    : m_sensorId(0)
    , m_value(0.0)
{
}

SensorData::SensorData(int id, double val, const QDateTime& ts, const QString& u)
    : m_sensorId(id)
    , m_value(val)
    , m_timestamp(ts)
    , m_unit(u)
{
}

bool SensorData::operator==(const SensorData& other) const
{
    return m_sensorId == other.m_sensorId && qFuzzyCompare(m_value, other.m_value)
           && m_timestamp == other.m_timestamp && m_unit == other.m_unit;
}

// ---------------------------------------------------------------------------
// 演示 1: 自定义类型注册
// ---------------------------------------------------------------------------
void demoCustomTypeRegistration()
{
    qDebug() << "\n=== 自定义类型注册 ===";

    // Q_DECLARE_METATYPE 让类型可以在编译时与 QVariant 关联
    // 但要在跨线程信号槽中使用，还需要运行时注册
    int typeId = qRegisterMetaType<SensorData>("SensorData");
    qDebug() << "注册 SensorData，类型 ID:" << typeId;

    qDebug() << "SensorData 是否已注册:" << QMetaType::isRegistered(typeId);
    qDebug() << "类型名称:" << QMetaType(typeId).name();
    qDebug() << "类型大小:" << QMetaType(typeId).sizeOf() << "字节";
}

// ---------------------------------------------------------------------------
// 演示 2: QVariant 存取自定义类型
// ---------------------------------------------------------------------------
void demoVariantStoreAndRetrieve()
{
    qDebug() << "\n=== QVariant 存取自定义类型 ===";

    SensorData original(42, 23.5, QDateTime::currentDateTime(), QString::fromUtf8("°C"));

    // 方式 1: QVariant::fromValue() 封装
    QVariant variant = QVariant::fromValue(original);
    qDebug() << "QVariant 包含类型:" << variant.typeName();
    qDebug() << "QVariant 是否可转换为 SensorData:" << variant.canConvert<SensorData>();

    // 方式 2: qvariant_cast<T> 提取（推荐，类型安全）
    SensorData retrieved = qvariant_cast<SensorData>(variant);
    qDebug() << "传感器 ID:" << retrieved.m_sensorId;
    qDebug() << "传感器值:" << retrieved.m_value << retrieved.m_unit;
    qDebug() << "时间戳:" << retrieved.m_timestamp.toString(Qt::ISODate);

    // 方式 3: QVariant::value<T>() 提取（等价于 qvariant_cast）
    SensorData retrieved2 = variant.value<SensorData>();
    qDebug() << "value<T>() 方式提取，ID:" << retrieved2.m_sensorId;

    // QVariant 保存的是副本（值语义），修改原始数据不影响已存储的值
    original.m_sensorId = 99;
    SensorData fromVariant = qvariant_cast<SensorData>(variant);
    qDebug() << "修改原始数据后 QVariant 中的 ID 仍为:" << fromVariant.m_sensorId;
}

// ---------------------------------------------------------------------------
// 演示 3: 类型转换检查
// ---------------------------------------------------------------------------
void demoTypeConversion()
{
    qDebug() << "\n=== 类型转换检查 ===";

    QVariant intVar(42);
    qDebug() << "int -> QString: canConvert =" << intVar.canConvert<QString>()
             << "，转换结果:" << intVar.toString();
    qDebug() << "int -> double: canConvert =" << intVar.canConvert<double>()
             << "，转换结果:" << intVar.toDouble();

    QVariant strVar("3.14159");
    qDebug() << "QString -> double: canConvert =" << strVar.canConvert<double>()
             << "，转换结果:" << strVar.toDouble();
    qDebug() << "QString -> int: canConvert =" << strVar.canConvert<int>()
             << "，转换结果:" << strVar.toInt();

    QVariant testVar("hello");
    qDebug() << "字符串\"hello\" -> int: canConvert =" << testVar.canConvert<int>()
             << "（convert 已废弃，建议使用 value<T>() 或 toInt()）";

    // 自定义类型无法转换为基本类型
    SensorData data(1, 0.0, QDateTime(), "m");
    QVariant customVar = QVariant::fromValue(data);
    qDebug() << "SensorData -> QString: canConvert =" << customVar.canConvert<QString>();
}
