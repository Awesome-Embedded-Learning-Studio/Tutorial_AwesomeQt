#include "mainwindow.h"
#include "studenttablepanel.h"
#include "namelistpanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// 主窗口：组合表格面板和姓名列表面板
// ============================================================================
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Model/View 架构示例 — 学生成绩管理");
    resize(800, 500);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // 说明标签
    auto *descLabel = new QLabel(
        "上半部分：QStandardItemModel + QTableView（双击编辑，右键删除）\n"
        "下半部分：QStringListModel + QListView（与表格姓名列同步）");
    descLabel->setStyleSheet("color: #555; font-size: 12px; padding: 4px;");
    mainLayout->addWidget(descLabel);

    // 使用 QSplitter 让上下两部分可以拖拽调整大小
    auto *splitter = new QSplitter(Qt::Vertical);

    m_tablePanel = new StudentTablePanel;
    m_namePanel = new NameListPanel;

    splitter->addWidget(m_tablePanel);
    splitter->addWidget(m_namePanel);
    splitter->setSizes({350, 150});

    mainLayout->addWidget(splitter, 1);

    // ---- 数据同步：表格 → 列表 ----
    connect(m_tablePanel, &StudentTablePanel::namesChanged, this, [this]() {
        m_namePanel->setNames(m_tablePanel->getStudentNames());
    });

    // 初始化列表
    m_namePanel->setNames(m_tablePanel->getStudentNames());

    // ---- 数据同步：列表 → 表格 ----
    connect(m_namePanel, &NameListPanel::addStudentRequested, this,
            [this](const QString &name) {
                // 默认成绩为 0，用户可以在表格里修改
                m_tablePanel->addStudent(name, 0, 0, 0);
            });

    connect(m_namePanel, &NameListPanel::removeStudentRequested, this,
            [this](int row) {
                if (row >= 0 && row < m_tablePanel->model()->rowCount()) {
                    m_tablePanel->model()->removeRow(row);
                    m_namePanel->setNames(m_tablePanel->getStudentNames());
                }
            });

    connect(m_namePanel, &NameListPanel::nameEdited, this,
            [this](int row, const QString &newName) {
                // 用 setData() 修改表格的第 0 列
                QModelIndex nameIndex = m_tablePanel->model()->index(row, 0);
                m_tablePanel->model()->setData(nameIndex, newName, Qt::EditRole);
            });

    // 添加一个底部状态栏
    auto *statusLayout = new QHBoxLayout;
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("color: #888; font-size: 11px; padding: 2px;");
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();

    auto *refreshBtn = new QPushButton("刷新统计");
    connect(refreshBtn, &QPushButton::clicked, this, [this]() {
        updateStatus();
    });
    statusLayout->addWidget(refreshBtn);
    mainLayout->addLayout(statusLayout);

    updateStatus();

    // 每次 namesChanged 都更新状态
    connect(m_tablePanel, &StudentTablePanel::namesChanged, this,
            &MainWindow::updateStatus);
}

void MainWindow::updateStatus()
{
    int count = m_tablePanel->model()->rowCount();
    m_statusLabel->setText(
        QString("共 %1 名学生  |  表格 QStandardItemModel: %2 行 x %3 列")
            .arg(count)
            .arg(m_tablePanel->model()->rowCount())
            .arg(m_tablePanel->model()->columnCount()));
}
