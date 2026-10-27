/**
 * @file main.cpp
 * @brief CPU / Memory Monitor 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "system_monitor_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AwesomeQt CPU / Memory Monitor");
    SystemMonitorWindow window;
    window.show();
    return app.exec();
}
