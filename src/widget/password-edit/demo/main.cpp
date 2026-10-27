/**
 * @file main.cpp
 * @brief PasswordEdit 演示程序入口
 * @copyright Copyright (c) 2026
 */

#include "password_edit_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    PasswordEditWindow window;
    window.show();
    return app.exec();
}
