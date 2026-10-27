/// @file    main.cpp
/// @brief   程序入口，构建并展示 ErrorMessageDemo 窗口。
///
/// 对应教程：进阶层 03-QtWidgets/68-QErrorMessage 进阶。

#include "error_message_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ErrorMessageDemo demo;
    demo.setWindowTitle(QObject::tr("QErrorMessage - Advanced"));
    demo.resize(520, 200);
    demo.show();

    return app.exec();
}
