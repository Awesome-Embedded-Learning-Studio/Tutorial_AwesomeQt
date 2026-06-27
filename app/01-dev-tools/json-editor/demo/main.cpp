/**
 * @file main.cpp
 * @brief JSON Editor 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "json_editor_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AwesomeQt JSON Editor");
    JsonEditorWindow window;
    window.show();
    return app.exec();
}
