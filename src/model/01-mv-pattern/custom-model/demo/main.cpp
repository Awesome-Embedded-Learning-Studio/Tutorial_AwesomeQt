/**
 * @file main.cpp
 * @brief CustomTableModel 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "custom-model_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    CustomModelWindow window;
    window.show();
    return app.exec();
}
