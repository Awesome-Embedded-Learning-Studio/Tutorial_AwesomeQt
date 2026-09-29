/// @file    main.cpp
/// @brief   QTextEdit 进阶演示程序入口。
///
/// 启动 RichTextNavigator 窗口，展示 QTextDocument 统计、QTextCursor 导航、
/// insertHtml 富文本插入以及 find 搜索高亮。
///
/// 对应教程：进阶层 03-QtWidgets/23-QTextEdit 进阶。

#include "rich_text_navigator.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 使用顶层 QWidget 来组合工具栏、编辑器和状态栏
    auto* window = new QWidget;
    auto* mainLayout = new QVBoxLayout(window);

    // 创建编辑器实例
    auto* editor = new RichTextNavigator;

    // 工具栏（由 RichTextNavigator 创建）
    auto* toolBar = editor->createToolBar();

    // 状态栏（由 RichTextNavigator 创建）
    auto* statusBar = editor->createStatusBar();

    // 组装布局
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(editor, 1);
    mainLayout->addWidget(statusBar);

    // 初始更新状态栏
    editor->updateStatusBar();

    window->setWindowTitle(QStringLiteral("QTextEdit Advanced - RichText Navigator"));
    window->resize(800, 600);
    window->show();

    return app.exec();
}
