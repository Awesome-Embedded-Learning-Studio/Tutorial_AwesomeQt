/// @file    main.cpp
/// @brief   QCheckBox 进阶演示程序入口。
///
/// 启动 TriStateTreeWidget 窗口，展示"角色 → 模块 → 权限"三层权限树
/// 的三态复选框联动传播。点击任意层级的节点，checkState 将自动向下传播到
/// 所有子孙节点，同时向上冒泡更新祖先节点的状态。
///
/// 对应教程：进阶层 03-QtWidgets/20-QCheckBox 进阶。

#include "tri_state_tree_widget.h"

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    // 顶部说明标签
    auto* hint = new QLabel(
        QStringLiteral("Click any checkbox to test tri-state propagation.\n"
                       "Parent -> children (top-down) and children -> parent (bottom-up).\n"
                       "PartiallyChecked appears when children have mixed states."));
    hint->setWordWrap(true);

    auto* tree = new TriStateTreeWidget;

    layout->addWidget(hint);
    layout->addWidget(tree, 1);

    container->setWindowTitle(QStringLiteral("Tri-state CheckState Demo - QCheckBox Advanced"));
    container->resize(500, 600);
    container->show();

    return app.exec();
}
