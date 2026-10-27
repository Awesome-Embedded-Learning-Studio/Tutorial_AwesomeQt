/*
 *  Qt 6 入门教程 - 示例 6.4
 *  主题：C++ 与 QML 互操作基础
 *
 * 本示例演示：
 * 1. Q_PROPERTY 暴露 C++ 属性到 QML
 * 2. QQmlContext::setContextProperty() 注入对象
 * 3. QML 调用 Q_INVOKABLE 方法
 * 4. C++ 发射信号到 QML 的信号处理器
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

using namespace Qt::StringLiterals;

#include "app_controller.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建控制器实例，注入到 QML 上下文
    AppController controller;
    engine.rootContext()->setContextProperty("appController", &controller);

    const QUrl url(u"qrc:/qt/qml/CppQmlInteropDemo/Main.qml"_s);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    // 定时从 C++ 端发射信号到 QML
    QTimer notificationTimer;
    notificationTimer.setInterval(5000);
    QObject::connect(&notificationTimer, &QTimer::timeout, &controller, [&controller]() {
        if (!controller.userName().isEmpty()) {
            emit controller.notificationRequested(
                "Hey " + controller.userName() + ", count is " + QString::number(controller.counter()));
        }
    });
    notificationTimer.start();

    return app.exec();
}
