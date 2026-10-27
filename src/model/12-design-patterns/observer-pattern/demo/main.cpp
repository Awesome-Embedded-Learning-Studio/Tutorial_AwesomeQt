/**
 * @file main.cpp
 * @brief Observer Pattern 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "observer-pattern_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ObserverPatternWindow window;
    window.show();
    return app.exec();
}
