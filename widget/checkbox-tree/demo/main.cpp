/**
 * @file main.cpp
 * @brief CheckboxTree 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */

#include "checkbox_tree_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    CheckboxTreeWindow window;
    window.show();
    return app.exec();
}
