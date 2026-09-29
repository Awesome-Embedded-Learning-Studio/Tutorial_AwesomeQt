/// @file    main.cpp
/// @brief   程序入口，构建并展示 QDialogButtonBox 自定义按钮演示对话框。
///
/// 对应教程：进阶层 03-QtWidgets/61-QDialogButtonBox 自定义帮助按钮与提示。

#include "settings_dialog.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SettingsDialog dialog;
    dialog.exec();

    return 0;
}
