/// @file    reflective_demo.cpp
/// @brief   反射演示辅助函数实现。
///
/// 对应教程：进阶层 01-QtBase/01-QObject 属性系统深度拆解。

#include "reflective_demo.h"

#include <QDebug>
#include <QMetaClassInfo>
#include <QMetaObject>
#include <QMetaProperty>

#include <QStringList>

void printAllProperties(const QObject* obj)
{
    const QMetaObject* metaObj = obj->metaObject();

    qDebug() << "=== 属性枚举 ===";
    qDebug() << "类名:" << metaObj->className();
    qDebug() << "总属性数:" << metaObj->propertyCount()
             << ", 本类属性起始偏移:" << metaObj->propertyOffset();
    qDebug() << "";

    // 从 0 开始遍历所有属性（包括从 QObject 继承的 objectName）
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        qDebug() << "  [" << i << "]" << prop.name()
                 << "类型:" << prop.typeName();

        QStringList tags;
        if (prop.isReadable()) {
            tags << QStringLiteral("可读");
        }
        if (prop.isWritable()) {
            tags << QStringLiteral("可写");
        }
        if (prop.isResettable()) {
            tags << QStringLiteral("可重置");
        }
        if (prop.hasNotifySignal()) {
            tags << QStringLiteral("有通知信号");
        }
        if (prop.isConstant()) {
            tags << QStringLiteral("CONSTANT");
        }
        if (prop.isFinal()) {
            tags << QStringLiteral("FINAL");
        }

        qDebug() << "    特性:" << tags.join(QStringLiteral(", "));
    }
    qDebug() << "";
}

void printOwnProperties(const QObject* obj)
{
    const QMetaObject* metaObj = obj->metaObject();

    qDebug() << "=== 本类属性（不含继承）===";
    for (int i = metaObj->propertyOffset(); i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        qDebug() << "  [" << i << "]" << prop.name()
                 << "=" << prop.read(obj).toString();
    }
    qDebug() << "";
}

void checkPropertyCapabilities(const QObject* obj)
{
    const QMetaObject* metaObj = obj->metaObject();

    qDebug() << "=== 属性能力检测 ===";
    for (int i = metaObj->propertyOffset(); i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        qDebug() << "属性:" << prop.name();
        qDebug() << "  isReadable:     " << (prop.isReadable() ? "是" : "否");
        qDebug() << "  isWritable:     " << (prop.isWritable() ? "是" : "否");
        qDebug() << "  isResettable:   " << (prop.isResettable() ? "是" : "否");
        qDebug() << "  hasNotifySignal:" << (prop.hasNotifySignal() ? "是" : "否");
        qDebug() << "  isConstant:     " << (prop.isConstant() ? "是" : "否");
        qDebug() << "  isFinal:        " << (prop.isFinal() ? "是" : "否");

        // 如果有通知信号，打印信号名称
        if (prop.hasNotifySignal()) {
            qDebug() << "  notifySignal:   " << prop.notifySignal().name();
        }
    }
    qDebug() << "";
}

void printClassInfo(const QObject* obj)
{
    const QMetaObject* metaObj = obj->metaObject();

    qDebug() << "=== Q_CLASSINFO 元数据 ===";
    qDebug() << "classInfoCount:" << metaObj->classInfoCount();
    for (int i = 0; i < metaObj->classInfoCount(); ++i) {
        QMetaClassInfo info = metaObj->classInfo(i);
        qDebug() << "  " << info.name() << "=" << info.value();
    }
    qDebug() << "";
}
