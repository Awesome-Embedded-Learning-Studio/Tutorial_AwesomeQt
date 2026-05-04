/*
 *  Qt 6 入门教程 - 示例 6.1
 *  主题：QML 语法基础与类型系统
 *
 * 本示例演示：
 * 1. QML 对象声明与层级关系
 * 2. 基础类型：int / real / string / bool / color / url
 * 3. id 机制引用对象
 * 4. JavaScript 表达式在属性值中的使用
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/QmlSyntaxDemo/Main.qml"_s);

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
