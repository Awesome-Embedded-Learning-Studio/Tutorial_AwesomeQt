/*
 *  Qt 6 入门教程 - 示例 6.2
 *  主题：属性绑定与响应式数据流
 *
 * 本示例演示：
 * 1. 属性绑定 width:parent.width*0.5 的自动追踪
 * 2. property 关键字声明自定义属性
 * 3. onPropertyChanged 信号处理器
 * 4. 绑定断裂陷阱的演示与修复
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/PropertyBindingDemo/Main.qml"_s);

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
