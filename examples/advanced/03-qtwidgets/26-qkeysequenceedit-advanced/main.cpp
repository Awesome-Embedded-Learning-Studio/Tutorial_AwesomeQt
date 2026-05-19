/// @file    main.cpp
/// @brief   QKeySequenceEdit 进阶演示程序入口。
///
/// 启动 ShortcutConflictPanel 窗口，展示多快捷键配置、冲突检测、
/// editingFinished 焦点丢失恢复以及系统保留快捷键检查。
///
/// 对应教程：进阶层 03-QtWidgets/26-QKeySequenceEdit 进阶。

#include "shortcut_conflict_panel.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ShortcutConflictPanel panel;
    panel.show();

    return app.exec();
}
