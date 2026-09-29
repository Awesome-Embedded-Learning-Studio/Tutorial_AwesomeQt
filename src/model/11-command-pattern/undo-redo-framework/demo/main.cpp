/**
 * @file main.cpp
 * @brief Undo/Redo Framework 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "undo_redo_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    UndoRedoWindow window;
    window.show();
    app.exec();
}
