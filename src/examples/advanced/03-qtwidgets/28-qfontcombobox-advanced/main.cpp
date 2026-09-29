/// @file    main.cpp
/// @brief   QFontComboBox 进阶演示程序入口。
///
/// 启动 FilteredFontCombo 窗口，展示 Writing System 过滤、
/// FontPreviewDelegate 字体预览与实时选中字体预览标签。
///
/// 对应教程：进阶层 03-QtWidgets/28-QFontComboBox 进阶。

#include "filtered_font_combo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    FilteredFontCombo widget;
    widget.show();

    return app.exec();
}
