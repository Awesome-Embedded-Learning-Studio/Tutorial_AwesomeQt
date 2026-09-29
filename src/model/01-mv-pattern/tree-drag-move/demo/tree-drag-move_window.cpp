/**
 * @file tree-drag-move_window.cpp
 * @brief TreeDragMove 演示实现——两棵树 + 拖放标志位 + 种子数据
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "tree-drag-move_window.h"

#include <QAction>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include "tree-drag-move.h"

TreeDragMoveWindow::TreeDragMoveWindow(QWidget* parent) : QMainWindow(parent) {
    setup_trees();
    setup_status_bar();

    setWindowTitle("TreeDragMove Demo —— QTreeWidget 拖放协议");
    resize(900, 600);
}

void TreeDragMoveWindow::setup_trees() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    left_tree_ = new AwesomeQt::TreeDragMove(this);
    right_tree_ = new AwesomeQt::TreeDragMove(this);

    // 教学点①：拖放四件套标志位——这是 QAbstractItemView 的拖放总开关
    // InternalMove：源由框架标记删除（实际删源见库类 dropEvent）
    left_tree_->setHeaderLabel("项目结构（拖我改层级 / 拖到右边转移）");
    left_tree_->setDragEnabled(true);
    left_tree_->setAcceptDrops(true);
    left_tree_->setDragDropMode(QAbstractItemView::InternalMove);
    left_tree_->setDefaultDropAction(Qt::MoveAction);

    // 右树同样开接收，作为「转移目的地」
    right_tree_->setHeaderLabel("回收区（从左边拖过来）");
    right_tree_->setDragEnabled(true);
    right_tree_->setAcceptDrops(true);
    right_tree_->setDragDropMode(QAbstractItemView::InternalMove);

    seed_left_tree();
    seed_right_tree();

    splitter->addWidget(left_tree_);
    splitter->addWidget(right_tree_);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    setCentralWidget(splitter);
}

void TreeDragMoveWindow::setup_status_bar() {
    status_label_ = new QLabel("提示：单树内拖 = 改层级（落节点上成子节点 / 落空白成同级）；"
                               "跨树拖 = 转移。Ctrl+拖 = 复制保留源。",
                               this);
    status_label_->setWordWrap(true);
    statusBar()->addWidget(status_label_, 1);

    auto* edit_menu = menuBar()->addMenu("&Edit");
    auto* expand_all = new QAction("Expand &All", this);
    expand_all->setShortcut(QKeySequence("Ctrl+E"));
    connect(expand_all, &QAction::triggered, this, [this]() {
        left_tree_->expandAll();
        right_tree_->expandAll();
    });
    edit_menu->addAction(expand_all);

    auto* collapse_all = new QAction("&Collapse All", this);
    collapse_all->setShortcut(QKeySequence("Ctrl+L"));
    connect(collapse_all, &QAction::triggered, this, [this]() {
        left_tree_->collapseAll();
        right_tree_->collapseAll();
    });
    edit_menu->addAction(collapse_all);
}

void TreeDragMoveWindow::seed_left_tree() {
    // 种一棵「项目结构」树，带两层子节点，方便演示层级变更
    auto* src = new QTreeWidgetItem({"src"});
    new QTreeWidgetItem(src, {"main.cpp"});
    new QTreeWidgetItem(src, {"widget.cpp"});
    auto* include = new QTreeWidgetItem(src, {"include"});
    new QTreeWidgetItem(include, {"widget.h"});
    auto* tests = new QTreeWidgetItem({"tests"});
    new QTreeWidgetItem(tests, {"test_widget.cpp"});
    auto* docs = new QTreeWidgetItem({"docs"});

    left_tree_->addTopLevelItem(src);
    left_tree_->addTopLevelItem(tests);
    left_tree_->addTopLevelItem(docs);
    left_tree_->expandAll();
}

void TreeDragMoveWindow::seed_right_tree() {
    // 回收区先放一个占位顶级节点，让「落到节点上 vs 落空白」两种落点都好演示
    auto* inbox = new QTreeWidgetItem({"Inbox（拖到这里成子项）"});
    right_tree_->addTopLevelItem(inbox);
    right_tree_->expandAll();
}
