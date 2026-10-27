/**
 * @file main.cpp
 * @brief TreeDragMove 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "tree-drag-move_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    TreeDragMoveWindow window;
    window.show();
    return app.exec();
}
