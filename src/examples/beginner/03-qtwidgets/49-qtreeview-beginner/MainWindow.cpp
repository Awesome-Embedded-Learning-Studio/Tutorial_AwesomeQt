#include "MainWindow.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QTreeView 综合演示 — 文件浏览器");
    resize(720, 520);
    initUi();
}

void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // ================================================================
    // 顶部：路径导航栏
    // ================================================================
    auto *navLayout = new QHBoxLayout;
    navLayout->addWidget(new QLabel("路径:"));

    m_pathInput = new QLineEdit;
    m_pathInput->setText(QDir::homePath());

    auto *goBtn = new QPushButton("跳转");
    goBtn->setFixedWidth(60);

    navLayout->addWidget(m_pathInput, 1);
    navLayout->addWidget(goBtn);

    mainLayout->addLayout(navLayout);

    // ================================================================
    // 中间：文件树 + 信息面板
    // ================================================================
    auto *contentLayout = new QHBoxLayout;

    // 文件系统 Model
    m_fsModel = new QFileSystemModel(this);
    m_fsModel->setRootPath(QDir::homePath());
    m_fsModel->setFilter(QDir::AllDirs | QDir::Files
                         | QDir::NoDotAndDotDot);

    // 文件树视图
    m_treeView = new QTreeView;
    m_treeView->setModel(m_fsModel);
    m_treeView->setSelectionMode(
        QAbstractItemView::SingleSelection);

    // 设置显示根节点为用户主目录
    QModelIndex rootIndex =
        m_fsModel->index(QDir::homePath());
    if (rootIndex.isValid()) {
        m_treeView->setRootIndex(rootIndex);
    }

    // 只显示文件名列，隐藏大小/类型/修改日期
    m_treeView->setColumnHidden(1, true);
    m_treeView->setColumnHidden(2, true);
    m_treeView->setColumnHidden(3, true);

    m_treeView->header()->setStretchLastSection(true);

    // 信息面板
    auto *infoPanel = new QWidget;
    infoPanel->setFixedWidth(240);
    auto *infoLayout = new QVBoxLayout(infoPanel);

    infoLayout->addWidget(new QLabel("文件信息:"));

    m_infoLabel = new QLabel("点击左侧文件查看详情");
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_infoLabel->setStyleSheet(
        "padding: 10px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #DDD;"
        "border-radius: 4px;");
    infoLayout->addWidget(m_infoLabel, 1);

    contentLayout->addWidget(m_treeView, 1);
    contentLayout->addWidget(infoPanel);

    mainLayout->addLayout(contentLayout, 1);

    // ================================================================
    // 底部：展开控制按钮 + 状态栏
    // ================================================================
    auto *btnLayout = new QHBoxLayout;

    auto *expandAllBtn = new QPushButton("展开全部");
    auto *collapseAllBtn = new QPushButton("折叠全部");
    auto *expandSelBtn = new QPushButton("展开选中");
    auto *collapseSelBtn = new QPushButton("折叠选中");

    btnLayout->addWidget(expandAllBtn);
    btnLayout->addWidget(collapseAllBtn);
    btnLayout->addWidget(expandSelBtn);
    btnLayout->addWidget(collapseSelBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    // 底部状态
    m_statusLabel = new QLabel("就绪");
    mainLayout->addWidget(m_statusLabel);

    setCentralWidget(centralWidget);

    // ================================================================
    // 信号连接
    // ================================================================
    // 点击文件时更新信息面板
    connect(m_treeView, &QTreeView::clicked,
            this, &MainWindow::onItemClicked);

    // 路径导航
    connect(goBtn, &QPushButton::clicked, this,
            &MainWindow::onNavigate);
    connect(m_pathInput, &QLineEdit::returnPressed, this,
            &MainWindow::onNavigate);

    // 展开控制
    connect(expandAllBtn, &QPushButton::clicked,
            m_treeView, &QTreeView::expandAll);
    connect(collapseAllBtn, &QPushButton::clicked,
            m_treeView, &QTreeView::collapseAll);

    connect(expandSelBtn, &QPushButton::clicked, this,
            [this]() {
        QModelIndex index = m_treeView->currentIndex();
        if (index.isValid()) {
            m_treeView->expand(index);
        }
    });

    connect(collapseSelBtn, &QPushButton::clicked, this,
            [this]() {
        QModelIndex index = m_treeView->currentIndex();
        if (index.isValid()) {
            m_treeView->collapse(index);
        }
    });

    // 展开/折叠信号
    connect(m_treeView, &QTreeView::expanded,
            this, [this](const QModelIndex &index) {
        QString name =
            index.data(Qt::DisplayRole).toString();
        m_statusLabel->setText(
            QString("已展开: %1").arg(name));
    });

    connect(m_treeView, &QTreeView::collapsed,
            this, [this](const QModelIndex &index) {
        QString name =
            index.data(Qt::DisplayRole).toString();
        m_statusLabel->setText(
            QString("已折叠: %1").arg(name));
    });
}

void MainWindow::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QString path = m_fsModel->filePath(index);
    QFileInfo info(path);

    QString details;
    details += QString("名称: %1\n").arg(info.fileName());
    details += QString("路径: %1\n").arg(info.absolutePath());

    if (info.isDir()) {
        details += "类型: 文件夹\n";
        QDir dir(path);
        int count =
            dir.entryList(QDir::NoDotAndDotDot
                          | QDir::AllEntries).size();
        details += QString("子项数: %1\n").arg(count);
    } else {
        details += "类型: 文件\n";
        if (info.size() < 1024) {
            details += QString("大小: %1 B\n")
                           .arg(info.size());
        } else if (info.size() < 1024 * 1024) {
            details += QString("大小: %1 KB\n")
                           .arg(info.size() / 1024.0, 0,
                                'f', 1);
        } else {
            details += QString("大小: %1 MB\n")
                           .arg(info.size() / (1024.0 * 1024.0),
                                0, 'f', 1);
        }
        details += QString("后缀: %1\n").arg(info.suffix());
    }

    details += QString("修改时间: %1")
                   .arg(info.lastModified()
                            .toString("yyyy-MM-dd HH:mm:ss"));

    m_infoLabel->setText(details);
}

void MainWindow::onNavigate()
{
    QString path = m_pathInput->text().trimmed();
    if (path.isEmpty()) return;

    QDir dir(path);
    if (!dir.exists()) {
        m_pathInput->setStyleSheet(
            "border: 1px solid #E53935;");
        m_statusLabel->setText(
            QString("路径不存在: %1").arg(path));
        return;
    }
    m_pathInput->setStyleSheet("");

    QModelIndex index = m_fsModel->index(path);
    if (index.isValid()) {
        m_treeView->setRootIndex(index);
        m_fsModel->setRootPath(path);
        m_statusLabel->setText(
            QString("已跳转: %1").arg(path));
    }
}
