/**
 * @file checkbox_tree_window.cpp
 * @brief CheckboxTree 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */

#include "checkbox_tree_window.h"

#include "checkbox_tree.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

CheckboxTreeWindow::CheckboxTreeWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
    populate_sample_tree();
}

void CheckboxTreeWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* root = new QHBoxLayout(central);

    // —— 左：勾选树本体 ——
    auto* tree_group = new QGroupBox("Checkbox Tree (try clicking the boxes)");
    auto* tree_layout = new QVBoxLayout(tree_group);
    tree_ = new AwesomeQt::CheckboxTree(tree_group);
    tree_layout->addWidget(tree_);
    root->addWidget(tree_group, 2);

    // —— 右：控制面板 + 输出 ——
    auto* side_group = new QGroupBox("Controls");
    auto* side_layout = new QVBoxLayout(side_group);

    auto* list_btn = new QPushButton("List checked items", side_group);
    auto* check_all_btn = new QPushButton("Check all", side_group);
    auto* uncheck_all_btn = new QPushButton("Uncheck all", side_group);

    propagation_toggle_ = new QCheckBox("Propagation enabled (parent<->child linkage)", side_group);
    propagation_toggle_->setChecked(true);

    output_ = new QTextEdit(side_group);
    output_->setReadOnly(true);
    output_->setPlaceholderText("Checked item paths will appear here...");

    side_layout->addWidget(list_btn);
    side_layout->addWidget(check_all_btn);
    side_layout->addWidget(uncheck_all_btn);
    side_layout->addWidget(propagation_toggle_);
    side_layout->addWidget(output_, 1);

    root->addWidget(side_group, 1);

    // 函数指针语法连接信号槽。
    connect(list_btn, &QPushButton::clicked, this, &CheckboxTreeWindow::list_checked_items);
    connect(check_all_btn, &QPushButton::clicked, tree_, &AwesomeQt::CheckboxTree::checkAll);
    connect(uncheck_all_btn, &QPushButton::clicked, tree_, &AwesomeQt::CheckboxTree::uncheckAll);
    connect(propagation_toggle_, &QCheckBox::toggled, tree_,
            &AwesomeQt::CheckboxTree::setPropagationEnabled);

    setWindowTitle("CheckboxTree Demo");
    resize(720, 480);
}

void CheckboxTreeWindow::populate_sample_tree() {
    // 三层示例树：项目 > 模块 > 文件。用 addItem 让控件自己挂节点 + 走初始化逻辑。
    using AwesomeQt::CheckboxTree;

    auto* project = tree_->addItem(nullptr, "Project AwesomeQt");
    auto* mod_a = tree_->addItem(project, "Module A (Core)");
    auto* mod_b = tree_->addItem(project, "Module B (Widgets)");

    tree_->addItem(mod_a, "core.h");
    tree_->addItem(mod_a, "core.cpp");
    tree_->addItem(mod_a, "core_p.h");

    tree_->addItem(mod_b, "widget.cpp");
    tree_->addItem(mod_b, "widget.h");
    tree_->addItem(mod_b, "resources.qrc");

    project->setExpanded(true);
    mod_a->setExpanded(true);
    mod_b->setExpanded(true);
}

void CheckboxTreeWindow::list_checked_items() {
    const QList<QTreeWidgetItem*> checked = tree_->checkedItems();
    if (checked.isEmpty()) {
        output_->append("- (no checked items)");
        return;
    }

    // 把每项的祖先路径拼出来，方便肉眼核对父子联动效果。
    for (QTreeWidgetItem* item : checked) {
        if (item == nullptr) {
            continue;
        }
        QStringList path;
        for (const QTreeWidgetItem* p = item; p != nullptr; p = p->parent()) {
            path.prepend(p->text(0));
        }
        output_->append("- " + path.join(" > "));
    }
}
