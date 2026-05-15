/// @file    main.cpp
/// @brief   QObject 属性系统高级演示程序入口。
///
/// 演示 Q_PROPERTY 的 CONSTANT / RESET / FINAL 修饰符、
/// setProperty() / property() 动态属性机制、
/// QMetaObject / QMetaProperty 反射式属性枚举与能力检测。
///
/// 对应教程：进阶层 01-QtBase/01-QObject 属性系统深度拆解。

#include "config_object.h"
#include "reflective_demo.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMetaProperty>
#include <QVariant>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 演示 1: ConfigObject 基本属性操作——CONSTANT / RESET / FINAL
    qDebug() << "\n[演示 1] ConfigObject 属性基本操作";
    qDebug() << "----------------------------------------------";

    ConfigObject config;

    // CONSTANT 属性：只能在构造时设置，没有 WRITE 函数
    qDebug() << "appVersion (CONSTANT):" << config.appVersion();

    // 尝试用 setProperty 写入 CONSTANT 属性——setProperty 会返回 false
    bool ok = config.setProperty("appVersion", QStringLiteral("2.0.0"));
    qDebug() << "setProperty(\"appVersion\", \"2.0.0\") 成功?" << ok;
    qDebug() << "appVersion 仍然是:" << config.appVersion();

    // 带 RESET 的属性：resetDebugMode() 恢复默认值 false
    config.setDebugMode(true);
    qDebug() << "设置 debugMode = true, 当前值:" << config.debugMode();

    // 通过 QMetaProperty::reset() 调用 RESET 函数
    const QMetaObject* metaObj = config.metaObject();
    int idx = metaObj->indexOfProperty("debugMode");
    if (idx >= 0) {
        QMetaProperty prop = metaObj->property(idx);
        prop.reset(&config);
        qDebug() << "调用 QMetaProperty::reset() 后, debugMode:"
                 << config.debugMode();
    }

    // FINAL 属性：正常读写，但子类不能用 Q_PROPERTY 覆盖
    config.setLogLevel(ConfigObject::kDebug);
    qDebug() << "logLevel (FINAL):" << config.logLevel()
             << ", 枚举名:"
             << config.metaObject()
                    ->enumerator(
                        config.metaObject()->indexOfEnumerator("LogLevel"))
                    .valueToKey(config.logLevel());

    qDebug() << "";

    // 演示 2: setProperty() / property() 动态属性
    qDebug() << "\n[演示 2] 动态属性 setProperty() / property()";
    qDebug() << "----------------------------------------------";

    ConfigObject obj;

    // setProperty 对已声明的 Q_PROPERTY：调用 WRITE 函数
    obj.setProperty("debugMode", true);
    qDebug() << "setProperty(\"debugMode\", true) 后:" << obj.debugMode();

    // setProperty 对未声明的名称：创建动态属性
    obj.setProperty("customTag", QStringLiteral("important"));
    obj.setProperty("internalFlags", 42);
    obj.setProperty("score", 95.5);

    qDebug() << "动态属性 customTag:"
             << obj.property("customTag").toString();
    qDebug() << "动态属性 internalFlags:"
             << obj.property("internalFlags").toInt();
    qDebug() << "动态属性 score:"
             << obj.property("score").toDouble();

    // dynamicPropertyNames() 返回所有动态属性名称
    qDebug() << "动态属性列表:" << obj.dynamicPropertyNames();

    // 访问不存在的属性返回无效 QVariant
    QVariant v = obj.property("nonExistent");
    qDebug() << "不存在的属性 isValid?" << v.isValid();

    qDebug() << "";

    // 演示 3: QMetaObject::propertyCount() / propertyOffset() 属性枚举
    qDebug() << "\n[演示 3] 属性枚举 propertyCount / propertyOffset";
    qDebug() << "----------------------------------------------";

    printAllProperties(&config);
    printOwnProperties(&config);

    // 演示 4: QMetaProperty 能力检测
    qDebug() << "\n[演示 4] QMetaProperty 能力检测";
    qDebug() << "----------------------------------------------";

    checkPropertyCapabilities(&config);

    // 演示 5: Q_CLASSINFO 元数据读取
    qDebug() << "\n[演示 5] Q_CLASSINFO 元数据";
    qDebug() << "----------------------------------------------";

    printClassInfo(&config);

    // 通过名称查找 classInfo
    const QMetaObject* mo = config.metaObject();
    int authorIdx = mo->indexOfClassInfo("Author");
    if (authorIdx >= 0) {
        qDebug() << "通过名称查找 classInfo(\"Author\"):"
                 << mo->classInfo(authorIdx).value();
    }

    qDebug() << "";

    // 总结
    qDebug() << "========================================";
    qDebug() << "属性系统高级演示结束";
    qDebug() << "要点回顾:";
    qDebug() << "  1. CONSTANT 属性不可写，无 WRITE 和 NOTIFY";
    qDebug() << "  2. RESET 函数用于恢复默认值";
    qDebug() << "  3. FINAL 属性禁止子类覆盖";
    qDebug() << "  4. setProperty() 对未声明属性创建动态属性";
    qDebug() << "  5. propertyOffset() 区分本类属性和继承属性";
    qDebug() << "  6. QMetaProperty 提供能力查询";
    qDebug() << "  7. Q_CLASSINFO 提供运行时元数据";
    qDebug() << "========================================";

    return 0;
}
