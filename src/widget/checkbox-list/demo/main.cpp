/**
 * @file main.cpp
 * @brief CheckboxList 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */

#include "checkbox_list_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    CheckboxListWindow window;
    window.show();
    return app.exec();
}
