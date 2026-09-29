/**
 * @file main.cpp
 * @brief Tetris 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "tetris_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AwesomeQt Tetris");
    app.setOrganizationName("AwesomeQt");
    TetrisWindow window;
    window.show();
    return app.exec();
}
