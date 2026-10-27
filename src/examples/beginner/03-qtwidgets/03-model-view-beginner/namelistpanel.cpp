#include "namelistpanel.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// 姓名列表面板：QStringListModel + QListView
// ============================================================================
NameListPanel::NameListPanel(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // 标题
    auto *titleLabel = new QLabel("学生姓名列表");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 4px;");
    layout->addWidget(titleLabel);

    // ---- 创建 Model 和 View ----
    m_model = new QStringListModel(this);
    m_listView = new QListView;
    m_listView->setModel(m_model);
    m_listView->setEditTriggers(
        QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_listView->setAlternatingRowColors(true);
    layout->addWidget(m_listView, 1);

    // ---- 操作按钮 ----
    auto *btnLayout = new QHBoxLayout;

    auto *addBtn = new QPushButton("添加");
    connect(addBtn, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        QString name = QInputDialog::getText(
            this, "添加学生", "请输入姓名：", QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            emit addStudentRequested(name.trimmed());
        }
    });

    auto *removeBtn = new QPushButton("删除");
    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        QModelIndex idx = m_listView->currentIndex();
        if (idx.isValid()) {
            emit removeStudentRequested(idx.row());
        }
    });

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    layout->addLayout(btnLayout);

    // ---- 监听列表编辑，同步回表格 ----
    connect(m_model, &QStringListModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
                    QString newName = m_model->data(m_model->index(row),
                                                    Qt::DisplayRole).toString();
                    emit nameEdited(row, newName);
                }
            });
}

void NameListPanel::setNames(const QStringList &names)
{
    m_model->setStringList(names);
}

QStringListModel *NameListPanel::model() const { return m_model; }
