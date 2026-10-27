// QtWidgets 入门示例 10: QMdiArea 多文档界面基础
// 演示：QMdiArea 子窗口创建与管理
//       QMdiSubWindow 标题、图标与关闭行为
//       子窗口排列模式（级联 / 平铺）
//       激活子窗口与信号监听
//       简易多文档文本编辑器

#include "mdimainwindow.h"

#include <QApplication>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QTextEdit>
#include <QCloseEvent>
#include <QTimer>
#include <QDebug>

// ============================================================================
// MdiMainWindow: MDI 多文档编辑器主窗口
// ============================================================================
MdiMainWindow::MdiMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QMdiArea 多文档界面演示");
    resize(900, 600);

    // ---- MDI 区域 ----
    m_mdiArea = new QMdiArea(this);
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(m_mdiArea);

    // ---- 菜单栏 ----
    setupMenuBar();

    // ---- 工具栏 ----
    setupToolBar();

    // ---- 状态栏 ----
    m_statusDocLabel = new QLabel("没有打开的文档");
    m_statusDocLabel->setMinimumWidth(200);
    m_statusCountLabel = new QLabel("文档数: 0");
    m_statusCountLabel->setStyleSheet("padding: 0 12px;");
    statusBar()->addWidget(m_statusDocLabel, 1);
    statusBar()->addPermanentWidget(m_statusCountLabel);

    // ---- 信号连接 ----
    connect(m_mdiArea, &QMdiArea::subWindowActivated,
            this, &MdiMainWindow::onSubWindowActivated);

    // 初始状态：创建两个示例文档
    createNewDocument("欢迎文档",
        "欢迎使用 QMdiArea 多文档界面演示！\n\n"
        "你可以通过菜单或工具栏创建新的文档窗口。\n"
        "子窗口可以在 MDI 区域内自由拖拽和调整大小。\n"
        "试试\"级联排列\"和\"平铺排列\"按钮。");

    createNewDocument("说明",
        "这是第二个子窗口。\n\n"
        "MDI（Multiple Document Interface）允许多个子窗口\n"
        "存在于一个主窗口内部，适合需要同时查看多个\n"
        "文档或数据视图的场景。");

    // 初始级联排列
    QTimer::singleShot(100, m_mdiArea, &QMdiArea::cascadeSubWindows);
}

/// @brief 创建菜单栏
void MdiMainWindow::setupMenuBar()
{
    // ---- 文件菜单 ----
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");

    auto *newAction = fileMenu->addAction("新建文档(&N)");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered,
            this, [this]() { createNewDocument(); });

    fileMenu->addSeparator();

    auto *closeAction = fileMenu->addAction("关闭当前文档(&W)");
    closeAction->setShortcut(QKeySequence::Close);
    connect(closeAction, &QAction::triggered, this, [this]() {
        QMdiSubWindow *active = m_mdiArea->activeSubWindow();
        if (active) {
            active->close();
        }
    });

    auto *closeAllAction = fileMenu->addAction("关闭所有文档");
    connect(closeAllAction, &QAction::triggered, this, [this]() {
        m_mdiArea->closeAllSubWindows();
    });

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction("退出(&Q)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered,
            this, &QWidget::close);

    // ---- 窗口菜单 ----
    m_windowMenu = menuBar()->addMenu("窗口(&W)");
    connect(m_windowMenu, &QMenu::aboutToShow,
            this, &MdiMainWindow::updateWindowMenu);
}

