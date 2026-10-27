/**
 * @file editable_table_window.cpp
 * @brief EditableTable 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "editable_table_window.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "editable_table.h"

using AwesomeQt::EditableTable;

EditableTableWindow::EditableTableWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
}

void EditableTableWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* root = new QVBoxLayout(central);

    // —— 表格组：五类型列，预填几行 ——
    auto* table_group =
        new QGroupBox("Editable Table (text / int 0-100 / double 0-1 / combo / check)");
    auto* table_layout = new QVBoxLayout(table_group);

    table_ = new EditableTable(table_group);
    table_->setAlternatingRowColors(true);
    table_->addColumn("Name", EditableTable::ColumnType::kText);
    table_->addColumn("Score", EditableTable::ColumnType::kInt, 0, 100);
    table_->addColumn("Ratio", EditableTable::ColumnType::kDouble, 0.0, 1.0);
    table_->addColumn("Color", EditableTable::ColumnType::kCombo, 0, 0, {"Red", "Green", "Blue"});
    table_->addColumn("Active", EditableTable::ColumnType::kCheck);

    // 预填几行（演示 setData 整表回填 + 越界/类型不符夹值）
    table_->setData({
        {"Alice", 88, 0.42, "Green", true},
        {"Bob", 120, 1.5, "Yellow", false}, // 120→100, 1.5→1.0, Yellow→Red(回库)
        {"Carol", 73, 0.66, "Blue", true},
    });

    table_layout->addWidget(table_);
    table_->resizeColumnsToContents();
    root->addWidget(table_group);

    // —— 操作组：增删清 + 打印 + 只读切换 ——
    auto* action_group = new QGroupBox("Operations");
    auto* action_layout = new QHBoxLayout(action_group);

    auto* add_btn = new QPushButton("Add Row", action_group);
    auto* del_btn = new QPushButton("Remove Selected", action_group);
    auto* clear_btn = new QPushButton("Clear", action_group);
    auto* dump_btn = new QPushButton("Print Data", action_group);
    editable_check_ = new QCheckBox("Editable", action_group);
    editable_check_->setChecked(table_->isEditable());

    action_layout->addWidget(add_btn);
    action_layout->addWidget(del_btn);
    action_layout->addWidget(clear_btn);
    action_layout->addWidget(dump_btn);
    action_layout->addStretch();
    action_layout->addWidget(editable_check_);
    root->addWidget(action_group);

    // —— 输出区：打印 data() ——
    auto* output_group = new QGroupBox("Data Output");
    auto* output_layout = new QVBoxLayout(output_group);
    auto* hint =
        new QLabel("Tip: double-click an int cell, type 9999 or 'abc' → delegate clamps to 100.");
    hint->setWordWrap(true);
    text_output_ = new QTextEdit(output_group);
    text_output_->setReadOnly(true);
    text_output_->setMinimumHeight(140);
    output_layout->addWidget(hint);
    output_layout->addWidget(text_output_);
    root->addWidget(output_group);

    // —— 连线（函数指针语法）——
    connect(add_btn, &QPushButton::clicked, this, [this]() {
        table_->addRow();
        dump_data();
    });
    connect(del_btn, &QPushButton::clicked, this, [this]() {
        // 删当前选中行；无选中则删最后一行（removeRow 自己处理 -1）
        table_->removeRow(table_->currentRow());
        dump_data();
    });
    connect(clear_btn, &QPushButton::clicked, this, [this]() {
        table_->clear();
        dump_data();
    });
    connect(dump_btn, &QPushButton::clicked, this, &EditableTableWindow::dump_data);
    connect(editable_check_, &QCheckBox::toggled, this,
            [this](bool checked) { table_->setEditable(checked); });

    // 编辑即回显，验证委托校验后的值
    connect(table_, &EditableTable::dataEdited, this,
            [this](int row, int col, const QVariant& value) {
                text_output_->append(
                    QString("edited: [%1,%2] = %3").arg(row).arg(col).arg(value.toString()));
            });

    resize(720, 560);
    dump_data();
}

void EditableTableWindow::dump_data() {
    const auto rows = table_->data();
    QString text = QString("rows = %1\n").arg(rows.size());
    for (int r = 0; r < rows.size(); ++r) {
        QStringList cells;
        for (const auto& cell : rows.at(r)) {
            cells << cell.toString();
        }
        text += QString("[%1] ").arg(r) + cells.join(" | ") + "\n";
    }
    text_output_->setPlainText(text);
}
