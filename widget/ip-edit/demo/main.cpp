/**
 * @file main.cpp
 * @brief IpEdit 演示程序入口
 * @copyright Copyright (c) 2026
 */

#include "ip_edit_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    IpEditWindow window;
    window.show();
    return app.exec();
}
