/**
 * @file main.cpp
 * @brief LogViewer 演示程序入口
 * @copyright Copyright (c) 2026
 */

#include "log_viewer_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    LogViewerWindow window;
    window.show();
    return app.exec();
}
