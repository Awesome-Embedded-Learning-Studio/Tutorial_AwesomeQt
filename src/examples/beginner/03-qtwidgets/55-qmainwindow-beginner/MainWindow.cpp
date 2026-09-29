#include "MainWindow.h"

#include <QApplication>
#include <QKeySequence>
#include <QMenuBar>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// MainWindow: 完整的主窗口配置
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QMainWindow 综合演示 — 文本编辑器");
    resize(1024, 640);

    initCentralWidget();
    initMenuBar();
    initToolBars();
    initStatusBar();
    initDockWidgets();

    // 恢复上次保存的窗口状态
    restoreWindowState();

    // 连接编辑器光标变化信号以更新状态栏
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateCursorPosition);
}

/// @brief 关闭事件：保存窗口状态
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowState();
    event->accept();
}

/// @brief 初始化中央控件
void MainWindow::initCentralWidget()
{
    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText(
        "在这里输入文本...\n\n"
        "本示例演示 QMainWindow 的完整配置：\n"
        "- 中央编辑器 (QPlainTextEdit)\n"
        "- 菜单栏 (文件、编辑、视图)\n"
        "- 工具栏 (文件、编辑)\n"
        "- 状态栏 (临时消息 + 光标位置)\n"
        "- Dock 窗口 (大纲、输出、属性)\n"
        "- 窗口状态持久化 (QSettings)\n");
    setCentralWidget(m_editor);
}

/// @brief 初始化菜单栏
void MainWindow::initMenuBar()
{
    // ================================================================
    // 文件菜单
    // ================================================================
    auto *fileMenu = menuBar()->addMenu("文件(&F)");

    m_newAction = fileMenu->addAction(
        style()->standardIcon(QStyle::SP_FileIcon),
        "新建(&N)");
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip("创建新文档");

    m_openAction = fileMenu->addAction(
        style()->standardIcon(QStyle::SP_DirOpenIcon),
        "打开(&O)");
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("打开已有文档");

    m_saveAction = fileMenu->addAction(
        style()->standardIcon(QStyle::SP_DialogSaveButton),
        "保存(&S)");
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("保存当前文档");

    fileMenu->addSeparator();

    auto *quitAction = fileMenu->addAction("退出(&Q)");
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setStatusTip("退出应用程序");
    connect(quitAction, &QAction::triggered,
            this, &QMainWindow::close);

    // ================================================================
    // 编辑菜单
    // ================================================================
    auto *editMenu = menuBar()->addMenu("编辑(&E)");

    auto *undoAction = editMenu->addAction("撤销(&U)");
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::undo);

    auto *redoAction = editMenu->addAction("重做(&R)");
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::redo);

    editMenu->addSeparator();

    auto *cutAction = editMenu->addAction("剪切(&T)");
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::cut);

    auto *copyAction = editMenu->addAction("复制(&C)");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::copy);

    auto *pasteAction = editMenu->addAction("粘贴(&P)");
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::paste);

    editMenu->addSeparator();

    auto *selectAllAction = editMenu->addAction("全选(&A)");
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::selectAll);

    // ================================================================
    // 视图菜单
    // ================================================================
    auto *viewMenu = menuBar()->addMenu("视图(&V)");

    // 稍后在 initDockWidgets 中添加 Dock 切换 Action
    m_viewMenu = viewMenu;

    viewMenu->addSeparator();

    auto *resetAction = viewMenu->addAction("恢复默认布局");
    connect(resetAction, &QAction::triggered,
            this, &MainWindow::resetLayout);

    // ================================================================
    // 帮助菜单
    // ================================================================
    auto *helpMenu = menuBar()->addMenu("帮助(&H)");
    auto *aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, [this]() {
        statusBar()->showMessage(
            "QMainWindow 综合演示 v1.0 — Qt 6.9.1", 3000);
    });
}

/// @brief 初始化工具栏
void MainWindow::initToolBars()
{
    // 文件工具栏
    m_fileToolBar = addToolBar("文件");
    m_fileToolBar->setObjectName("fileToolBar");
    m_fileToolBar->addAction(m_newAction);
    m_fileToolBar->addAction(m_openAction);
    m_fileToolBar->addAction(m_saveAction);

    // 编辑工具栏
    m_editToolBar = addToolBar("编辑");
    m_editToolBar->setObjectName("editToolBar");

    auto *undoBtn = m_editToolBar->addAction(
        style()->standardIcon(QStyle::SP_ArrowBack),
        "撤销");
    connect(undoBtn, &QAction::triggered,
            m_editor, &QPlainTextEdit::undo);

    auto *redoBtn = m_editToolBar->addAction(
        style()->standardIcon(QStyle::SP_ArrowForward),
        "重做");
    connect(redoBtn, &QAction::triggered,
            m_editor, &QPlainTextEdit::redo);
}

