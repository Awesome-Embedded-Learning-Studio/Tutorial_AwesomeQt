/// @file    persistent_main_window.cpp
/// @brief   PersistentMainWindow 实现——Dock 管理、布局持久化与 setCorner 演示。
///
/// 对应教程：进阶层 03-QtWidgets/07-主窗口进阶。

#include "persistent_main_window.h"

#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>

// ─── 常量 ───────────────────────────────────────────────────────────────────────

/// 布局版本号，Dock 结构变化时递增以区分不兼容的旧数据。
static constexpr int kStateVersion = 1;

/// QSettings 中保存窗口几何信息的 key。
static constexpr auto kGeometryKey = "main_window/geometry";

/// QSettings 中保存 Dock/工具栏布局的 key。
static constexpr auto kStateKey = "main_window/state";

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

PersistentMainWindow::PersistentMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // 中央编辑区
    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText(
        QStringLiteral("代码编辑区域——拖拽周围的 Dock 面板来调整布局，"
                       "关闭后重新打开即可看到布局被完整恢复。"));
    setCentralWidget(m_editor);

    // setCorner 必须在 addDockWidget 之前调用
    setupCorners();
    setupDockWidgets();
    setupToolBars();
    setupMenuBar();

    setWindowTitle(QStringLiteral("QMainWindow Layout Persistence Demo"));
    resize(1100, 700);
    statusBar()->showMessage(QStringLiteral("就绪 — 拖拽 Dock 面板调整布局，关闭窗口后布局自动保存"));
}

// ─────────────────────────────────────────────────────────────────────────────
// setCorner——角落区域归属
// ─────────────────────────────────────────────────────────────────────────────

void PersistentMainWindow::setupCorners()
{
    // 左侧 Dock 占据左上角和左下角，让左侧面板延伸到窗口顶部和底部
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    // 右侧 Dock 占据右上角和右下角
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

// ─────────────────────────────────────────────────────────────────────────────
// Dock 面板创建
// ─────────────────────────────────────────────────────────────────────────────

void PersistentMainWindow::setupDockWidgets()
{
    // 左侧文件浏览器——objectName 是 restoreState 识别 Dock 的唯一标识
    m_fileExplorerDock = new QDockWidget(QStringLiteral("文件浏览器"), this);
    m_fileExplorerDock->setObjectName(QStringLiteral("fileExplorer"));
    m_fileExplorerDock->setWidget(new QPlainTextEdit);
    addDockWidget(Qt::LeftDockWidgetArea, m_fileExplorerDock);

    // 右侧属性面板
    m_propertyDock = new QDockWidget(QStringLiteral("属性面板"), this);
    m_propertyDock->setObjectName(QStringLiteral("propertyEditor"));
    m_propertyDock->setWidget(new QPlainTextEdit);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);

    // 右侧工具箱——和属性面板同区域，tab 化
    m_toolBoxDock = new QDockWidget(QStringLiteral("工具箱"), this);
    m_toolBoxDock->setObjectName(QStringLiteral("toolBox"));
    m_toolBoxDock->setWidget(new QPlainTextEdit);
    addDockWidget(Qt::RightDockWidgetArea, m_toolBoxDock);

    // 演示 tabifyDockWidget：让工具箱和属性面板以 tab 形式堆叠
    tabifyDockWidget(m_propertyDock, m_toolBoxDock);
    // 默认激活属性面板 tab
    m_propertyDock->raise();

    // 底部编译输出
    m_outputDock = new QDockWidget(QStringLiteral("编译输出"), this);
    m_outputDock->setObjectName(QStringLiteral("outputPanel"));
    auto* outputEdit = new QPlainTextEdit;
    outputEdit->setReadOnly(true);
    outputEdit->setPlaceholderText(QStringLiteral("编译信息将显示在这里..."));
    m_outputDock->setWidget(outputEdit);
    addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);
}

// ─────────────────────────────────────────────────────────────────────────────
// 工具栏
// ─────────────────────────────────────────────────────────────────────────────

