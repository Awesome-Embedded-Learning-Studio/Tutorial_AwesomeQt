/// @file    menu_bar_demo.cpp
/// @brief   MenuBarDemo 类实现——动态菜单与最近文件列表。
///
/// 对应教程：进阶层 03-QtWidgets/56-QMenuBar 进阶。

#include "menu_bar_demo.h"

#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QDateTime>

MenuBarDemo::MenuBarDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_statusLabel(nullptr)
    , m_recentMenu(nullptr)
{
    auto* central = new QTextEdit(this);
    central->setPlaceholderText(tr("Main content area..."));
    setCentralWidget(central);

    m_statusLabel = new QLabel(tr("Ready"), this);
    statusBar()->addWidget(m_statusLabel, 1);

    setupMenus();

    setWindowTitle(tr("QMenuBar Advanced Demo"));
    resize(600, 400);
}

void MenuBarDemo::setupMenus()
{
    // --- File 菜单 ---
    auto* fileMenu = menuBar()->addMenu(tr("&File"));

    auto* openAction = fileMenu->addAction(tr("&Open File..."));
    connect(openAction, &QAction::triggered, this, &MenuBarDemo::openFile);

    fileMenu->addSeparator();

    // @note "最近文件"菜单在 aboutToShow 时重建，保证列表最新
    m_recentMenu = fileMenu->addMenu(tr("Recent Files"));
    connect(m_recentMenu, &QMenu::aboutToShow,
            this, &MenuBarDemo::rebuildRecentFilesMenu);

    fileMenu->addSeparator();

    auto* clearAction = fileMenu->addAction(tr("Clear Recent Files"));
    connect(clearAction, &QAction::triggered, this, &MenuBarDemo::clearRecentFiles);

    fileMenu->addSeparator();

    auto* exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // --- Edit 菜单（动态添加 action 演示） ---
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));

    auto* addAction = editMenu->addAction(tr("Add Dynamic Item"));
    connect(addAction, &QAction::triggered, this, [this, editMenu]() {
        // @note 运行时动态向菜单添加新项
        const int count = editMenu->actions().size();
        auto* newAction = editMenu->addAction(
            tr("Dynamic Item %1").arg(count));
        connect(newAction, &QAction::triggered, this, [this, newAction]() {
            updateStatus(tr("Triggered: %1").arg(newAction->text()));
        });
        updateStatus(tr("Added: %1").arg(newAction->text()));
    });

    // --- Help 菜单 ---
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    auto* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        updateStatus(tr("MenuBar Demo - demonstrates dynamic menus and recent files"));
    });
}

void MenuBarDemo::openFile()
{
    // @note 模拟打开文件：生成带时间戳的文件名
    const QString timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
    const QString fileName = tr("document_%1.txt").arg(timestamp);

    // 加入最近文件列表（去重，移到最前面）
    m_recentFiles.removeAll(fileName);
    m_recentFiles.prepend(fileName);

    // @note 超过最大条目数时移除最旧的
    while (m_recentFiles.size() > kMaxRecentFiles) {
        m_recentFiles.removeLast();
    }

    updateStatus(tr("Opened: %1").arg(fileName));
}

void MenuBarDemo::clearRecentFiles()
{
    m_recentFiles.clear();
    updateStatus(tr("Recent files cleared"));
}

void MenuBarDemo::openRecentFile()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    const QString fileName = action->data().toString();
    updateStatus(tr("Opened recent: %1").arg(fileName));
}

void MenuBarDemo::rebuildRecentFilesMenu()
{
    m_recentMenu->clear();

    if (m_recentFiles.isEmpty()) {
        // @note 空列表时显示一个禁用的占位项
        auto* emptyAction = m_recentMenu->addAction(tr("(No recent files)"));
        emptyAction->setEnabled(false);
        return;
    }

    for (int i = 0; i < m_recentFiles.size(); ++i) {
        // @note 编号前缀 1-5 是常见惯例，方便用户识别
        auto* action = m_recentMenu->addAction(
            tr("&%1 %2").arg(i + 1).arg(m_recentFiles.at(i)));
        action->setData(m_recentFiles.at(i));
        connect(action, &QAction::triggered,
                this, &MenuBarDemo::openRecentFile);
    }
}

void MenuBarDemo::updateStatus(const QString& message)
{
    m_statusLabel->setText(message);
}
