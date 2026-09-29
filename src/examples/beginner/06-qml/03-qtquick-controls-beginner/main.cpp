/*
 *  Qt 6 入门教程 - 示例 6.3
 *  主题：Qt Quick Controls 组件基础
 *
 * 本示例演示：
 * 1. ApplicationWindow 主窗口结构
 * 2. Button / TextField / ComboBox / CheckBox / Slider 常用控件
 * 3. ColumnLayout / RowLayout / GridLayout QML 布局
 * 4. Dialog / Popup / Menu 弹出组件
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/QuickControlsDemo/Main.qml"_s);

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
