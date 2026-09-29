#include "studenttablepanel.h"

#include <QAbstractItemView>
#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// 学生成绩表格面板：QStandardItemModel + QTableView
// ============================================================================
StudentTablePanel::StudentTablePanel(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // ---- 创建 Model 并填充初始数据 ----
    m_model = new QStandardItemModel(0, 4, this);
    m_model->setHorizontalHeaderLabels({"姓名", "语文", "数学", "英语"});

    // 初始数据：四名同学
    addStudentRaw("张三", 92, 88, 95);
    addStudentRaw("李四", 85, 91, 78);
    addStudentRaw("王五", 78, 95, 82);
    addStudentRaw("赵六", 90, 76, 88);

    // ---- 创建 View 并绑定 Model ----
    m_tableView = new QTableView;
    m_tableView->setModel(m_model);
    m_tableView->setEditTriggers(
        QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 列宽自适应
    m_tableView->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    for (int col = 1; col < 4; ++col) {
        m_tableView->horizontalHeader()->setSectionResizeMode(
            col, QHeaderView::ResizeToContents);
    }

    layout->addWidget(m_tableView);

    // ---- 右键菜单：删除行 ----
    connect(m_tableView, &QTableView::customContextMenuRequested, this,
            &StudentTablePanel::showContextMenu);

    // ---- 监听数据变化，通知外部 ----
    connect(m_model, &QStandardItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                // 检查是否有姓名列（第 0 列）的变化
                if (topLeft.column() == 0 || bottomRight.column() == 0) {
                    emit namesChanged();
                }
            });
}

QStringList StudentTablePanel::getStudentNames() const
{
    QStringList names;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        names << m_model->item(row, 0)->text();
    }
    return names;
}

void StudentTablePanel::addStudent(const QString &name, int chinese, int math, int english)
{
    addStudentRaw(name, chinese, math, english);
    emit namesChanged();
}

QStandardItemModel *StudentTablePanel::model() const { return m_model; }

void StudentTablePanel::addStudentRaw(const QString &name, int chinese, int math, int english)
{
    int row = m_model->rowCount();
    m_model->setItem(row, 0, new QStandardItem(name));
    m_model->setItem(row, 1, new QStandardItem(QString::number(chinese)));
    m_model->setItem(row, 2, new QStandardItem(QString::number(math)));
    m_model->setItem(row, 3, new QStandardItem(QString::number(english)));
}

void StudentTablePanel::showContextMenu(const QPoint &pos)
{
    QModelIndex index = m_tableView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    QMenu menu;
    QAction *deleteAction = menu.addAction("删除该行");
    QAction *chosen = menu.exec(m_tableView->viewport()->mapToGlobal(pos));

    if (chosen == deleteAction) {
        int row = index.row();
        QString name = m_model->item(row, 0)->text();
        auto reply = QMessageBox::question(
            this, "确认删除",
            QString("确定要删除 %1 的记录吗？").arg(name),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            m_model->removeRow(row);
            emit namesChanged();
        }
    }
}
