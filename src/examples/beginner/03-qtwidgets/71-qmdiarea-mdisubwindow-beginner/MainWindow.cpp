// QtWidgets 入门示例 71: QMdiArea / QMdiSubWindow 多文档界面
// 演示：addSubWindow 添加子窗口
//       tileSubWindows / cascadeSubWindows 排列
//       subWindowActivated 信号追踪活动窗口
//       子窗口菜单项自动更新

#include "MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>

// ============================================================================
// MainWindow: MDI 多文档界面主窗口
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_documentCounter(0)
{
    setWindowTitle("MDI 多文档界面演示");
    resize(900, 650);

    // ---- MDI 区域 ----
    m_mdiArea = new QMdiArea;
    m_mdiArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    setCentralWidget(m_mdiArea);

    // 连接活动窗口信号
    connect(m_mdiArea,
            &QMdiArea::subWindowActivated,
            this,
            &MainWindow::onSubWindowActivated);

    // ---- 菜单栏 ----
    setupMenus();

    // ---- 工具栏 ----
    setupToolBar();

    // ---- 初始状态 ----
    updateStatusBar();
}

void MainWindow::setupMenus()
{
    // 文件菜单
    auto *fileMenu = menuBar()->addMenu("文件(&F)");

    fileMenu->addAction(
        "新建文档(&N)", QKeySequence::New, this,
        &MainWindow::onNewDocument);

    m_closeAction = fileMenu->addAction(
        "关闭当前(&W)", this,
        &MainWindow::onCloseCurrent);
    m_closeAction->setEnabled(false);

    fileMenu->addSeparator();

    fileMenu->addAction(
        "退出(&Q)", QKeySequence::Quit, this,
        &QWidget::close);

    // 编辑菜单
    auto *editMenu = menuBar()->addMenu("编辑(&E)");

    m_copyAction = editMenu->addAction(
        "复制(&C)", QKeySequence::Copy, this,
        &MainWindow::onCopy);
    m_copyAction->setEnabled(false);

    m_pasteAction = editMenu->addAction(
        "粘贴(&V)", QKeySequence::Paste, this,
        &MainWindow::onPaste);
    m_pasteAction->setEnabled(false);

    // 窗口菜单
    m_windowMenu =
        menuBar()->addMenu("窗口(&W)");
    connect(m_windowMenu, &QMenu::aboutToShow,
            this,
            &MainWindow::onUpdateWindowMenu);
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar("工具");
    toolbar->setMovable(false);

    toolbar->addAction(
        "新建", this,
        &MainWindow::onNewDocument);

    toolbar->addSeparator();

    toolbar->addAction(
        "层叠", m_mdiArea,
        &QMdiArea::cascadeSubWindows);
    toolbar->addAction(
        "平铺", m_mdiArea,
        &QMdiArea::tileSubWindows);

    toolbar->addSeparator();

    toolbar->addAction(
        "关闭所有", m_mdiArea,
        &QMdiArea::closeAllSubWindows);
}

// ====================================================================
// 新建文档
// ====================================================================
void MainWindow::onNewDocument()
{
    ++m_documentCounter;

    auto *editor = new QTextEdit;
    editor->setPlaceholderText(
        QString("文档 %1 的编辑区域...")
            .arg(m_documentCounter));

    auto *subWindow =
        m_mdiArea->addSubWindow(editor);
    subWindow->setWindowTitle(
        QString("未命名 %1")
            .arg(m_documentCounter));
    subWindow->resize(400, 300);
    subWindow->show();

    m_mdiArea->setActiveSubWindow(subWindow);
}

// ====================================================================
// 关闭当前活动子窗口
// ====================================================================
void MainWindow::onCloseCurrent()
{
    auto *active = m_mdiArea->activeSubWindow();
    if (active) {
        active->close();
    }
}

// ====================================================================
// 复制 / 粘贴（作用于活动子窗口中的 QTextEdit）
// ====================================================================
void MainWindow::onCopy()
{
    auto *editor = activeTextEdit();
    if (editor) {
        editor->copy();
    }
}

void MainWindow::onPaste()
{
    auto *editor = activeTextEdit();
    if (editor) {
        editor->paste();
    }
}

// ====================================================================
// 活动子窗口切换
// ====================================================================
void MainWindow::onSubWindowActivated(QMdiSubWindow *subWindow)
{
    const bool hasActive = (subWindow != nullptr);
    m_closeAction->setEnabled(hasActive);
    m_copyAction->setEnabled(hasActive);
    m_pasteAction->setEnabled(hasActive);

    updateStatusBar();
}

// ====================================================================
// 动态构建窗口菜单
// ====================================================================
void MainWindow::onUpdateWindowMenu()
{
    m_windowMenu->clear();

    // 固定操作项
    m_windowMenu->addAction(
        "新建文档(&N)", this,
        &MainWindow::onNewDocument);

    m_windowMenu->addSeparator();

    m_windowMenu->addAction(
        "层叠排列(&C)", m_mdiArea,
        &QMdiArea::cascadeSubWindows);
    m_windowMenu->addAction(
        "平铺排列(&T)", m_mdiArea,
        &QMdiArea::tileSubWindows);

    m_windowMenu->addSeparator();

    m_windowMenu->addAction(
        "关闭当前", this,
        &MainWindow::onCloseCurrent);
    m_windowMenu->addAction(
        "关闭所有", m_mdiArea,
        &QMdiArea::closeAllSubWindows);

    // 子窗口列表
    const auto subWindows =
        m_mdiArea->subWindowList();

    if (!subWindows.isEmpty()) {
        m_windowMenu->addSeparator();

        const auto *active =
            m_mdiArea->activeSubWindow();

        // 数字快捷键最多 9 个
        const int count =
            qMin(subWindows.size(), 9);

        for (int i = 0; i < count; ++i) {
            auto *sub = subWindows.at(i);
            QString text =
                QString("&%1 %2")
                    .arg(i + 1)
                    .arg(sub->windowTitle());

            auto *action =
                m_windowMenu->addAction(text);
            action->setCheckable(true);
            action->setChecked(sub == active);

            connect(action,
                    &QAction::triggered,
                    this,
                    [this, sub]() {
                m_mdiArea->setActiveSubWindow(
                    sub);
            });
        }

        // 超过 9 个的窗口不加速键
        for (int i = 9;
             i < subWindows.size(); ++i) {
            auto *sub = subWindows.at(i);
            auto *action =
                m_windowMenu->addAction(
                    sub->windowTitle());
            action->setCheckable(true);
            action->setChecked(sub == active);

            connect(action,
                    &QAction::triggered,
                    this,
                    [this, sub]() {
                m_mdiArea->setActiveSubWindow(
                    sub);
            });
        }
    }
}

// ====================================================================
// 更新状态栏
// ====================================================================
void MainWindow::updateStatusBar()
{
    const auto subWindows =
        m_mdiArea->subWindowList();
    const auto *active =
        m_mdiArea->activeSubWindow();

    QString msg;
    if (active) {
        msg = QString("活动文档: %1 | ")
                  .arg(active->windowTitle());
    }
    msg += QString("共 %1 个文档")
               .arg(subWindows.size());

    statusBar()->showMessage(msg);
}

/// @brief 获取活动子窗口中的 QTextEdit
QTextEdit *MainWindow::activeTextEdit() const
{
    auto *sub = m_mdiArea->activeSubWindow();
    if (!sub) {
        return nullptr;
    }
    return qobject_cast<QTextEdit *>(
        sub->widget());
}
