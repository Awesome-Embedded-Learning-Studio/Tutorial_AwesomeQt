/**
 * QSqlTableModel 数据库表格视图示例
 *
 * 本示例演示 QSqlTableModel + QTableView 的完整工作流：
 * 1. Model 创建与表绑定
 * 2. EditStrategy 三种策略（本例用 OnManualSubmit）
 * 3. setFilter / setSort 过滤与排序
 * 4. insertRow / removeRow 新增与删除
 * 5. submitAll / revertAll 提交与回滚
 * 6. 自定义表头与列可见性
 */

#include "MainWindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QTableView>
#include <QToolBar>
#include <QAction>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

// ============================================================================
// 主窗口：QSqlTableModel + QTableView + 工具栏 + 筛选栏
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QSqlTableModel 示例 — 员工管理");
    resize(800, 500);

    setupModel();
    setupView();
    setupToolbar();
    setupFilterBar();

    // 主布局
    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->addLayout(m_filterLayout);
    mainLayout->addWidget(m_tableView);

    setCentralWidget(centralWidget);
}

void MainWindow::setupModel()
{
    m_model = new QSqlTableModel(this);
    m_model->setTable("employees");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 自定义表头
    m_model->setHeaderData(0, Qt::Horizontal, "ID");
    m_model->setHeaderData(1, Qt::Horizontal, "姓名");
    m_model->setHeaderData(2, Qt::Horizontal, "部门");
    m_model->setHeaderData(3, Qt::Horizontal, "薪资");
    m_model->setHeaderData(4, Qt::Horizontal, "入职日期");

    // 默认按薪资降序
    m_model->setSort(3, Qt::DescendingOrder);
    m_model->select();
}

void MainWindow::setupView()
{
    m_tableView = new QTableView;
    m_tableView->setModel(m_model);

    // 隐藏 ID 列
    m_tableView->hideColumn(0);

    // 交替行背景色
    m_tableView->setAlternatingRowColors(true);

    // 选中整行
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置列宽
    m_tableView->setColumnWidth(1, 150);
    m_tableView->setColumnWidth(2, 130);
    m_tableView->setColumnWidth(3, 100);
    m_tableView->setColumnWidth(4, 130);
}

void MainWindow::setupToolbar()
{
    auto *toolbar = addToolBar("操作");

    auto *addAction = toolbar->addAction("新增");
    auto *deleteAction = toolbar->addAction("删除");
    toolbar->addSeparator();
    auto *saveAction = toolbar->addAction("保存");
    auto *revertAction = toolbar->addAction("撤销");
    toolbar->addSeparator();
    auto *refreshAction = toolbar->addAction("刷新");

    connect(addAction, &QAction::triggered, this, &MainWindow::addRow);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteRow);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveChanges);
    connect(revertAction, &QAction::triggered, this, &MainWindow::revertChanges);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshData);
}

void MainWindow::setupFilterBar()
{
    m_filterLayout = new QHBoxLayout;

    m_filterLayout->addWidget(new QLabel("姓名筛选:"));
    m_nameFilterEdit = new QLineEdit;
    m_nameFilterEdit->setPlaceholderText("输入关键字...");
    m_filterLayout->addWidget(m_nameFilterEdit);

    m_filterLayout->addWidget(new QLabel("部门:"));
    m_deptFilterEdit = new QLineEdit;
    m_deptFilterEdit->setPlaceholderText("输入部门名称...");
    m_filterLayout->addWidget(m_deptFilterEdit);

    auto *filterBtn = new QPushButton("筛选");
    auto *clearFilterBtn = new QPushButton("清除");
    m_filterLayout->addWidget(filterBtn);
    m_filterLayout->addWidget(clearFilterBtn);
    m_filterLayout->addStretch();

    connect(filterBtn, &QPushButton::clicked, this, &MainWindow::applyFilter);
    connect(clearFilterBtn, &QPushButton::clicked, this, &MainWindow::clearFilter);
    connect(m_nameFilterEdit, &QLineEdit::returnPressed, this, &MainWindow::applyFilter);
    connect(m_deptFilterEdit, &QLineEdit::returnPressed, this, &MainWindow::applyFilter);
}

// === 操作槽函数 ===

void MainWindow::addRow()
{
    int row = m_model->rowCount();
    if (!m_model->insertRow(row)) {
        QMessageBox::warning(this, "错误", "无法新增行");
        return;
    }

    // 选中并滚动到新行
    m_tableView->selectRow(row);
    m_tableView->scrollToBottom();
    m_tableView->edit(m_model->index(row, 1));  // 自动进入编辑模式
}

void MainWindow::deleteRow()
{
    int row = m_tableView->currentIndex().row();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选中要删除的行");
        return;
    }

    QString name = m_model->data(m_model->index(row, 1)).toString();
    auto reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除员工 \"%1\" 吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_model->removeRow(row);
        // OnManualSubmit 策略下，删除在 submitAll 时生效
    }
}

void MainWindow::saveChanges()
{
    if (!m_model->submitAll()) {
        QMessageBox::warning(this, "保存失败",
                             m_model->lastError().text());
    } else {
        m_model->select();  // 刷新显示
        qDebug() << "数据保存成功";
    }
}

void MainWindow::revertChanges()
{
    m_model->revertAll();
    qDebug() << "已撤销所有未保存的修改";
}

void MainWindow::refreshData()
{
    m_model->select();
    qDebug() << "数据已刷新";
}

void MainWindow::applyFilter()
{
    QStringList conditions;

    QString nameKeyword = m_nameFilterEdit->text().trimmed();
    if (!nameKeyword.isEmpty()) {
        conditions << QString("name LIKE '%%1%'").arg(nameKeyword);
    }

    QString deptKeyword = m_deptFilterEdit->text().trimmed();
    if (!deptKeyword.isEmpty()) {
        conditions << QString("department LIKE '%%1%'").arg(deptKeyword);
    }

    QString filter = conditions.join(" AND ");
    m_model->setFilter(filter);
    m_model->select();

    qDebug() << "筛选条件:" << (filter.isEmpty() ? "无" : filter);
}

void MainWindow::clearFilter()
{
    m_nameFilterEdit->clear();
    m_deptFilterEdit->clear();
    m_model->setFilter("");
    m_model->select();
    qDebug() << "筛选已清除";
}
