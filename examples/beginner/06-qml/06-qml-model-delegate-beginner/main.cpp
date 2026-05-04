/*
 *  Qt 6 入门教程 - 示例 6.6
 *  主题：QML Model/Delegate 数据驱动视图
 *
 * 本示例演示：
 * 1. C++ QAbstractListModel 暴露给 QML ListView / GridView
 * 2. 自定义 delegate 渲染列表/网格项
 * 3. 通过 Q_INVOKABLE 方法增删数据
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

using namespace Qt::StringLiterals;

#include "fruit_model.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建 C++ 模型，注入 QML 上下文
    FruitModel fruitModel;
    engine.rootContext()->setContextProperty("fruitModel", &fruitModel);

    const QUrl url(u"qrc:/qt/qml/QmlModelDelegateDemo/Main.qml"_s);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
