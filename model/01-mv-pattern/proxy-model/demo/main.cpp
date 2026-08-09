/**
 * @file main.cpp
 * @brief Proxy-Model 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "proxy-model_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ProxyModelWindow window;
    window.show();
    app.exec();
}
