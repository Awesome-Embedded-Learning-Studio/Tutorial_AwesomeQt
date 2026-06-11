/// @file    main.cpp
/// @brief   程序入口，构建并展示 QMessageBox 自定义图标与详情区域演示窗口。
///
/// 对应教程：进阶层 03-QtWidgets/62-QMessageBox 自定义图标与详情区域。

#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
