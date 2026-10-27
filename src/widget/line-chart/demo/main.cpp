/**
 * @file main.cpp
 * @brief LineChart 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "line_chart_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    LineChartWindow window;
    window.show();
    return app.exec();
}
