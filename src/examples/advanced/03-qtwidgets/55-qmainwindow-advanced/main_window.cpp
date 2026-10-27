/// @file    main_window.cpp
/// @brief   MainWindow 实现——多显示器适配与全屏模式切换。
///
/// 对应教程：进阶层 03-QtWidgets/55-QMainWindow 多显示器适配与全屏模式切换。

#include "main_window.h"

#include <QAction>
#include <QDockWidget>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>
#include <QStatusBar>
#include <QTimer>
#include <QTextEdit>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_screenInfoLabel(nullptr)
    , m_dockLeft(nullptr)
    , m_dockRight(nullptr)
    , m_isFullscreen(false)
{
    setWindowTitle(tr("QMainWindow Advanced Demo"));

    // 中央控件
    auto* centralEdit = new QTextEdit(this);
    centralEdit->setPlaceholderText(tr("Central widget area..."));
    setCentralWidget(centralEdit);

    setupDockWidgets();
    setupMenus();

    // 状态栏屏幕信息标签
    m_screenInfoLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_screenInfoLabel, 1);
    updateScreenInfo();

    // @note 屏幕添加/移除时更新信息
    // qGuiApp 是 QGuiApplication* 全局指针，类型正确可直接连接信号
    connect(qGuiApp, &QGuiApplication::screenAdded,
            this, &MainWindow::updateScreenInfo);
    connect(qGuiApp, &QGuiApplication::screenRemoved,
            this, &MainWindow::updateScreenInfo);

    // @note 使用定时器定期刷新屏幕信息（简化实现，避免 QWindow 信号依赖）
    auto* refreshTimer = new QTimer(this);
    refreshTimer->setInterval(2000);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::updateScreenInfo);
    refreshTimer->start();

    resize(800, 600);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F11) {
        toggleFullscreen();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::moveToScreen()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    const int screenIndex = action->data().toInt();
    const QList<QScreen*> screens = QGuiApplication::screens();

    if (screenIndex < 0 || screenIndex >= screens.size()) {
        return;
    }

    QScreen* targetScreen = screens.at(screenIndex);
    // @note setGeometry 使用目标屏幕的坐标体系，需要加上屏幕偏移
    const QRect screenGeometry = targetScreen->availableGeometry();
    setGeometry(screenGeometry);
}

void MainWindow::toggleFullscreen()
{
    if (m_isFullscreen) {
        // 恢复正常模式
        showNormal();
        setGeometry(m_normalGeometry);
        menuBar()->show();
        statusBar()->show();
        m_isFullscreen = false;
    } else {
        // 保存当前几何信息用于恢复
        m_normalGeometry = geometry();
        menuBar()->hide();
        statusBar()->hide();
        showFullScreen();
        m_isFullscreen = true;
    }
}

void MainWindow::updateScreenInfo()
{
    QScreen* currentScreen = screen();
    if (!currentScreen) {
        m_screenInfoLabel->setText(tr("Screen: Unknown"));
        return;
    }

    const QString name = currentScreen->name();
    const QSize resolution = currentScreen->size();
    const qreal dpi = currentScreen->logicalDotsPerInch();
    const qreal scaleFactor = currentScreen->devicePixelRatio();

    m_screenInfoLabel->setText(
        tr("Screen: %1 | %2x%3 | DPI: %4 | Scale: %5")
            .arg(name)
            .arg(resolution.width())
            .arg(resolution.height())
            .arg(dpi, 0, 'f', 1)
            .arg(scaleFactor, 0, 'f', 1));
}

void MainWindow::setupMenus()
{
    auto* viewMenu = menuBar()->addMenu(tr("&View"));

    // 全屏切换
    auto* fullscreenAction = viewMenu->addAction(tr("Toggle Fullscreen (F11)"));
    connect(fullscreenAction, &QAction::triggered,
            this, &MainWindow::toggleFullscreen);

    viewMenu->addSeparator();

    // 多显示器子菜单
    auto* screenMenu = viewMenu->addMenu(tr("Move to Screen"));
    // @note 每次显示前重建菜单，因为屏幕可能热插拔
    connect(screenMenu, &QMenu::aboutToShow, this, [this, screenMenu]() {
        screenMenu->clear();
        const QList<QScreen*> screens = QGuiApplication::screens();
        for (int i = 0; i < screens.size(); ++i) {
            QScreen* scr = screens.at(i);
            auto* action = screenMenu->addAction(
                tr("Screen %1: %2 (%3x%4)")
                    .arg(i + 1)
                    .arg(scr->name())
                    .arg(scr->size().width())
                    .arg(scr->size().height()));
            action->setData(i);
            connect(action, &QAction::triggered,
                    this, &MainWindow::moveToScreen);
        }
    });
}

void MainWindow::setupDockWidgets()
{
    m_dockLeft = new QDockWidget(tr("File Tree"), this);
    m_dockLeft->setWidget(new QTextEdit(this));
    addDockWidget(Qt::LeftDockWidgetArea, m_dockLeft);

    m_dockRight = new QDockWidget(tr("Properties"), this);
    m_dockRight->setWidget(new QTextEdit(this));
    addDockWidget(Qt::RightDockWidgetArea, m_dockRight);
}
