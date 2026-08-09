/**
 * @file proxy-model_window.cpp
 * @brief Proxy-Model 演示实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "proxy-model_window.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

namespace {
/// @brief 演示用源数据：姓名 / 年龄 / 城市。故意混入会让字典序出错的数字串。
struct PersonRow {
    const char* name;
    int age;
    const char* city;
};
const QList<PersonRow> kSampleData = {
    {"Alice", 29, "Beijing"}, {"bob", 22, "Shanghai"}, {"Charlie", 35, "Guangzhou"},
    {"david", 9, "Shenzhen"}, {"Eve", 41, "Hangzhou"}, {"Frank", 100, "Chengdu"},
    {"Grace", 7, "Nanjing"},  {"Henry", 18, "Wuhan"},
};
} // namespace

ProxyModelWindow::ProxyModelWindow(QWidget* parent) : QMainWindow(parent) {
    setup_source_model();
    setup_proxy();
    setup_widgets();

    setWindowTitle("Proxy-Model Demo (Sort + Filter + Custom Compare)");
    resize(640, 420);
}

void ProxyModelWindow::setup_source_model() {
    source_model_ = new QStandardItemModel(this);
    source_model_->setHorizontalHeaderLabels(
        {QStringLiteral("Name"), QStringLiteral("Age"), QStringLiteral("City")});

    for (const auto& row : kSampleData) {
        QList<QStandardItem*> cells;
        cells << new QStandardItem(QString::fromLatin1(row.name))
              << new QStandardItem(QString::number(row.age))
              << new QStandardItem(QString::fromLatin1(row.city));
        // 行内单元格交给 model（model 是它们的 parent）托管，避免泄漏告警
        source_model_->appendRow(cells);
    }
}

void ProxyModelWindow::setup_proxy() {
    proxy_model_ = new AwesomeQt::SortFilterProxyModel(this);
    proxy_model_->setSourceModel(source_model_);
    // Age 列按数值排序：否则 "100" < "9" < "35"（字典序错得离谱）
    proxy_model_->setNumericColumns({1});
    // 默认按 Age 列、升序、开排序，启动即看到数值排序效果
    proxy_model_->setFilterKeyColumn(0);
    proxy_model_->setSortRole(Qt::DisplayRole);
    proxy_model_->sort(1, Qt::AscendingOrder);
}

void ProxyModelWindow::setup_widgets() {
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    // 顶部控制条：搜索 + 排序方向 + 过滤范围
    auto* control_group = new QGroupBox(QStringLiteral("Controls"), central);
    auto* form = new QFormLayout(control_group);

    search_edit_ = new QLineEdit(control_group);
    search_edit_->setPlaceholderText(QStringLiteral("e.g. alice / beijing / 9"));
    connect(search_edit_, &QLineEdit::textChanged, this, &ProxyModelWindow::onSearchChanged);
    form->addRow(QStringLiteral("Filter:"), search_edit_);

    ascending_check_ = new QCheckBox(QStringLiteral("Ascending"), control_group);
    ascending_check_->setChecked(true);
    connect(ascending_check_, &QCheckBox::toggled, this, &ProxyModelWindow::onSortDirectionChanged);
    form->addRow(QStringLiteral("Sort direction:"), ascending_check_);

    scope_combo_ = new QComboBox(control_group);
    scope_combo_->addItem(QStringLiteral("Active column only"),
                          QVariant(static_cast<int>(AwesomeQt::FilterScope::kActiveColumn)));
    scope_combo_->addItem(QStringLiteral("All columns"),
                          QVariant(static_cast<int>(AwesomeQt::FilterScope::kAllColumns)));
    connect(scope_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ProxyModelWindow::onFilterScopeChanged);
    form->addRow(QStringLiteral("Filter scope:"), scope_combo_);

    root->addWidget(control_group);

    // 表格：挂在代理上，不是直接挂源模型——这是「代理在中间」的关键
    table_view_ = new QTableView(central);
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    root->addWidget(table_view_);

    auto* hint = new QLabel(
        QStringLiteral("Tip: 点表头切换排序列；勾选框切升降序；搜索框实时过滤；下拉切换全列扫描。"),
        central);
    root->addWidget(hint);

    setCentralWidget(central);
}

void ProxyModelWindow::onSearchChanged(const QString& text) {
    // 用 setFilterFixedString：关键词按字面子串匹配，转义问题交给 Qt
    proxy_model_->setFilterFixedString(text);
}

void ProxyModelWindow::onSortDirectionChanged(bool ascending) {
    const int sort_column = proxy_model_->sortColumn();
    // 列号可能为 -1（尚未排序），兜底回到 Age 列
    const int column = sort_column < 0 ? 1 : sort_column;
    proxy_model_->sort(column, ascending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

void ProxyModelWindow::onFilterScopeChanged(int index) {
    const int scope_value = scope_combo_->itemData(index).toInt();
    proxy_model_->setFilterScope(static_cast<AwesomeQt::FilterScope>(scope_value));
}
