/**
 * @file main.cpp
 * @brief RangeSlider demo 程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include <QApplication>

#include "range_slider_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    RangeSliderWindow w;
    w.show();
    return app.exec();
}