/// @brief 创建工具栏
void MdiMainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("工具");
    toolBar->setMovable(false);

    auto *newBtn = toolBar->addAction("新建文档");
    connect(newBtn, &QAction::triggered,
            this, [this]() { createNewDocument(); });

    toolBar->addSeparator();

    auto *cascadeBtn = toolBar->addAction("级联排列");
    connect(cascadeBtn, &QAction::triggered,
            m_mdiArea, &QMdiArea::cascadeSubWindows);

    auto *tileBtn = toolBar->addAction("平铺排列");
    connect(tileBtn, &QAction::triggered,
            m_mdiArea, &QMdiArea::tileSubWindows);

    toolBar->addSeparator();

    auto *nextBtn = toolBar->addAction("下一个");
    connect(nextBtn, &QAction::triggered,
            m_mdiArea, &QMdiArea::activateNextSubWindow);

    auto *prevBtn = toolBar->addAction("上一个");
    connect(prevBtn, &QAction::triggered,
            m_mdiArea, &QMdiArea::activatePreviousSubWindow);

    toolBar->addSeparator();

    // 视图模式切换
    auto *tabModeBtn = toolBar->addAction("标签页模式");
    connect(tabModeBtn, &QAction::triggered, this, [this]() {
        m_mdiArea->setViewMode(QMdiArea::TabbedView);
    });

    auto *subWinModeBtn = toolBar->addAction("子窗口模式");
    connect(subWinModeBtn, &QAction::triggered, this, [this]() {
        m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    });
}

/// @brief 创建一个新的文档子窗口
/// @param title 子窗口标题（为空时自动生成）
/// @param text 编辑器初始文本
void MdiMainWindow::createNewDocument(const QString &title,
                                      const QString &text)
{
    ++m_docCounter;

    // 创建文本编辑器作为子窗口内容
    auto *editor = new QTextEdit();
    editor->setPlainText(text.isEmpty()
        ? QString("这是文档 %1 的内容。\n\n在这里编辑你的文本...").arg(m_docCounter)
        : text);
    editor->document()->setModified(false);

    // 添加到 MDI 区域
    auto *subWin = m_mdiArea->addSubWindow(editor);
    subWin->setWindowTitle(title.isEmpty()
        ? QString("文档 %1").arg(m_docCounter) : title);
    subWin->setAttribute(Qt::WA_DeleteOnClose);
    subWin->resize(450, 350);
    subWin->show();

    // 监听子窗口销毁，更新文档计数
    connect(subWin, &QMdiSubWindow::destroyed, this, [this]() {
        // 延迟更新，等 subWindowList 同步完成
        QTimer::singleShot(0, this, [this]() {
            updateStatusBar(nullptr);
        });
    });
}

/// @brief 子窗口激活时更新状态栏和窗口菜单
void MdiMainWindow::onSubWindowActivated(QMdiSubWindow *subWin)
{
    updateStatusBar(subWin);
}

/// @brief 更新状态栏信息
void MdiMainWindow::updateStatusBar(QMdiSubWindow *active)
{
    int count = m_mdiArea->subWindowList().size();
    m_statusCountLabel->setText("文档数: " + QString::number(count));

    if (!active) {
        active = m_mdiArea->activeSubWindow();
    }

    if (active) {
        m_statusDocLabel->setText("当前文档: " + active->windowTitle());
    } else if (count == 0) {
        m_statusDocLabel->setText("没有打开的文档");
    }
}

/// @brief 动态更新"窗口"菜单，列出所有子窗口
void MdiMainWindow::updateWindowMenu()
{
    m_windowMenu->clear();

    auto windows = m_mdiArea->subWindowList();
    QMdiSubWindow *active = m_mdiArea->activeSubWindow();

    for (auto *subWin : windows) {
        QString title = subWin->windowTitle();
        if (title.isEmpty()) {
            title = "无标题";
        }
        auto *action = m_windowMenu->addAction(title);
        action->setCheckable(true);
        action->setChecked(subWin == active);
        connect(action, &QAction::triggered, this, [this, subWin]() {
            m_mdiArea->setActiveSubWindow(subWin);
        });
    }

    if (windows.isEmpty()) {
        m_windowMenu->addAction("(无打开的文档)")->setEnabled(false);
    }
}

/// @brief 关闭主窗口前检查
void MdiMainWindow::closeEvent(QCloseEvent *event)
{
    m_mdiArea->closeAllSubWindows();
    if (m_mdiArea->subWindowList().isEmpty()) {
        event->accept();
    } else {
        event->ignore();
    }
}
