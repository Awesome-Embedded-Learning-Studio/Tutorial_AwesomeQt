/**
 * @file main.cpp
 * @brief SpeedMeter 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "speed_meter_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    SpeedMeterWindow window;
    window.show();
    return app.exec();
}
