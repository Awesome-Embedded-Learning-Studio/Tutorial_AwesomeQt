// QtWidgets 入门示例 07: QMainWindow 主窗口体系基础
// 演示：QMenuBar / QMenu / QAction 菜单系统
//       QToolBar 工具栏按钮与分隔线
//       QStatusBar 状态栏（临时消息 + 永久组件）
//       QDockWidget 可停靠面板
//       简易文本编辑器

#include "mainwindow.h"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextCursor>
#include <QFont>
#include <QCloseEvent>
#include <QDebug>

// ============================================================================
// MainWindow: 简易文本编辑器
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("简易文本编辑器 — QMainWindow 演示");
    resize(900, 640);

    // ---- 中央控件 ----
    m_textEdit = new QTextEdit;
    m_textEdit->setFont(QFont("Monospace", 11));
    setCentralWidget(m_textEdit);

    // ---- 菜单栏 ----
    setupMenuBar();

    // ---- 工具栏 ----
    setupToolBar();

    // ---- 状态栏 ----
    setupStatusBar();

    // ---- 停靠面板 ----
    setupDockWidgets();

    // ---- 信号连接 ----
    connect(m_textEdit, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::updateCursorPosition);
    connect(m_textEdit, &QTextEdit::textChanged,
            this, &MainWindow::updateFileInfo);

    // 初始状态
    updateCursorPosition();
    updateFileInfo();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_textEdit->document()->isModified()) {
        auto result = QMessageBox::question(
            this, "未保存的更改",
            "当前文档有未保存的更改，确定要退出吗？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        if (result == QMessageBox::Save) {
            onSaveFile();
            event->accept();
        } else if (result == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

// ====================================================================
// 菜单栏构建
// ====================================================================
void MainWindow::setupMenuBar()
{
    // ---- 文件菜单 ----
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");

    m_newAction = fileMenu->addAction("新建(&N)");
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip("创建一个新文件");
    connect(m_newAction, &QAction::triggered,
            this, &MainWindow::onNewFile);

    m_openAction = fileMenu->addAction("打开(&O)");
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("打开已有文件");
    connect(m_openAction, &QAction::triggered,
            this, &MainWindow::onOpenFile);

    m_saveAction = fileMenu->addAction("保存(&S)");
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("保存当前文件");
    connect(m_saveAction, &QAction::triggered,
            this, &MainWindow::onSaveFile);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("退出(&Q)");
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("退出应用程序");
    connect(exitAction, &QAction::triggered,
            this, &QWidget::close);

    // ---- 编辑菜单 ----
    QMenu *editMenu = menuBar()->addMenu("编辑(&E)");

    m_copyAction = editMenu->addAction("复制(&C)");
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setStatusTip("复制选中文本");
    connect(m_copyAction, &QAction::triggered,
            m_textEdit, &QTextEdit::copy);

    m_pasteAction = editMenu->addAction("粘贴(&V)");
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setStatusTip("粘贴剪贴板内容");
    connect(m_pasteAction, &QAction::triggered,
            m_textEdit, &QTextEdit::paste);

    editMenu->addSeparator();

    QAction *selectAllAction = editMenu->addAction("全选(&A)");
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setStatusTip("选中全部文本");
    connect(selectAllAction, &QAction::triggered,
            m_textEdit, &QTextEdit::selectAll);

    // ---- 帮助菜单 ----
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    QAction *aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "关于",
            "QMainWindow 主窗口体系基础演示\n\n"
            "菜单栏 / 工具栏 / 状态栏 / 停靠面板");
    });
}

// ====================================================================
// 工具栏构建
// ====================================================================
void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(true);
    toolBar->setFloatable(true);
    toolBar->setIconSize(QSize(20, 20));

    // 复用菜单中的 QAction，菜单和工具栏共享同一个操作
    toolBar->addAction(m_newAction);
    toolBar->addAction(m_openAction);
    toolBar->addAction(m_saveAction);

    toolBar->addSeparator();

    // 在工具栏中嵌入自定义控件：字体大小选择
    auto *sizeLabel = new QLabel(" 字体大小: ");
    toolBar->addWidget(sizeLabel);

    auto *fontSizeCombo = new QComboBox;
    fontSizeCombo->addItems({"9", "10", "11", "12", "14", "16", "18", "20"});
    fontSizeCombo->setCurrentText("11");
    fontSizeCombo->setMaximumWidth(70);
    toolBar->addWidget(fontSizeCombo);

    connect(fontSizeCombo, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
        bool ok = false;
        int size = text.toInt(&ok);
        if (ok && size > 0) {
            QFont font = m_textEdit->font();
            font.setPointSize(size);
            m_textEdit->setFont(font);
        }
    });
}

// ====================================================================
// 状态栏构建
// ====================================================================
void MainWindow::setupStatusBar()
{
    // 左侧：光标位置（临时消息区域）
    m_positionLabel = new QLabel("行 1, 列 1");
    statusBar()->addWidget(m_positionLabel, 1);

    // 右侧：字符总数（永久组件）
    m_charCountLabel = new QLabel("字符数: 0");
    m_charCountLabel->setStyleSheet("padding: 0 8px;");
    statusBar()->addPermanentWidget(m_charCountLabel);

    // 显示初始提示消息，3 秒后消失
    statusBar()->showMessage("就绪 — 开始输入文本吧", 3000);
}

