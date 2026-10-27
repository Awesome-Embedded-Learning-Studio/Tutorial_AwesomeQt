/// @file    main.cpp
/// @brief   QPrintPreviewDialog 自定义工具栏演示程序入口。
///
/// 启动 PreviewMainWindow 窗口，展示如何在打印预览对话框中
/// 添加自定义工具栏按钮（水印开关），以及条件绘制水印文本。
///
/// 对应教程：进阶层 03-QtWidgets/74-打印预览对话框进阶。

#include "widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PreviewMainWindow window;
    window.show();

    return app.exec();
}
