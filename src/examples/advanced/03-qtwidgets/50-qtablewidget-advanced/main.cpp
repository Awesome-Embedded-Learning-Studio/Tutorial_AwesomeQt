/// @file    main.cpp
/// @brief   程序入口，构建并展示 FrozenTable 控件。
///
/// 对应教程：进阶层 03-QtWidgets/50-QTableWidget 高级用法。

#include "frozen_table.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    FrozenTable table(8, 8);
    table.resize(800, 500);
    table.show();

    return app.exec();
}
