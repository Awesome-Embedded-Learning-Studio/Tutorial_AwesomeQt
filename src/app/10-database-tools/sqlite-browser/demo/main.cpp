/**
 * @file main.cpp
 * @brief SQLite Browser 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "sqlite_browser_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AwesomeQt SQLite Browser");
    SqliteBrowserWindow window;
    window.show();
    return app.exec();
}
