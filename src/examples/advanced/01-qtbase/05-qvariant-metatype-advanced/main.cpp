/// @file    main.cpp
/// @brief   程序入口，串联 QVariant 与 QMetaType 的全部演示。
///
/// 对应教程：进阶层 01-QtBase/05-QVariant 与 QMetaType。

#include "custom_type.h"
#include "metatype_reflection.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========== QVariant 与 QMetaType 进阶示例 ==========\n";

    // 演示 1: 自定义类型注册
    qDebug() << "[演示 1] 自定义类型注册";
    demoCustomTypeRegistration();

    // 演示 2: QVariant 存取自定义类型
    qDebug() << "\n[演示 2] QVariant 存取自定义类型";
    demoVariantStoreAndRetrieve();

    // 演示 3: 类型转换检查
    qDebug() << "\n[演示 3] QVariant 类型转换检查";
    demoTypeConversion();

    // 演示 4: QMetaType 反射能力
    qDebug() << "\n[演示 4] QMetaType 反射能力";
    demoMetaTypeReflection();

    // 演示 5: QVariant 容器与异构存储
    qDebug() << "\n[演示 5] QVariant 容器与异构存储";
    demoVariantContainer();

    qDebug() << "\n===========================================";
    qDebug() << "要点总结:";
    qDebug() << "  - Q_DECLARE_METATYPE 让自定义类型可用于 QVariant";
    qDebug() << "  - qRegisterMetaType 用于跨线程信号槽传递";
    qDebug() << "  - qvariant_cast<T> 提供类型安全的提取";
    qDebug() << "  - QMetaType 提供运行时类型反射能力";

    return 0;
}
