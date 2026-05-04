#include "MainWindow.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QTableView 综合演示 — 员工管理");
    resize(800, 520);
    initUi();
    populateEmployeeData();
    applySpan();
    updateStatistics();
}

/// @brief 初始化界面
void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // ================================================================
    // 顶部：搜索框 + 部门筛选
    // ================================================================
    auto *filterLayout = new QHBoxLayout;

    filterLayout->addWidget(new QLabel("搜索姓名:"));
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("输入关键词...");
    filterLayout->addWidget(m_searchEdit, 1);

    filterLayout->addWidget(new QLabel("部门:"));
    m_deptCombo = new QComboBox;
    m_deptCombo->addItem("全部");
    m_deptCombo->addItem("研发部");
    m_deptCombo->addItem("产品部");
    m_deptCombo->addItem("设计部");
    m_deptCombo->addItem("运维部");
    filterLayout->addWidget(m_deptCombo);

    mainLayout->addLayout(filterLayout);

    // ================================================================
    // 中央：QTableView + QStandardItemModel + QSortFilterProxyModel
    // ================================================================
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(
        {"编号", "姓名", "部门", "职位", "工龄", "绩效"});

    // 代理 Model：支持排序和过滤
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(
        Qt::CaseInsensitive);

    m_tableView = new QTableView;
    m_tableView->setModel(m_proxyModel);
    m_tableView->setSortingEnabled(true);
    m_tableView->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(
        QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(
        QAbstractItemView::DoubleClicked
        | QAbstractItemView::EditKeyPressed);
    m_tableView->verticalHeader()->hide();

    // 混合列宽策略
    m_tableView->horizontalHeader()
        ->setMinimumSectionSize(60);
    m_tableView->horizontalHeader()
        ->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()
        ->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()
        ->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()
        ->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()
        ->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()
        ->setStretchLastSection(true);

    mainLayout->addWidget(m_tableView, 1);

    // ================================================================
    // 底部：统计标签
    // ================================================================
    m_statsLabel = new QLabel;
    m_statsLabel->setStyleSheet(
        "padding: 4px 8px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #DDD;"
        "border-radius: 3px;");
    mainLayout->addWidget(m_statsLabel);

    setCentralWidget(centralWidget);

    // ================================================================
    // 信号连接
    // ================================================================
    // 搜索框实时过滤姓名
    connect(m_searchEdit, &QLineEdit::textChanged, this,
            &MainWindow::onFilterChanged);

    // 部门下拉框过滤
    connect(m_deptCombo, &QComboBox::currentTextChanged, this,
            &MainWindow::onFilterChanged);

    // 选中行变化时更新详情
    connect(m_tableView->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current) {
        if (!current.isValid()) return;
        // 通过 proxy 映射回 source model 获取数据
        QModelIndex sourceIndex =
            m_proxyModel->mapToSource(current);
        int row = sourceIndex.row();
        QString name =
            m_model->item(row, 1)
                ? m_model->item(row, 1)->text()
                : "";
        QString dept =
            m_model->item(row, 2)
                ? m_model->item(row, 2)->text()
                : "";
        m_statsLabel->setText(
            m_statsLabel->text()
            + QString(" | 选中: %1 (%2)")
                  .arg(name, dept));
    });
}

