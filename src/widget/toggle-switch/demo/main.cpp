/**
 * @file main.cpp
 * @brief ToggleSwitch 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "toggle_switch_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ToggleSwitchWindow window;
    window.show();
    return app.exec();
}
