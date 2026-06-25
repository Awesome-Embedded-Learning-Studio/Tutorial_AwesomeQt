/**
 * @file main.cpp
 * @brief CircleProgress 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "circle_progress_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    CircleProgressWindow window;
    window.show();
    return app.exec();
}
