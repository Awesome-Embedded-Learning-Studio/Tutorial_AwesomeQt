/**
 * @file main.cpp
 * @brief HMI Dashboard 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "hmi_dashboard_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HmiDashboardWindow window;
    window.show();
    app.exec();
}
