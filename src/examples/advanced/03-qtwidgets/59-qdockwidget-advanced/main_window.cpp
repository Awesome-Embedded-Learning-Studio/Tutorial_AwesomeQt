/// @file    main_window.cpp
/// @brief   演示 QDockWidget 布局持久化的主窗口类实现。
///
/// 对应教程：进阶层 03-QtWidgets/59-QDockWidget 布局持久化。
/// 实现了 saveState()/restoreState() 配合 QSettings 的完整流程，
/// 以及菜单触发的布局重置功能。

#include "main_window.h"

#include <QCloseEvent>
#include <QDockWidget>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QStringList>
#include <QTableWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_fileTreeDock(nullptr)
    , m_propertiesDock(nullptr)
    , m_outputDock(nullptr)
    , m_fileTree(nullptr)
    , m_propertiesTable(nullptr)
    , m_outputLog(nullptr)
{
    setWindowTitle(tr("QDockWidget Layout Persistence Demo"));
    resize(1000, 700);

    // 中央控件：简单的占位文本编辑器
    auto* centralEdit = new QTextEdit(this);
    centralEdit->setPlaceholderText(
        tr("This is the central editor area.\n"
           "Drag dock panels around to rearrange the layout.\n"
           "Close and reopen the app to see layout restoration."));
    setCentralWidget(centralEdit);

    setupDockWidgets();
    setupMenus();

    // 尝试恢复上次保存的布局，若无保存数据则使用默认排列
    if (!restoreLayout())
    {
        resetLayout();
    }
}

MainWindow::~MainWindow()
{
    // 析构时也保存一次，确保布局始终被持久化
    saveLayout();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveLayout();
    event->accept();
}

void MainWindow::setupDockWidgets()
{
    // --- 文件树面板 ---
    m_fileTreeDock = new QDockWidget(tr("File Tree"), this);
    m_fileTree = new QTreeWidget(m_fileTreeDock);
    m_fileTree->setHeaderLabels({tr("Name"), tr("Size")});

    // 填充示例文件树数据
    auto* projectItem = new QTreeWidgetItem(m_fileTree, QStringList{"MyProject", "-"});
    new QTreeWidgetItem(projectItem, QStringList{"main.cpp", "2 KB"});
    new QTreeWidgetItem(projectItem, QStringList{"widget.h", "1 KB"});
    new QTreeWidgetItem(projectItem, QStringList{"widget.cpp", "3 KB"});
    auto* resItem = new QTreeWidgetItem(projectItem, QStringList{"resources", "-"});
    new QTreeWidgetItem(resItem, QStringList{"icon.png", "512 B"});
    m_fileTree->expandAll();
    m_fileTreeDock->setWidget(m_fileTree);

    // --- 属性表面板 ---
    m_propertiesDock = new QDockWidget(tr("Properties"), this);
    m_propertiesTable = new QTableWidget(4, 2, m_propertiesDock);
    m_propertiesTable->setHorizontalHeaderLabels({tr("Property"), tr("Value")});
    m_propertiesTable->horizontalHeader()->setStretchLastSection(true);

    // 填充示例属性数据
    QStringList properties = {"Width", "Height", "Background", "Opacity"};
    QStringList values = {"800", "600", "#FFFFFF", "1.0"};
    for (int i = 0; i < properties.size(); ++i)
    {
        m_propertiesTable->setItem(i, 0, new QTableWidgetItem(properties[i]));
        m_propertiesTable->setItem(i, 1, new QTableWidgetItem(values[i]));
    }
    m_propertiesDock->setWidget(m_propertiesTable);

    // --- 输出日志面板 ---
    m_outputDock = new QDockWidget(tr("Output"), this);
    m_outputLog = new QTextEdit(m_outputDock);
    m_outputLog->setReadOnly(true);
    m_outputLog->setPlainText(
        tr("Build started...\n"
           "Compiling main.cpp...\n"
           "Compiling widget.cpp...\n"
           "Linking...\n"
           "Build finished successfully."));
    m_outputDock->setWidget(m_outputLog);

    // 允许停靠到任意区域，支持嵌套（tab 化）
    for (auto* dock : {m_fileTreeDock, m_propertiesDock, m_outputDock})
    {
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        dock->setFeatures(QDockWidget::DockWidgetMovable
                          | QDockWidget::DockWidgetFloatable
                          | QDockWidget::DockWidgetClosable);
        addDockWidget(Qt::LeftDockWidgetArea, dock);
    }
}

void MainWindow::setupMenus()
{
    auto* viewMenu = menuBar()->addMenu(tr("&View"));

    // 切换各面板可见性的动作
    viewMenu->addAction(m_fileTreeDock->toggleViewAction());
    viewMenu->addAction(m_propertiesDock->toggleViewAction());
    viewMenu->addAction(m_outputDock->toggleViewAction());
    viewMenu->addSeparator();

    // 重置布局动作
    auto* resetAction = viewMenu->addAction(tr("Reset Layout"));
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetLayout);
}

void MainWindow::saveLayout()
{
    // @note 组织名和应用名用于构建 QSettings 的存储路径，
    //       不同平台会自动选择合适的后端（注册表/INI/defaults）。
    QSettings settings("AwesomeQt", "DockWidgetDemo");

    // saveGeometry 保存窗口位置和大小
    settings.setValue("geometry", saveGeometry());

    // saveState 保存所有 QDockWidget 的停靠位置、大小、可见性和 tab 顺序
    // @note 该方法只保存 QMainWindow 直接管理的 dock 和 toolbar，
    //       不包括手动管理的浮动窗口。
    settings.setValue("windowState", saveState());
}

bool MainWindow::restoreLayout()
{
    QSettings settings("AwesomeQt", "DockWidgetDemo");

    if (!settings.contains("geometry"))
    {
        return false;
    }

    // restoreGeometry 恢复窗口位置和大小，包括最大化/全屏状态
    restoreGeometry(settings.value("geometry").toByteArray());

    // restoreState 恢复所有 QDockWidget 的停靠状态
    // @note 必须在所有 QDockWidget 都已添加到 MainWindow 之后调用，
    //       否则状态数据找不到对应的 dock 对象。
    restoreState(settings.value("windowState").toByteArray());

    return true;
}

void MainWindow::resetLayout()
{
    // 先把所有面板设为可见，否则无法调整位置
    m_fileTreeDock->show();
    m_propertiesDock->show();
    m_outputDock->show();

    // 将面板从当前位置拆下，重新停靠到默认位置
    // @note addDockWidget 调用会自动将已存在的 dock 移到新位置
    addDockWidget(Qt::LeftDockWidgetArea, m_fileTreeDock);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);

    // 将属性面板 tab 化到文件树面板旁边（进阶用法）
    // tabifyDockWidget 会让两个 dock 共享同一区域，以标签页切换
    tabifyDockWidget(m_fileTreeDock, m_propertiesDock);
    m_fileTreeDock->raise();
}
