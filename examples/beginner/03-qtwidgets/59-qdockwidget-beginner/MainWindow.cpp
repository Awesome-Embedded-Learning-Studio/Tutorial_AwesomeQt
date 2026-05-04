#include "MainWindow.h"

// ============================================================================
// MainWindow: 模拟简易 IDE 布局
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QDockWidget 可停靠面板演示");
    resize(1000, 700);

    // ---- 中央控件 ----
    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText("// 在这里编写代码...");
    setCentralWidget(m_editor);

    // ---- 停靠面板 ----
    setupDockWidgets();

    // ---- 菜单栏 ----
    setupMenuBar();

    // ---- 状态栏初始消息 ----
    statusBar()->showMessage("就绪 — 拖拽面板试试", 3000);
}

// ====================================================================
// 创建 Dock 面板的通用方法
// ====================================================================
QDockWidget* MainWindow::createDock(const QString &title,
                             QWidget *widget,
                             Qt::DockWidgetArea area,
                             Qt::DockWidgetAreas allowedAreas)
{
    auto *dock = new QDockWidget(title, this);
    // 设置 objectName 供 restoreState 匹配
    dock->setObjectName(title);
    dock->setWidget(widget);
    dock->setAllowedAreas(allowedAreas);

    // 所有面板不允许关闭（隐藏关闭按钮），
    // 但可以通过视图菜单 toggleViewAction 控制
    dock->setFeatures(QDockWidget::DockWidgetMovable |
                       QDockWidget::DockWidgetFloatable);

    addDockWidget(area, dock);
    return dock;
}

// ====================================================================
// 停靠面板布局
// ====================================================================
void MainWindow::setupDockWidgets()
{
    // ---- 左侧面板组 ----

    // 文件浏览器
    auto *fileTree = new QTreeWidget;
    fileTree->setHeaderLabel("项目文件");
    populateFileTree(fileTree);

    m_fileDock = createDock("文件浏览器", fileTree,
                             Qt::LeftDockWidgetArea,
                             Qt::LeftDockWidgetArea |
                             Qt::RightDockWidgetArea);

    // 符号列表
    auto *symbolList = new QListWidget;
    symbolList->addItems({"MainWindow", "setupUI()", "onSave()",
                           "onLoad()", "m_editor", "m_fileDock"});
    m_symbolDock = createDock("符号列表", symbolList,
                               Qt::LeftDockWidgetArea,
                               Qt::LeftDockWidgetArea |
                               Qt::RightDockWidgetArea);

    // 书签
    auto *bookmarkList = new QListWidget;
    bookmarkList->addItems({"第 12 行: 初始化",
                             "第 45 行: 事件处理",
                             "第 89 行: 文件操作"});
    m_bookmarkDock = createDock("书签", bookmarkList,
                                 Qt::LeftDockWidgetArea,
                                 Qt::LeftDockWidgetArea |
                                 Qt::RightDockWidgetArea);

    // 标签化合并左侧三个面板
    tabifyDockWidget(m_fileDock, m_symbolDock);
    tabifyDockWidget(m_fileDock, m_bookmarkDock);
    m_fileDock->raise();  // 默认显示文件浏览器

    // ---- 底部面板组 ----

    // 编译输出
    auto *outputEdit = new QTextEdit;
    outputEdit->setReadOnly(true);
    outputEdit->setPlainText(
        "构建 started...\n"
        "编译 main.cpp...\n"
        "链接 project...\n"
        "构建成功（0 错误, 0 警告）");
    m_outputDock = createDock("编译输出", outputEdit,
                               Qt::BottomDockWidgetArea,
                               Qt::BottomDockWidgetArea);

    // 问题列表
    auto *problemList = new QListWidget;
    problemList->addItems({"无问题"});
    m_problemDock = createDock("问题列表", problemList,
                                Qt::BottomDockWidgetArea,
                                Qt::BottomDockWidgetArea);

    // 标签化合并底部两个面板
    tabifyDockWidget(m_outputDock, m_problemDock);
    m_outputDock->raise();  // 默认显示编译输出

    // ---- 信号监听 ----
    connectDockSignals(m_fileDock);
    connectDockSignals(m_symbolDock);
    connectDockSignals(m_bookmarkDock);
    connectDockSignals(m_outputDock);
    connectDockSignals(m_problemDock);
}

// ====================================================================
// 连接 Dock 状态信号
// ====================================================================
void MainWindow::connectDockSignals(QDockWidget *dock)
{
    // 浮动状态变化
    connect(dock, &QDockWidget::topLevelChanged,
            this, [this, dock](bool topLevel) {
        if (topLevel) {
            statusBar()->showMessage(
                dock->windowTitle() + " 已浮动", 2000);
        } else {
            statusBar()->showMessage(
                dock->windowTitle() + " 已停靠", 2000);
        }
    });

    // 停靠位置变化
    connect(dock, &QDockWidget::dockLocationChanged,
            this, [this, dock](Qt::DockWidgetArea area) {
        QString position;
        switch (area) {
            case Qt::LeftDockWidgetArea:
                position = "左侧"; break;
            case Qt::RightDockWidgetArea:
                position = "右侧"; break;
            case Qt::TopDockWidgetArea:
                position = "顶部"; break;
            case Qt::BottomDockWidgetArea:
                position = "底部"; break;
            default:
                position = "浮动"; break;
        }
        statusBar()->showMessage(
            dock->windowTitle() + " → " + position, 2000);
    });
}

// ====================================================================
// 菜单栏
// ====================================================================
void MainWindow::setupMenuBar()
{
    auto *viewMenu = menuBar()->addMenu("视图(&V)");
    viewMenu->addAction(m_fileDock->toggleViewAction());
    viewMenu->addAction(m_symbolDock->toggleViewAction());
    viewMenu->addAction(m_bookmarkDock->toggleViewAction());
    viewMenu->addSeparator();
    viewMenu->addAction(m_outputDock->toggleViewAction());
    viewMenu->addAction(m_problemDock->toggleViewAction());
}

// ====================================================================
// 填充模拟文件树
// ====================================================================
void MainWindow::populateFileTree(QTreeWidget *tree)
{
    auto *root = new QTreeWidgetItem(tree, {"MyProject"});
    auto *src = new QTreeWidgetItem(root, {"src"});
    new QTreeWidgetItem(src, {"main.cpp"});
    new QTreeWidgetItem(src, {"widget.cpp"});
    new QTreeWidgetItem(src, {"widget.h"});

    auto *res = new QTreeWidgetItem(root, {"resources"});
    new QTreeWidgetItem(res, {"icon.png"});
    new QTreeWidgetItem(res, {"style.qss"});

    new QTreeWidgetItem(root, {"CMakeLists.txt"});
    new QTreeWidgetItem(root, {"README.md"});

    tree->expandAll();
}
