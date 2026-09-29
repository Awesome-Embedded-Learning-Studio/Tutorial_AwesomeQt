/// @file    big_table_model.cpp
/// @brief   BigTableModel 和 BigTableWindow 的实现。

#include "big_table_model.h"

#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
    constexpr int kDefaultRows = 1000000;
    constexpr int kDefaultCols = 10;
}

BigTableModel::BigTableModel(int rowCount, int columnCount, QObject* parent)
    : QAbstractTableModel(parent)
    , m_rowCount(rowCount)
    , m_columnCount(columnCount)
{
}

int BigTableModel::rowCount(const QModelIndex& parent) const
{
    // 顶层（parent 无效）才有行，子项无行
    if (parent.isValid()) {
        return 0;
    }
    return m_rowCount;
}

int BigTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_columnCount;
}

QVariant BigTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        // 数据完全按需生成——不存储、不缓存、不预计算
        return QString("Row %1, Col %2")
            .arg(index.row() + 1)
            .arg(index.column() + 1);
    }

    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return {};
}

QVariant BigTableModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        return QString("Column %1").arg(section + 1);
    }

    // 纵向表头直接显示行号
    return QString::number(section + 1);
}

BigTableWindow::BigTableWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_model(new BigTableModel(kDefaultRows, kDefaultCols, this))
{
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    auto* tableView = new QTableView(this);
    tableView->setModel(m_model);

    // 开启行高亮和交替行背景色，方便观察虚拟滚动效果
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(tableView);
    setCentralWidget(centralWidget);

    // 状态栏显示总行数，直观证明百万行模型可以流畅滚动
    statusBar()->showMessage(
        QString("Total rows: %1  |  Data is generated on-the-fly in data()")
            .arg(kDefaultRows));

    setWindowTitle("QTableView 百万行虚拟滚动");
    resize(900, 600);
}