void PersistentMainWindow::setupToolBars()
{
    auto* fileToolBar = addToolBar(QStringLiteral("文件"));
    fileToolBar->setObjectName(QStringLiteral("fileToolBar"));
    fileToolBar->addAction(QStringLiteral("新建"), this, [this]() {
        m_editor->clear();
        statusBar()->showMessage(QStringLiteral("新建文件"), 2000);
    });
    fileToolBar->addAction(QStringLiteral("打开"), this, [this]() {
        statusBar()->showMessage(QStringLiteral("打开文件（演示用，无实际操作）"), 2000);
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// 菜单栏
// ─────────────────────────────────────────────────────────────────────────────

void PersistentMainWindow::setupMenuBar()
{
    // 文件菜单
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("文件(&F)"));
    fileMenu->addAction(QStringLiteral("退出(&Q)"), this, &QWidget::close);

    // 视图菜单——集成 toggleViewAction，自动同步 Dock 显示/隐藏状态
    auto* viewMenu = menuBar()->addMenu(QStringLiteral("视图(&V)"));
    viewMenu->addAction(m_fileExplorerDock->toggleViewAction());
    viewMenu->addAction(m_propertyDock->toggleViewAction());
    viewMenu->addAction(m_toolBoxDock->toggleViewAction());
    viewMenu->addAction(m_outputDock->toggleViewAction());
    viewMenu->addSeparator();
    viewMenu->addAction(QStringLiteral("重置为默认布局(&R)"), this,
                        &PersistentMainWindow::resetToDefaultLayout);
}

// ─────────────────────────────────────────────────────────────────────────────
// 布局恢复
// ─────────────────────────────────────────────────────────────────────────────

void PersistentMainWindow::restoreLayout()
{
    QSettings settings(QStringLiteral("AwesomeQt"), QStringLiteral("MainWindowDemo"));

    // 先恢复窗口几何信息（位置、大小、最大化状态）
    restoreGeometry(settings.value(kGeometryKey).toByteArray());

    // restoreState 必须在窗口 show() 之后调用才能正确还原停靠位置
    const bool restored =
        restoreState(settings.value(kStateKey).toByteArray(), kStateVersion);

    if (!restored) {
        // 首次运行：默认隐藏编译输出面板，模拟 IDE 行为
        m_outputDock->hide();
        statusBar()->showMessage(QStringLiteral("首次运行——使用默认布局"), 3000);
    } else {
        statusBar()->showMessage(QStringLiteral("已恢复上次的窗口布局"), 3000);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 重置默认布局
// ─────────────────────────────────────────────────────────────────────────────

void PersistentMainWindow::resetToDefaultLayout()
{
    // 清除 QSettings 中保存的状态数据
    QSettings settings(QStringLiteral("AwesomeQt"), QStringLiteral("MainWindowDemo"));
    settings.remove(kGeometryKey);
    settings.remove(kStateKey);

    // 显示所有 Dock
    m_fileExplorerDock->show();
    m_propertyDock->show();
    m_toolBoxDock->show();
    m_outputDock->show();

    // 重新停靠到初始位置
    reDock(m_fileExplorerDock, Qt::LeftDockWidgetArea);
    reDock(m_propertyDock, Qt::RightDockWidgetArea);
    reDock(m_toolBoxDock, Qt::RightDockWidgetArea);
    reDock(m_outputDock, Qt::BottomDockWidgetArea);

    // 重新 tab 化
    tabifyDockWidget(m_propertyDock, m_toolBoxDock);
    m_propertyDock->raise();

    // 恢复默认窗口大小
    resize(1100, 700);

    statusBar()->showMessage(QStringLiteral("已重置为默认布局"), 3000);
}

void PersistentMainWindow::reDock(QDockWidget* dock, Qt::DockWidgetArea area)
{
    // 先设为浮动再停靠，确保能从任意状态回到目标区域
    dock->setFloating(false);
    addDockWidget(area, dock);
}

// ─────────────────────────────────────────────────────────────────────────────
// closeEvent——保存布局
// ─────────────────────────────────────────────────────────────────────────────

void PersistentMainWindow::closeEvent(QCloseEvent* event)
{
    // 保存窗口几何信息和 Dock/工具栏布局到 QSettings
    QSettings settings(QStringLiteral("AwesomeQt"), QStringLiteral("MainWindowDemo"));
    settings.setValue(kGeometryKey, saveGeometry());
    settings.setValue(kStateKey, saveState(kStateVersion));

    QMainWindow::closeEvent(event);
}
