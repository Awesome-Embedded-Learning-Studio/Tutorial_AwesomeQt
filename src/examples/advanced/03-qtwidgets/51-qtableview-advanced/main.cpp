/// @file    main.cpp
/// @brief   程序入口，构建并展示 BigTableWindow。
///
/// 对应教程：进阶层 03-QtWidgets/51-QTableView 百万行数据虚拟滚动。

#include "big_table_model.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    BigTableWindow window;
    window.show();

    return app.exec();
}
