#include "MainWindow.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QTreeWidget 综合演示 — 项目文件浏览器");
    resize(640, 500);
    initUi();
    populateProjectTree();
}

void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainHLayout = new QHBoxLayout(centralWidget);

    // ================================================================
    // 左侧：QTreeWidget（三列树表）
    // ================================================================
    m_treeWidget = new QTreeWidget;
    m_treeWidget->setColumnCount(3);
    m_treeWidget->setHeaderLabels({"名称", "类型", "大小"});
    m_treeWidget->setSelectionMode(
        QAbstractItemView::SingleSelection);

    // 表头列宽策略
    m_treeWidget->header()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setStretchLastSection(true);

    // ================================================================
    // 右侧：信息面板 + 操作面板
    // ================================================================
    auto *rightPanel = new QWidget;
    rightPanel->setFixedWidth(220);
    auto *rightLayout = new QVBoxLayout(rightPanel);

    // 路径信息
    rightLayout->addWidget(new QLabel("当前路径:"));
    m_pathLabel = new QLabel("未选中");
    m_pathLabel->setWordWrap(true);
    m_pathLabel->setStyleSheet(
        "padding: 8px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #DDD;"
        "border-radius: 4px;");
    rightLayout->addWidget(m_pathLabel);

    // 名称输入
    rightLayout->addWidget(new QLabel("节点名称:"));
    m_nameInput = new QLineEdit;
    m_nameInput->setPlaceholderText("输入文件/文件夹名");
    rightLayout->addWidget(m_nameInput);

    // 操作按钮
    auto *addFolderBtn = new QPushButton("添加文件夹");
    auto *addFileBtn = new QPushButton("添加文件");
    auto *deleteBtn = new QPushButton("删除选中节点");
    deleteBtn->setStyleSheet(
        "QPushButton { color: #D32F2F; }");

    rightLayout->addWidget(addFolderBtn);
    rightLayout->addWidget(addFileBtn);
    rightLayout->addWidget(deleteBtn);

    rightLayout->addStretch();

    // 展开折叠状态标签
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setWordWrap(true);
    rightLayout->addWidget(m_statusLabel);

    // 组装水平布局
    mainHLayout->addWidget(m_treeWidget, 1);
    mainHLayout->addWidget(rightPanel);

    setCentralWidget(centralWidget);

    // ================================================================
    // 信号连接
    // ================================================================
    // 选中变化时更新路径
    connect(m_treeWidget, &QTreeWidget::currentItemChanged,
            this, &MainWindow::onCurrentItemChanged);

    // 单击节点
    connect(m_treeWidget, &QTreeWidget::itemClicked,
            this, [this](QTreeWidgetItem *item, int column) {
        if (!item) return;
        m_statusLabel->setText(
            QString("点击: %1 (列 %2)")
                .arg(item->text(0))
                .arg(column));
    });

    // 展开节点
    connect(m_treeWidget, &QTreeWidget::itemExpanded,
            this, [this](QTreeWidgetItem *item) {
        if (!item) return;
        m_statusLabel->setText(
            QString("已展开: %1").arg(item->text(0)));
    });

    // 折叠节点
    connect(m_treeWidget, &QTreeWidget::itemCollapsed,
            this, [this](QTreeWidgetItem *item) {
        if (!item) return;
        m_statusLabel->setText(
            QString("已折叠: %1").arg(item->text(0)));
    });

    // 双击编辑
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked,
            this, &MainWindow::onItemDoubleClicked);

    // 添加文件夹
    connect(addFolderBtn, &QPushButton::clicked, this,
            [this]() { onAddNode("文件夹"); });

    // 添加文件
    connect(addFileBtn, &QPushButton::clicked, this,
            [this]() { onAddNode("文件"); });

    // 删除节点
    connect(deleteBtn, &QPushButton::clicked, this,
            &MainWindow::onDeleteNode);
}