/// @brief 填充模拟的员工数据
void MainWindow::populateEmployeeData()
{
    struct Employee {
        QString id;
        QString name;
        QString dept;
        QString title;
        int years;
        QString perf;
    };

    const Employee employees[] = {
        {"E001", "陈一",   "研发部", "高级工程师", 5, "S"},
        {"E002", "刘二",   "研发部", "工程师",     3, "A"},
        {"E003", "张三",   "研发部", "实习生",     1, "B+"},
        {"E004", "李四",   "产品部", "产品经理",   6, "S"},
        {"E005", "王五",   "产品部", "产品助理",   2, "A"},
        {"E006", "赵六",   "设计部", "高级设计师", 4, "A+"},
        {"E007", "钱七",   "设计部", "设计师",     2, "B+"},
        {"E008", "孙八",   "运维部", "运维工程师", 3, "A"},
        {"E009", "周九",   "运维部", "运维主管",   7, "S"},
        {"E010", "吴十",   "研发部", "架构师",     8, "S+"},
    };

    for (const auto &e : employees) {
        QList<QStandardItem *> row;
        auto *idItem = new QStandardItem(e.id);
        idItem->setFlags(
            idItem->flags() & ~Qt::ItemIsEditable);
        row << idItem
            << new QStandardItem(e.name)
            << new QStandardItem(e.dept)
            << new QStandardItem(e.title)
            << new QStandardItem(QString::number(e.years))
            << new QStandardItem(e.perf);
        m_model->appendRow(row);
    }
}

/// @brief 对同一部门的行做合并展示
void MainWindow::applySpan()
{
    // 按部门分组，合并部门列中连续相同的行
    // 先按部门排序，让同部门的行排在一起
    // 这里用简单方式：手动查找同部门行
    QMap<QString, QList<int>> deptRows;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *deptItem = m_model->item(i, 2);
        if (deptItem) {
            deptRows[deptItem->text()].append(i);
        }
    }

    // 注意：setSpan 和排序功能配合需要谨慎
    // 当开启排序后，行顺序会变化，合并会错位
    // 此处仅做演示，实际项目中需要更细致的处理
    for (auto it = deptRows.constBegin();
         it != deptRows.constEnd(); ++it) {
        const QList<int> &rows = it.value();
        if (rows.size() > 1) {
            // 纵向合并部门列
            int startRow = rows.first();
            m_tableView->setSpan(
                startRow, 2, rows.size(), 1);
        }
    }
}

/// @brief 搜索/筛选变更时更新 proxy model 过滤条件
void MainWindow::onFilterChanged()
{
    QString searchText = m_searchEdit->text().trimmed();
    QString dept = m_deptCombo->currentText();

    // 构建过滤正则：姓名模糊匹配
    if (!searchText.isEmpty()) {
        QRegularExpression regex(
            QRegularExpression::wildcardToRegularExpression(
                "*" + searchText + "*"));
        m_proxyModel->setFilterRegularExpression(regex);
        m_proxyModel->setFilterKeyColumn(1);  // 姓名列
    } else {
        m_proxyModel->setFilterRegularExpression(
            QRegularExpression());
        m_proxyModel->setFilterKeyColumn(1);
    }

    // 部门过滤：用额外的行过滤
    if (dept != "全部") {
        // 代理 model 只能设一个过滤条件
        // 这里用 setFilterFixedString 做部门精确匹配
        // 搜索和部门同时过滤需要链式 proxy 或自定义逻辑
        m_proxyModel->setFilterKeyColumn(2);
        m_proxyModel->setFilterFixedString(dept);
    } else if (searchText.isEmpty()) {
        m_proxyModel->setFilterFixedString("");
    }

    updateStatistics();
}

/// @brief 更新底部统计数据
void MainWindow::updateStatistics()
{
    int count = m_proxyModel->rowCount();
    double totalYears = 0.0;
    int validCount = 0;

    for (int i = 0; i < count; ++i) {
        QModelIndex proxyIndex =
            m_proxyModel->index(i, 4);
        QModelIndex sourceIndex =
            m_proxyModel->mapToSource(proxyIndex);
        QStandardItem *yearsItem =
            m_model->item(sourceIndex.row(), 4);
        if (yearsItem) {
            bool ok = false;
            double years = yearsItem->text().toDouble(&ok);
            if (ok) {
                totalYears += years;
                ++validCount;
            }
        }
    }

    double avg = (validCount > 0)
                     ? (totalYears / validCount)
                     : 0.0;

    m_statsLabel->setText(
        QString("共 %1 人 | 平均工龄: %2 年")
            .arg(count)
            .arg(avg, 0, 'f', 1));
}
