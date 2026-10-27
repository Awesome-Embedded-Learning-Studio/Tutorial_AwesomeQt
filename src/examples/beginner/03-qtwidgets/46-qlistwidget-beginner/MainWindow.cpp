// QtWidgets 入门示例 46: QListWidget 便捷列表控件
// 演示：addItem / addItems / insertItem 添加条目
//       currentItem / selectedItems 获取选中
//       QListWidgetItem 图标/复选框/自定义数据
//       itemDoubleClicked / itemChanged 信号

#include "MainWindow.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// MainWindow: QListWidget 综合演示（待办任务管理器）
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QListWidget 综合演示 — 待办任务管理器");
    resize(480, 540);
    initUi();
}

void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // ================================================================
    // 顶部：添加任务输入栏
    // ================================================================
    auto *inputLayout = new QHBoxLayout;

    m_taskInput = new QLineEdit;
    m_taskInput->setPlaceholderText("输入新任务名称，按回车或点击添加...");

    auto *addBtn = new QPushButton("添加任务");
    addBtn->setFixedWidth(90);

    inputLayout->addWidget(m_taskInput);
    inputLayout->addWidget(addBtn);

    // ================================================================
    // 中间：任务列表 QListWidget
    // ================================================================
    m_taskList = new QListWidget;
    m_taskList->setSelectionMode(
        QAbstractItemView::ExtendedSelection);
    m_taskList->setContextMenuPolicy(Qt::CustomContextMenu);

    // ================================================================
    // 底部：状态栏 + 操作按钮
    // ================================================================
    auto *bottomLayout = new QHBoxLayout;

    m_statusLabel = new QLabel;
    updateStatusText();

    auto *deleteBtn = new QPushButton("删除选中");
    auto *clearDoneBtn = new QPushButton("清除已完成");

    bottomLayout->addWidget(m_statusLabel, 1);
    bottomLayout->addWidget(deleteBtn);
    bottomLayout->addWidget(clearDoneBtn);

    // 组装主布局
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_taskList, 1);
    mainLayout->addLayout(bottomLayout);

    setCentralWidget(centralWidget);

    // ================================================================
    // 信号连接
    // ================================================================
    connect(addBtn, &QPushButton::clicked, this,
            &MainWindow::onAddTask);
    connect(m_taskInput, &QLineEdit::returnPressed, this,
            &MainWindow::onAddTask);
    connect(deleteBtn, &QPushButton::clicked, this,
            &MainWindow::onDeleteSelected);
    connect(clearDoneBtn, &QPushButton::clicked, this,
            &MainWindow::onClearDone);

    // 条目变化时更新样式和状态文字
    connect(m_taskList, &QListWidget::itemChanged, this,
            &MainWindow::onItemChanged);

    // 双击编辑任务名称
    connect(m_taskList, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onItemDoubleClicked);

    // 右键上下文菜单
    connect(m_taskList, &QListWidget::customContextMenuRequested,
            this, &MainWindow::onContextMenu);

    // 选中变化时更新状态文字
    connect(m_taskList, &QListWidget::itemSelectionChanged,
            this, &MainWindow::updateStatusText);

    // 预填充任务（在 m_statusLabel 初始化之后）
    addTask("完成 Qt 教程第 46 篇", 1);
    addTask("修复项目编译警告", 2);
    addTask("更新 README 文档", 3);
    addTask("代码审查 PR #42", 2);
    addTask("部署测试环境", 1);
}

void MainWindow::addTask(const QString &name, int priority)
{
    auto *item = new QListWidgetItem;
    item->setText(name);
    item->setCheckState(Qt::Unchecked);
    item->setData(kPriorityRole, priority);

    // 根据优先级设置文字颜色
    applyPriorityStyle(item, priority);

    m_taskList->addItem(item);
    updateStatusText();
}

