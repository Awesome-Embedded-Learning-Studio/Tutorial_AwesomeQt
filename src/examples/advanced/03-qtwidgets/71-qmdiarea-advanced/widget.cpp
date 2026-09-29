/// @file    widget.cpp
/// @brief   MdiMainWindow 实现：QMdiArea 标签页模式与窗口菜单集成。
///
/// 对应教程：进阶层 03-QtWidgets/10-MDI 高级用法。
/// 演示 TabbedView / SubWindowView 切换、动态窗口菜单、级联/平铺。

#include "widget.h"

#include <QAction>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTextEdit>
#include <QWidget>

MdiMainWindow::MdiMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_mdiArea(nullptr)
    , m_windowMenu(nullptr)
    , m_windowCounter(0)
{
    setupMdiArea();
    setupMenus();

    setWindowTitle(tr("QMdiArea Advanced Demo"));
    resize(900, 600);
}

void MdiMainWindow::setupMdiArea()
{
    m_mdiArea = new QMdiArea(this);
    setCentralWidget(m_mdiArea);

    // @note 默认使用 TabbedView 模式，方便标签页切换演示
    m_mdiArea->setViewMode(QMdiArea::TabbedView);
    m_mdiArea->setTabsClosable(true);
    m_mdiArea->setTabsMovable(true);
}

void MdiMainWindow::setupMenus()
{
    // --- Window 菜单 ---
    m_windowMenu = menuBar()->addMenu(tr("&Window"));

    QAction* newAction = m_windowMenu->addAction(tr("&New Document"));
    connect(newAction, &QAction::triggered, this, &MdiMainWindow::createSubWindow);

    m_windowMenu->addSeparator();

    QAction* cascadeAction = m_windowMenu->addAction(tr("&Cascade"));
    connect(cascadeAction, &QAction::triggered, this, &MdiMainWindow::cascadeSubWindows);

    QAction* tileAction = m_windowMenu->addAction(tr("&Tile"));
    connect(tileAction, &QAction::triggered, this, &MdiMainWindow::tileSubWindows);

    m_windowMenu->addSeparator();

    QAction* toggleViewAction = m_windowMenu->addAction(tr("Switch to &MDI View"));
    connect(toggleViewAction, &QAction::triggered, this, &MdiMainWindow::toggleViewMode);

    m_windowMenu->addSeparator();

    // @note 子窗口列表占位区域，在 updateWindowMenu 中动态填充
    // 这里连接 subWindowListChanged 信号以保持菜单与实际子窗口同步
    connect(m_mdiArea, &QMdiArea::subWindowActivated,
            this, &MdiMainWindow::updateWindowMenu);
}

void MdiMainWindow::createSubWindow()
{
    ++m_windowCounter;

    auto* editor = new QTextEdit;
    editor->setPlaceholderText(
        tr("Document %1 — start typing here...").arg(m_windowCounter));

    // @note QMdiArea 接管 subWindow 的所有权，无需手动 delete
    QMdiSubWindow* sub = m_mdiArea->addSubWindow(editor);
    sub->setWindowTitle(tr("Document %1").arg(m_windowCounter));
    sub->show();

    updateWindowMenu();
}

void MdiMainWindow::cascadeSubWindows()
{
    // @note 级联排列前先切换到 SubWindowView，TabbedView 下级联无视觉效果
    m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    m_mdiArea->cascadeSubWindows();
}

void MdiMainWindow::tileSubWindows()
{
    // @note 平铺排列前切换到 SubWindowView，理由同 cascadeSubWindows
    m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    m_mdiArea->tileSubWindows();
}

void MdiMainWindow::toggleViewMode()
{
    if (m_mdiArea->viewMode() == QMdiArea::TabbedView) {
        m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    } else {
        m_mdiArea->setViewMode(QMdiArea::TabbedView);
    }
}

void MdiMainWindow::updateWindowMenu()
{
    // @note 先移除菜单尾部旧的子窗口条目（前 5 个是固定 Action + 2 个分隔符）
    // 固定项: New, sep, Cascade, Tile, sep, ToggleView, sep = 7 items
    const int fixedCount = 7;
    while (m_windowMenu->actions().size() > fixedCount) {
        QAction* last = m_windowMenu->actions().last();
        m_windowMenu->removeAction(last);
        // QAction 由 addAction 创建时无父对象，需要手动删除防止泄漏
        delete last;
    }

    // 动态添加当前所有子窗口到菜单
    const QList<QMdiSubWindow*> windows = m_mdiArea->subWindowList();
    for (QMdiSubWindow* sub : windows) {
        QAction* action = m_windowMenu->addAction(sub->windowTitle());
        // @note 捕获 sub 指针用于激活对应子窗口
        connect(action, &QAction::triggered, this, [this, sub]() {
            m_mdiArea->setActiveSubWindow(sub);
        });
    }
}