/// @brief 初始化状态栏
void MainWindow::initStatusBar()
{
    // 临时消息（3 秒后消失）
    statusBar()->showMessage("就绪", 3000);

    // 永久控件：光标位置
    m_positionLabel = new QLabel("行 1, 列 1");
    m_positionLabel->setMinimumWidth(120);
    m_positionLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addPermanentWidget(m_positionLabel);

    // 永久控件：字符统计
    m_charCountLabel = new QLabel("字符: 0");
    m_charCountLabel->setMinimumWidth(100);
    m_charCountLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addPermanentWidget(m_charCountLabel);

    connect(m_editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateCharCount);
}

/// @brief 初始化 Dock 窗口
void MainWindow::initDockWidgets()
{
    // ================================================================
    // 大纲面板（左侧）
    // ================================================================
    m_outlineDock = new QDockWidget("大纲", this);
    m_outlineDock->setObjectName("outlineDock");
    m_outlineDock->setAllowedAreas(
        Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *outlineTree = new QTreeView;
    outlineTree->setHeaderHidden(true);
    m_outlineDock->setWidget(outlineTree);

    addDockWidget(Qt::LeftDockWidgetArea, m_outlineDock);

    // ================================================================
    // 输出面板（底部）
    // ================================================================
    m_outputDock = new QDockWidget("输出", this);
    m_outputDock->setObjectName("outputDock");
    m_outputDock->setAllowedAreas(
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

    auto *outputEdit = new QPlainTextEdit;
    outputEdit->setReadOnly(true);
    outputEdit->setMaximumHeight(150);
    outputEdit->setPlainText(
        "应用程序启动完成。\n"
        "窗口布局已从 QSettings 恢复。");
    m_outputDock->setWidget(outputEdit);

    addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);

    // ================================================================
    // 属性面板（右侧）
    // ================================================================
    m_propertiesDock = new QDockWidget("属性", this);
    m_propertiesDock->setObjectName("propertiesDock");
    m_propertiesDock->setAllowedAreas(
        Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *propsWidget = new QWidget;
    auto *propsLayout = new QVBoxLayout(propsWidget);

    auto *fontLabel = new QLabel(
        "字体: Monospace\n大小: 14pt\n编码: UTF-8");
    fontLabel->setStyleSheet(
        "padding: 8px; color: #555;");
    propsLayout->addWidget(fontLabel);
    propsLayout->addStretch();

    m_propertiesDock->setWidget(propsWidget);

    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

    // 把属性面板标签化到大纲面板旁边
    tabifyDockWidget(m_outlineDock, m_propertiesDock);
    m_outlineDock->raise();

    // ================================================================
    // 视图菜单：添加 Dock 切换 Action
    // ================================================================
    if (m_viewMenu) {
        m_viewMenu->addSeparator();
        m_viewMenu->addAction(
            m_outlineDock->toggleViewAction());
        m_viewMenu->addAction(
            m_propertiesDock->toggleViewAction());
        m_viewMenu->addAction(
            m_outputDock->toggleViewAction());
    }
}

/// @brief 保存窗口状态到 QSettings
void MainWindow::saveWindowState()
{
    QSettings settings("AwesomeQt", "QMainWindowDemo");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

/// @brief 从 QSettings 恢复窗口状态
void MainWindow::restoreWindowState()
{
    QSettings settings("AwesomeQt", "QMainWindowDemo");
    if (settings.contains("geometry")) {
        restoreGeometry(
            settings.value("geometry").toByteArray());
    }
    if (settings.contains("windowState")) {
        restoreState(
            settings.value("windowState").toByteArray());
    }
}

/// @brief 恢复默认布局
void MainWindow::resetLayout()
{
    // 先移除所有 Dock，再按默认位置重新添加
    removeDockWidget(m_outlineDock);
    removeDockWidget(m_outputDock);
    removeDockWidget(m_propertiesDock);

    addDockWidget(Qt::LeftDockWidgetArea, m_outlineDock);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    tabifyDockWidget(m_outlineDock, m_propertiesDock);
    m_outlineDock->raise();
    addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);

    // 显示所有 Dock
    m_outlineDock->show();
    m_propertiesDock->show();
    m_outputDock->show();

    // 重置窗口大小
    resize(1024, 640);

    statusBar()->showMessage("已恢复默认布局", 2000);
}

/// @brief 更新状态栏中的光标位置
void MainWindow::updateCursorPosition()
{
    QTextCursor cursor = m_editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;
    m_positionLabel->setText(
        QString("行 %1, 列 %2").arg(line).arg(column));
}

/// @brief 更新字符统计
void MainWindow::updateCharCount()
{
    int count = m_editor->toPlainText().length();
    m_charCountLabel->setText(
        QString("字符: %1").arg(count));
}
