/**
 * @file main.cpp
 * @brief Image Viewer 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "image_viewer_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ImageViewerWindow window;
    window.show();
    app.exec();
}
