/// @file    main.cpp
/// @brief   Model/View 进阶演示程序入口。
///
/// 展示自定义 QAbstractTableModel + QStyledItemDelegate 的完整协作流程：
/// TaskTableModel 提供可编辑数据，ProgressDelegate 渲染进度条，
/// QTableView 作为 View 展示。
///
/// 对应教程：进阶层 03-QtWidgets/03-Model/View 进阶。

#include "progress_delegate.h"
#include "task_table_model.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    auto* mainLayout = new QVBoxLayout(window);

    // 标题说明
    auto* title = new QLabel(QStringLiteral(
        "Model/View 进阶演示\n"
        "1. 双击任务名称列可编辑\n"
        "2. 进度列使用自定义 Delegate 渲染进度条\n"
        "3. 在搜索框输入关键字可过滤任务"));
    title->setWordWrap(true);
    mainLayout->addWidget(title);

    // 搜索过滤行
    auto* searchRow = new QHBoxLayout;
    auto* searchLabel = new QLabel(QStringLiteral("搜索:"));
    auto* searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText(QStringLiteral("输入关键字过滤任务..."));
    searchRow->addWidget(searchLabel);
    searchRow->addWidget(searchEdit, 1);
    mainLayout->addLayout(searchRow);

    // 核心：自定义 Model
    auto* model = new TaskTableModel(window);

    // 代理层：QSortFilterProxyModel 提供排序和过滤
    auto* proxyModel = new QSortFilterProxyModel(window);
    proxyModel->setSourceModel(model);
    // 启用点击表头排序
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // 在名称列中进行过滤
    proxyModel->setFilterKeyColumn(0);

    // View：QTableView
    auto* tableView = new QTableView;
    tableView->setModel(proxyModel);
    tableView->setSortingEnabled(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setStretchLastSection(true);

    // 核心：为进度列安装自定义 Delegate
    auto* progressDelegate = new ProgressDelegate(tableView);
    tableView->setItemDelegateForColumn(2, progressDelegate);

    mainLayout->addWidget(tableView);

    // 操作按钮行
    auto* buttonRow = new QHBoxLayout;
    auto* addBtn = new QPushButton(QStringLiteral("添加任务"));
    auto* removeBtn = new QPushButton(QStringLiteral("删除选中行"));
    buttonRow->addWidget(addBtn);
    buttonRow->addWidget(removeBtn);
    buttonRow->addStretch();
    mainLayout->addLayout(buttonRow);

    // 状态标签
    auto* statusLabel =
        new QLabel(QStringLiteral("任务总数：%1").arg(model->rowCount()));
    mainLayout->addWidget(statusLabel);

    // ── 信号槽连接 ──

    // 搜索框实时过滤——更新过滤条件后必须调用 invalidateFilter()
    QObject::connect(searchEdit, &QLineEdit::textChanged, [proxyModel](const QString& text) {
        proxyModel->setFilterFixedString(text);
        // setFilterFixedString 内部会触发 invalidateFilter，无需手动调用
    });

    // 添加任务
    QObject::connect(addBtn, &QPushButton::clicked, [model, statusLabel]() {
        const int count = model->rowCount();
        Task newTask{
            QStringLiteral("新任务 %1").arg(count + 1),
            5,  // 默认中等优先级
            0   // 默认进度 0
        };
        model->addTask(newTask);
        statusLabel->setText(QStringLiteral("任务总数：%1").arg(model->rowCount()));
    });

    // 删除选中行
    QObject::connect(removeBtn, &QPushButton::clicked, [tableView, model, proxyModel, statusLabel]() {
        const QModelIndexList selected = tableView->selectionModel()->selectedRows();
        if (selected.isEmpty()) {
            return;
        }

        // 收集源模型行号并按降序排列，避免删除时索引偏移
        QVector<int> rows;
        rows.reserve(selected.size());
        for (const auto& idx : selected) {
            rows.append(proxyModel->mapToSource(idx).row());
        }
        std::sort(rows.begin(), rows.end(), std::greater<int>());

        for (int row : rows) {
            model->removeTask(row);
        }

        statusLabel->setText(QStringLiteral("任务总数：%1").arg(model->rowCount()));
    });

    // Model 数据变更时更新状态
    QObject::connect(model, &QAbstractItemModel::rowsInserted, [model, statusLabel]() {
        statusLabel->setText(QStringLiteral("任务总数：%1").arg(model->rowCount()));
    });
    QObject::connect(model, &QAbstractItemModel::rowsRemoved, [model, statusLabel]() {
        statusLabel->setText(QStringLiteral("任务总数：%1").arg(model->rowCount()));
    });

    window->setWindowTitle(QStringLiteral("Model/View Advanced Demo"));
    window->resize(650, 450);
    window->show();

    return app.exec();
}
