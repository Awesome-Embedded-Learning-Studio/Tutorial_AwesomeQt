/// @file    main.cpp
/// @brief   程序入口，构建并运行 DemoWizard。
///
/// 对应教程：进阶层 03-QtWidgets/69-QWizard 进阶。

#include "wizard_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    DemoWizard wizard;
    wizard.resize(600, 400);
    wizard.show();

    return app.exec();
}