// ====================================================================
// 停靠面板构建
// ====================================================================
void MainWindow::setupDockWidgets()
{
    // ---- 左侧：文件浏览器面板 ----
    auto *fileDock = new QDockWidget("文件浏览器", this);
    fileDock->setAllowedAreas(Qt::LeftDockWidgetArea |
                               Qt::RightDockWidgetArea);

    auto *fileTree = new QTreeWidget;
    fileTree->setHeaderLabel("项目文件");
    auto *rootItem = new QTreeWidgetItem(fileTree, {"项目根目录"});
    new QTreeWidgetItem(rootItem, {"main.cpp"});
    new QTreeWidgetItem(rootItem, {"CMakeLists.txt"});
    new QTreeWidgetItem(rootItem, {"README.md"});
    new QTreeWidgetItem(rootItem, {"resources"});
    fileTree->expandAll();
    fileDock->setWidget(fileTree);

    addDockWidget(Qt::LeftDockWidgetArea, fileDock);

    // ---- 右侧：文件信息面板 ----
    m_fileInfoDock = new QDockWidget("文件信息", this);
    m_fileInfoDock->setAllowedAreas(Qt::LeftDockWidgetArea |
                                     Qt::RightDockWidgetArea);

    m_fileInfoTable = new QTableWidget;
    m_fileInfoTable->setColumnCount(2);
    m_fileInfoTable->setHorizontalHeaderLabels({"属性", "值"});
    m_fileInfoTable->horizontalHeader()->setStretchLastSection(true);
    m_fileInfoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_fileInfoTable->verticalHeader()->setVisible(false);

    // 初始信息行
    setFileInfoRow(0, "路径", "未保存");
    setFileInfoRow(1, "字符数", "0");
    setFileInfoRow(2, "行数", "1");
    setFileInfoRow(3, "最后修改", "-");

    m_fileInfoDock->setWidget(m_fileInfoTable);
    addDockWidget(Qt::RightDockWidgetArea, m_fileInfoDock);

    // ---- 视图菜单：控制面板显示/隐藏 ----
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    // toggleViewAction() 是 QDockWidget 自带的 QAction
    // 选中状态对应面板可见性，文本就是面板标题
    viewMenu->addAction(fileDock->toggleViewAction());
    viewMenu->addAction(m_fileInfoDock->toggleViewAction());
}

// ====================================================================
// 文件操作
// ====================================================================
void MainWindow::onNewFile()
{
    if (m_textEdit->document()->isModified()) {
        auto result = QMessageBox::question(
            this, "新建文件",
            "当前文档有未保存的更改，确定要新建吗？",
            QMessageBox::Yes | QMessageBox::No
        );
        if (result != QMessageBox::Yes) {
            return;
        }
    }
    m_textEdit->clear();
    m_currentFilePath.clear();
    setWindowTitle("简易文本编辑器 — 新文件");
    statusBar()->showMessage("新建文件", 2000);
}

void MainWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(
        this, "打开文件", QString(),
        "文本文件 (*.txt *.cpp *.h *.md);;所有文件 (*)"
    );
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "打开失败",
                             "无法打开文件: " + file.errorString());
        return;
    }

    QTextStream in(&file);
    m_textEdit->setPlainText(in.readAll());
    m_currentFilePath = path;

    QFileInfo info(path);
    setWindowTitle("简易文本编辑器 — " + info.fileName());
    statusBar()->showMessage("已打开: " + path, 3000);
    updateFileInfo();
}

void MainWindow::onSaveFile()
{
    QString path = m_currentFilePath;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(
            this, "保存文件", QString(),
            "文本文件 (*.txt);;所有文件 (*)"
        );
        if (path.isEmpty()) {
            return;
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "保存失败",
                             "无法保存文件: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << m_textEdit->toPlainText();
    m_currentFilePath = path;
    m_textEdit->document()->setModified(false);

    QFileInfo info(path);
    setWindowTitle("简易文本编辑器 — " + info.fileName());
    statusBar()->showMessage("已保存: " + path, 3000);
    updateFileInfo();
}

// ====================================================================
// 状态更新
// ====================================================================
void MainWindow::updateCursorPosition()
{
    QTextCursor cursor = m_textEdit->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock() + 1;
    m_positionLabel->setText(
        QString("行 %1, 列 %2").arg(line).arg(col));
}

void MainWindow::updateFileInfo()
{
    QString text = m_textEdit->toPlainText();
    int charCount = text.length();
    int lineCount = m_textEdit->document()->blockCount();

    m_charCountLabel->setText("字符数: " + QString::number(charCount));

    // 更新文件信息面板
    setFileInfoRow(0, "路径",
        m_currentFilePath.isEmpty() ? "未保存" : m_currentFilePath);
    setFileInfoRow(1, "字符数", QString::number(charCount));
    setFileInfoRow(2, "行数", QString::number(lineCount));

    if (!m_currentFilePath.isEmpty()) {
        QFileInfo info(m_currentFilePath);
        setFileInfoRow(3, "最后修改",
            info.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
    }
}

void MainWindow::setFileInfoRow(int row, const QString &property,
                                const QString &value)
{
    if (m_fileInfoTable->rowCount() <= row) {
        m_fileInfoTable->insertRow(row);
    }
    if (!m_fileInfoTable->item(row, 0)) {
        m_fileInfoTable->setItem(row, 0, new QTableWidgetItem);
        m_fileInfoTable->setItem(row, 1, new QTableWidgetItem);
    }
    m_fileInfoTable->item(row, 0)->setText(property);
    m_fileInfoTable->item(row, 1)->setText(value);
}
