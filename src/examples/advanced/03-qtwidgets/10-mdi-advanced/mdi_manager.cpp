/// @file    mdi_manager.cpp
/// @brief   MdiManager 类实现——MDI 子窗口管理主窗口。
///
/// 对应教程：进阶层 03-QtWidgets/10-MDI 进阶。

#include "mdi_manager.h"
#include "mdi_child.h"

#include <QLabel>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

MdiManager::MdiManager(QWidget* parent)
    : QMainWindow(parent)
    , m_mdiArea(nullptr)
    , m_windowMenu(nullptr)
    , m_statusLabel(nullptr)
    , m_childCounter(0)
{
    // 创建 MDI 区域作为中央控件
    m_mdiArea = new QMdiArea;
    setCentralWidget(m_mdiArea);

    // ── 菜单栏 ──
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("文件(&F)"));

    auto* newAction = fileMenu->addAction(QStringLiteral("新建(&N)"));
    newAction->setShortcut(QStringLiteral("Ctrl+N"));
    connect(newAction, &QAction::triggered, this, &MdiManager::createChild);

    fileMenu->addSeparator();

    auto* closeAllAction = fileMenu->addAction(QStringLiteral("关闭所有(&W)"));
    connect(closeAllAction, &QAction::triggered, m_mdiArea, &QMdiArea::closeAllSubWindows);

    // 窗口菜单
    m_windowMenu = menuBar()->addMenu(QStringLiteral("窗口(&W)"));

    auto* cascadeAction = m_windowMenu->addAction(QStringLiteral("级联排列(&C)"));
    connect(cascadeAction, &QAction::triggered, this, &MdiManager::cascadeSubWindows);

    auto* tileAction = m_windowMenu->addAction(QStringLiteral("平铺排列(&T)"));
    connect(tileAction, &QAction::triggered, this, &MdiManager::tileSubWindows);

    m_windowMenu->addSeparator();

    // 状态栏
    m_statusLabel = new QLabel(QStringLiteral("无打开文档"));
    statusBar()->addWidget(m_statusLabel);

    // 监听子窗口激活事件
    connect(m_mdiArea, &QMdiArea::subWindowActivated, this, &MdiManager::onSubWindowActivated);

    setWindowTitle(QStringLiteral("MDI Advanced Demo"));
    resize(800, 600);
}

// ─────────────────────────────────────────────────────────────────────────────
// 子窗口操作
// ─────────────────────────────────────────────────────────────────────────────

void MdiManager::createChild()
{
    ++m_childCounter;

    auto* child = new MdiChild;
    child->setDocumentTitle(QStringLiteral("文档 %1").arg(m_childCounter));

    // 将 MdiChild 包装为 MDI 子窗口添加到 MDI 区域
    auto* subWin = m_mdiArea->addSubWindow(child);
    subWin->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动销毁子窗口
    subWin->show();
}

void MdiManager::cascadeSubWindows()
{
    m_mdiArea->cascadeSubWindows();
}

void MdiManager::tileSubWindows()
{
    m_mdiArea->tileSubWindows();
}

// ─────────────────────────────────────────────────────────────────────────────
// 子窗口激活处理
// ─────────────────────────────────────────────────────────────────────────────

void MdiManager::onSubWindowActivated(QMdiSubWindow* subWin)
{
    // 子窗口全部关闭后 subWin 为 nullptr
    if (!subWin) {
        m_statusLabel->setText(QStringLiteral("无打开文档"));
        rebuildWindowMenu();
        return;
    }

    // 获取子窗口内的 MdiChild 控件
    auto* child = qobject_cast<MdiChild*>(subWin->widget());
    if (child) {
        const QString title = subWin->windowTitle().isEmpty()
                                  ? QStringLiteral("无标题")
                                  : subWin->windowTitle();
        m_statusLabel->setText(
            QStringLiteral("活动文档: %1 | 打开文档数: %2")
                .arg(title)
                .arg(m_mdiArea->subWindowList().size()));
    }

    rebuildWindowMenu();
}

// ─────────────────────────────────────────────────────────────────────────────
// 窗口菜单重建
// ─────────────────────────────────────────────────────────────────────────────

void MdiManager::rebuildWindowMenu()
{
    // 移除分隔符之后的所有动态菜单项
    const QList<QAction*> actions = m_windowMenu->actions();
    // 前三项是固定的：级联、平铺、分隔符，后面的都是动态窗口列表
    bool foundSeparator = false;
    for (auto* action : actions) {
        if (action->isSeparator()) {
            foundSeparator = true;
            continue;
        }
        if (foundSeparator) {
            m_windowMenu->removeAction(action);
        }
    }

    QMdiSubWindow* activeWin = m_mdiArea->activeSubWindow();
    const QList<QMdiSubWindow*> subWindows = m_mdiArea->subWindowList();

    for (auto* subWin : subWindows) {
        const QString title = subWin->windowTitle().isEmpty()
                                  ? QStringLiteral("无标题")
                                  : subWin->windowTitle();

        auto* action = m_windowMenu->addAction(title);
        action->setCheckable(true);
        action->setChecked(subWin == activeWin);

        // 点击菜单项切换到对应子窗口
        connect(action, &QAction::triggered, this, [this, subWin]() {
            m_mdiArea->setActiveSubWindow(subWin);
        });
    }
}