void MainWindow::populateProjectTree()
{
    // 根节点
    auto *root = new QTreeWidgetItem(m_treeWidget);
    root->setText(0, "QtProject");
    root->setText(1, "项目");
    root->setText(2, "--");
    root->setExpanded(true);

    // src 文件夹
    auto *src = createChild(root, "src", "文件夹", "--");

    createChild(src, "main.cpp", "C++ 源文件", "2.1 KB");
    createChild(src, "widget.h", "C++ 头文件", "1.8 KB");
    createChild(src, "widget.cpp", "C++ 源文件", "5.6 KB");

    // include 文件夹
    auto *include = createChild(root, "include", "文件夹", "--");

    createChild(include, "widget.h", "C++ 头文件", "1.8 KB");
    createChild(include, "helper.h", "C++ 头文件", "0.9 KB");

    // resources 文件夹
    auto *res = createChild(root, "resources", "文件夹", "--");

    createChild(res, "icon.png", "图片文件", "24.3 KB");
    createChild(res, "style.qss", "样式表", "3.1 KB");

    // 根目录下的文件
    createChild(root, "CMakeLists.txt", "构建文件", "1.2 KB");
    createChild(root, "README.md", "Markdown", "0.5 KB");
}

QTreeWidgetItem *MainWindow::createChild(QTreeWidgetItem *parent,
                                         const QString &name,
                                         const QString &type,
                                         const QString &size)
{
    auto *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, type);
    item->setText(2, size);
    return item;
}

QString MainWindow::getNodePath(QTreeWidgetItem *item) const
{
    if (!item) return "未选中";

    QStringList parts;
    QTreeWidgetItem *current = item;
    while (current) {
        parts.prepend(current->text(0));
        current = current->parent();
    }
    return parts.join(" / ");
}

void MainWindow::onCurrentItemChanged(QTreeWidgetItem *current,
                                       QTreeWidgetItem * /*previous*/)
{
    m_pathLabel->setText(getNodePath(current));
}

void MainWindow::onItemDoubleClicked(QTreeWidgetItem *item, int /*column*/)
{
    if (!item) return;

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "修改名称",
        "请输入新的名称:",
        QLineEdit::Normal,
        item->text(0),
        &ok);

    if (ok && !newName.trimmed().isEmpty()) {
        m_treeWidget->blockSignals(true);
        item->setText(0, newName.trimmed());
        m_treeWidget->blockSignals(false);
        // 更新路径显示
        m_pathLabel->setText(getNodePath(item));
    }
}

void MainWindow::onAddNode(const QString &nodeType)
{
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty()) {
        m_nameInput->setStyleSheet(
            "border: 1px solid #E53935;");
        return;
    }
    m_nameInput->setStyleSheet("");

    auto *parent = m_treeWidget->currentItem();
    if (!parent) {
        // 没有选中节点时添加为顶层节点
        parent = m_treeWidget->invisibleRootItem();
    }

    QString type =
        (nodeType == "文件夹") ? "文件夹" : "文件";
    QString size = (nodeType == "文件夹") ? "--" : "0 KB";

    m_treeWidget->blockSignals(true);
    auto *newItem = createChild(parent, name, type, size);
    parent->setExpanded(true);
    m_treeWidget->blockSignals(false);

    m_nameInput->clear();
    m_treeWidget->setCurrentItem(newItem);
    m_pathLabel->setText(getNodePath(newItem));
}

void MainWindow::onDeleteNode()
{
    auto *current = m_treeWidget->currentItem();
    if (!current) {
        m_statusLabel->setText("请先选中一个节点");
        return;
    }

    QString name = current->text(0);
    int childCount = current->childCount();

    QString msg = QString("确定删除 \"%1\"?").arg(name);
    if (childCount > 0) {
        msg += QString("\n该节点包含 %1 个子节点，将一并删除。")
                   .arg(childCount);
    }

    auto result = QMessageBox::question(
        this, "确认删除", msg,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (result == QMessageBox::Yes) {
        delete current;
        m_pathLabel->setText("未选中");
        m_statusLabel->setText(
            QString("已删除: %1").arg(name));
    }
}
