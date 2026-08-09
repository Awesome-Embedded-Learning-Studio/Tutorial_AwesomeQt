/**
 * @file custom-model_window.cpp
 * @brief CustomTableModel 演示实现——QTableView + 增删行 + 优先级 SpinBox 代理 + 完成态勾选
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "custom-model_window.h"

#include <algorithm> // std::sort / std::greater

#include <QHeaderView>
#include <QList>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include "custom-model.h"

namespace {

/// @brief 优先级列代理：双击编辑时弹出 QSpinBox(0..3)，避免乱输
class PrioritySpinBox : public QSpinBox {
  public:
    explicit PrioritySpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {
        setRange(0, 3);
        setAlignment(Qt::AlignCenter);
    }
};

} // namespace

CustomModelWindow::CustomModelWindow(QWidget* parent) : QMainWindow(parent) {
    // 创建模型 + 视图，把模型交给视图——视图全靠模型提供的 rowCount/data/headerData 渲染
    model_ = new AwesomeQt::CustomTableModel(this);
    table_view_ = new QTableView(this);
    table_view_->setModel(model_);

    // 列宽：标题/负责人拉宽，优先级/完成态收窄
    table_view_->horizontalHeader()->setSectionResizeMode(AwesomeQt::CustomTableModel::kTitle,
                                                          QHeaderView::Stretch);
    table_view_->horizontalHeader()->setSectionResizeMode(AwesomeQt::CustomTableModel::kAssignee,
                                                          QHeaderView::Stretch);
    table_view_->setColumnWidth(AwesomeQt::CustomTableModel::kPriority, 80);
    table_view_->setColumnWidth(AwesomeQt::CustomTableModel::kDone, 60);

    // 允许整行选中（删除时取选中行的集合更直观）
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setEditTriggers(QAbstractItemView::DoubleClicked |
                                 QAbstractItemView::EditKeyPressed);

    setup_toolbar();

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->addWidget(table_view_);
    setCentralWidget(central);

    setWindowTitle(QStringLiteral("CustomTableModel 演示——自管数据 + 增删行 + 多角色"));
    resize(640, 420);
}

void CustomModelWindow::setup_toolbar() {
    auto* toolbar = addToolBar(QStringLiteral("任务"));

    add_btn_ = new QPushButton(QStringLiteral("＋ 增行"), this);
    remove_btn_ = new QPushButton(QStringLiteral("－ 删选中行"), this);
    seed_btn_ = new QPushButton(QStringLiteral("↻ 加几条示例"), this);

    toolbar->addWidget(add_btn_);
    toolbar->addWidget(remove_btn_);
    toolbar->addWidget(seed_btn_);

    // 函数指针语法 connect，禁用 SIGNAL/SLOT 宏
    connect(add_btn_, &QPushButton::clicked, this, &CustomModelWindow::add_row);
    connect(remove_btn_, &QPushButton::clicked, this, &CustomModelWindow::remove_selected_rows);
    connect(seed_btn_, &QPushButton::clicked, this, &CustomModelWindow::seed_extra);

    // 给优先级列装个 SpinBox 代理——双击优先级单元格就弹 0..3 选数器
    // (这里用 setIndexWidget 是「即时型」代理，够演示编辑回写；正经可复用代理用
    // QStyledItemDelegate) 注：setIndexWidget 只对当前可见行建
    // widget，滚动新行出来时视图会问模型要—— 为教学简洁此处只给固定几行建 SpinBox，演示「setData
    // 回写」链路即可。
    for (int r = 0; r < model_->rowCount(); ++r) {
        auto* spin = new PrioritySpinBox(table_view_);
        spin->setValue(model_->taskAt(r).priority);
        table_view_->setIndexWidget(model_->index(r, AwesomeQt::CustomTableModel::kPriority), spin);
        // SpinBox 值变 → setData 回写模型（走 EditRole），dataChanged 自动刷新该行
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, spin](int v) {
            // 反查 SpinBox 所在行（避免闭包捕获过期的 r）
            for (int row = 0; row < model_->rowCount(); ++row) {
                if (table_view_->indexWidget(
                        model_->index(row, AwesomeQt::CustomTableModel::kPriority)) == spin) {
                    model_->setData(model_->index(row, AwesomeQt::CustomTableModel::kPriority), v,
                                    Qt::EditRole);
                    break;
                }
            }
        });
    }
}

void CustomModelWindow::add_row() {
    // 末尾插入一行空任务（insertRows 配 beginInsertRows/endInsertRows）
    model_->insertRow(model_->rowCount());
    // 滚到末尾让新行可见
    table_view_->scrollToBottom();
}

void CustomModelWindow::remove_selected_rows() {
    // 取选中行：从大到小删，避免删一行后后面的行号失效
    const auto selected = table_view_->selectionModel()->selectedRows();
    QList<int> rows;
    rows.reserve(selected.size());
    for (const QModelIndex& idx : selected) {
        rows.append(idx.row());
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows) {
        model_->removeRow(row);
    }
}

void CustomModelWindow::seed_extra() {
    // 演示便捷 API appendTask：一条事务插一行（等价 insertRows + setData 四列）
    model_->appendTask({QStringLiteral("代码评审"), QStringLiteral("Eve"), 2, false});
    model_->appendTask({QStringLiteral("更新文档"), QStringLiteral("Frank"), 1, true});
}
