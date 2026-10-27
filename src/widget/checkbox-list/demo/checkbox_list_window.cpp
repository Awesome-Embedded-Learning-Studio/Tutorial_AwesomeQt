/**
 * @file checkbox_list_window.cpp
 * @brief CheckboxList 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */

#include "checkbox_list_window.h"

#include "checkbox_list.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

CheckboxListWindow::CheckboxListWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
    populate_sample_items();
}

void CheckboxListWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* root = new QHBoxLayout(central);

    // —— 左：勾选列表本体 ——
    auto* list_group = new QGroupBox("Checkbox List (try toggling the boxes)");
    auto* list_layout = new QVBoxLayout(list_group);
    list_ = new AwesomeQt::CheckboxList(list_group);
    list_layout->addWidget(list_);
    root->addWidget(list_group, 2);

    // —— 右：控制面板 + 输出 ——
    auto* side_group = new QGroupBox("Controls");
    auto* side_layout = new QVBoxLayout(side_group);

    auto* list_btn = new QPushButton("List checked items", side_group);
    auto* check_all_btn = new QPushButton("Check all", side_group);
    auto* uncheck_all_btn = new QPushButton("Uncheck all", side_group);
    auto* invert_btn = new QPushButton("Invert selection", side_group);

    alternating_toggle_ = new QCheckBox("Alternating row colors", side_group);
    alternating_toggle_->setChecked(false);

    output_ = new QTextEdit(side_group);
    output_->setReadOnly(true);
    output_->setPlaceholderText("Checked item texts will appear here...");

    side_layout->addWidget(list_btn);
    side_layout->addWidget(check_all_btn);
    side_layout->addWidget(uncheck_all_btn);
    side_layout->addWidget(invert_btn);
    side_layout->addWidget(alternating_toggle_);
    side_layout->addWidget(output_, 1);

    root->addWidget(side_group, 1);

    // 函数指针语法连接信号槽。
    connect(list_btn, &QPushButton::clicked, this, &CheckboxListWindow::list_checked_items);
    connect(check_all_btn, &QPushButton::clicked, list_, &AwesomeQt::CheckboxList::checkAll);
    connect(uncheck_all_btn, &QPushButton::clicked, list_, &AwesomeQt::CheckboxList::uncheckAll);
    connect(invert_btn, &QPushButton::clicked, list_, &AwesomeQt::CheckboxList::invertChecked);
    connect(alternating_toggle_, &QCheckBox::toggled, list_,
            &AwesomeQt::CheckboxList::setAlternatingRowColors);

    setWindowTitle("CheckboxList Demo");
    resize(640, 440);
}

void CheckboxListWindow::populate_sample_items() {
    // 示例：文件权限清单。用 addItems 批量加，前两项预先勾选。
    list_->addItem("Read", true);
    list_->addItem("Write", true);
    list_->addItem("Execute", false);
    list_->addItem("Delete", false);
    list_->addItems({"Modify permissions", "Take ownership", "Change attributes"});
}

void CheckboxListWindow::list_checked_items() {
    // 把 checkedTexts() 输出下方，肉眼核对勾选结果。
    const QStringList checked = list_->checkedTexts();
    if (checked.isEmpty()) {
        output_->append("- (no checked items)");
        return;
    }

    output_->append(QString("Checked (%1):").arg(checked.size()));
    for (const QString& text : checked) {
        output_->append("- " + text);
    }
}
