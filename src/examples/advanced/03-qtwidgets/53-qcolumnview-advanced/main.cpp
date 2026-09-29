/// @file    main.cpp
/// @brief   程序入口，构建并展示 ColumnViewWindow。
///
/// 对应教程：进阶层 03-QtWidgets/53-QColumnView 高级用法。

#include "column_model.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ColumnViewWindow window;
    window.show();

    return app.exec();
}
