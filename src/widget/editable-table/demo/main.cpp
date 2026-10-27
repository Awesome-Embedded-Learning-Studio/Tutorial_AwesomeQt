/**
 * @file main.cpp
 * @brief EditableTable 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "editable_table_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    EditableTableWindow window;
    window.show();
    return app.exec();
}
