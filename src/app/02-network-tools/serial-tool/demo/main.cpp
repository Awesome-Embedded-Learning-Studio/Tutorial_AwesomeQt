/**
 * @file main.cpp
 * @brief Serial Tool 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "serial_tool_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AwesomeQt Serial Tool");
    SerialToolWindow window;
    window.show();
    return app.exec();
}