void MainWindow::applyPriorityStyle(QListWidgetItem *item, int priority)
{
    switch (priority) {
    case 1:
        item->setForeground(QColor("#D32F2F"));
        break;
    case 2:
        item->setForeground(QColor("#F57C00"));
        break;
    default:
        item->setForeground(QColor("#333333"));
        break;
    }
}

void MainWindow::onAddTask()
{
    QString text = m_taskInput->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    addTask(text, 3);
    m_taskInput->clear();
    // 滚动到底部显示新添加的任务
    m_taskList->scrollToBottom();
}

void MainWindow::onDeleteSelected()
{
    const auto selected = m_taskList->selectedItems();
    for (auto *item : selected) {
        int row = m_taskList->row(item);
        delete m_taskList->takeItem(row);
    }
    updateStatusText();
}

void MainWindow::onClearDone()
{
    int row = m_taskList->count() - 1;
    while (row >= 0) {
        auto *item = m_taskList->item(row);
        if (item && item->checkState() == Qt::Checked) {
            delete m_taskList->takeItem(row);
        }
        --row;
    }
    updateStatusText();
}

void MainWindow::onItemChanged(QListWidgetItem *item)
{
    if (!item) return;

    // 阻塞信号避免递归
    m_taskList->blockSignals(true);

    if (item->checkState() == Qt::Checked) {
        // 已完成：灰色 + 删除线
        QFont font = item->font();
        font.setStrikeOut(true);
        item->setFont(font);
        item->setForeground(QColor("#AAAAAA"));
    } else {
        // 未完成：恢复优先级样式
        QFont font = item->font();
        font.setStrikeOut(false);
        item->setFont(font);
        int priority = item->data(kPriorityRole).toInt();
        applyPriorityStyle(item, priority);
    }

    m_taskList->blockSignals(false);
    updateStatusText();
}

void MainWindow::onItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "修改任务名称",
        "请输入新的任务名称:",
        QLineEdit::Normal,
        item->text(),
        &ok);

    if (ok && !newName.trimmed().isEmpty()) {
        m_taskList->blockSignals(true);
        item->setText(newName.trimmed());
        m_taskList->blockSignals(false);
    }
}

void MainWindow::onContextMenu(const QPoint &pos)
{
    auto *item = m_taskList->itemAt(pos);
    if (!item) return;

    auto *menu = new QMenu(this);
    menu->addAction("设为高优先级", [this, item]() {
        m_taskList->blockSignals(true);
        item->setData(kPriorityRole, 1);
        if (item->checkState() != Qt::Checked) {
            applyPriorityStyle(item, 1);
        }
        m_taskList->blockSignals(false);
    });
    menu->addAction("设为中优先级", [this, item]() {
        m_taskList->blockSignals(true);
        item->setData(kPriorityRole, 2);
        if (item->checkState() != Qt::Checked) {
            applyPriorityStyle(item, 2);
        }
        m_taskList->blockSignals(false);
    });
    menu->addAction("设为普通优先级", [this, item]() {
        m_taskList->blockSignals(true);
        item->setData(kPriorityRole, 3);
        if (item->checkState() != Qt::Checked) {
            applyPriorityStyle(item, 3);
        }
        m_taskList->blockSignals(false);
    });
    menu->addSeparator();
    menu->addAction("删除此任务", [this, item]() {
        int row = m_taskList->row(item);
        delete m_taskList->takeItem(row);
        updateStatusText();
    });

    menu->exec(m_taskList->mapToGlobal(pos));
    menu->deleteLater();
}

void MainWindow::updateStatusText()
{
    int total = m_taskList->count();
    int done = 0;
    for (int i = 0; i < total; ++i) {
        if (m_taskList->item(i)->checkState() == Qt::Checked) {
            ++done;
        }
    }
    int selected = m_taskList->selectedItems().size();

    m_statusLabel->setText(
        QString("总计: %1  已完成: %2  选中: %3")
            .arg(total)
            .arg(done)
            .arg(selected));
}
