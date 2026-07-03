/**
 * @file main.cpp
 * @brief Network Tool 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "network_tool_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AwesomeQt Network Tool");
    NetworkToolWindow window;
    window.show();
    return app.exec();
}
