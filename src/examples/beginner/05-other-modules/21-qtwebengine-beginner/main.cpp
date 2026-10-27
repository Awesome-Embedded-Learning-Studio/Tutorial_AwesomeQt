#include <QApplication>
#include <QDebug>

#include "browserwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "QtWebEngine 内嵌浏览器示例";
    qDebug() << "本示例演示 QWebEngineView + runJavaScript + Profile 管理";

    BrowserWindow window;
    window.show();

    // 默认加载 Qt 官网
    window.loadUrl();

    return app.exec();
}
