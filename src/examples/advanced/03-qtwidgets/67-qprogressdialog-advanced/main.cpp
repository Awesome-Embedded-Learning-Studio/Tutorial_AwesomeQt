/// @file    main.cpp
/// @brief   程序入口，构建并展示 ProgressDialogDemo 窗口。
///
/// 对应教程：进阶层 03-QtWidgets/67-QProgressDialog 进阶。

#include "progress_dialog_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ProgressDialogDemo demo;
    demo.setWindowTitle(QObject::tr("QProgressDialog - Advanced"));
    demo.resize(400, 200);
    demo.show();

    return app.exec();
}
