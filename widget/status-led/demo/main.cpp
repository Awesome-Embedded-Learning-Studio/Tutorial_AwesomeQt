/**
 * @file main.cpp
 * @brief StatusLED 演示程序入口
 * @copyright Copyright (c) 2026
 */

#include "status_led_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    StatusLEDWindow window;
    window.show();
    app.exec();
}