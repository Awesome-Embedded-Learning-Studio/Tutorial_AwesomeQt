/// @file    toolbar_demo.cpp
/// @brief   ToolbarDemo 实现——响应式工具栏自动折叠。
///
/// 对应教程：进阶层 03-QtWidgets/57-QToolBar 进阶。

#include "toolbar_demo.h"

#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QResizeEvent>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QToolBar>

ToolbarDemo::ToolbarDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_mainToolbar(nullptr)
    , m_sizeInfoLabel(nullptr)
{
    setupUI();

    setWindowTitle(tr("QToolBar Responsive Demo"));
    resize(700, 450);
}

void ToolbarDemo::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateToolbarStyle();
}

void ToolbarDemo::setupUI()
{
    // 中央控件
    auto* central = new QTextEdit(this);
    central->setPlaceholderText(
        tr("Resize the window to see toolbar style change...\n\n"
           "Width >= %1px: TextBesideIcon\n"
           "Width < %1px: IconOnly").arg(kCompactThreshold));
    setCentralWidget(central);

    // --- 主工具栏 ---
    m_mainToolbar = addToolBar(tr("Main Toolbar"));
    // @note 默认使用 TextBesideIcon，窗口缩小时自动切换
    m_mainToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_mainToolbar->setMovable(false);    // 禁止拖动，简化演示

    // 添加多个 action 模拟真实工具栏
    auto* newAction = m_mainToolbar->addAction(
        style()->standardIcon(QStyle::SP_FileIcon), tr("New"));
    auto* openAction = m_mainToolbar->addAction(
        style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Open"));
    auto* saveAction = m_mainToolbar->addAction(
        style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save"));

    m_mainToolbar->addSeparator();

    auto* undoAction = m_mainToolbar->addAction(
        style()->standardIcon(QStyle::SP_ArrowBack), tr("Undo"));
    auto* redoAction = m_mainToolbar->addAction(
        style()->standardIcon(QStyle::SP_ArrowForward), tr("Redo"));

    m_mainToolbar->addSeparator();

    auto* cutAction = m_mainToolbar->addAction(
        style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Cut"));
    auto* copyAction = m_mainToolbar->addAction(
        style()->standardIcon(QStyle::SP_DialogApplyButton), tr("Copy"));

    // 连接一些简单的状态反馈
    for (auto* action : m_mainToolbar->actions()) {
        if (!action->isSeparator()) {
            connect(action, &QAction::triggered, this, [this, action]() {
                statusBar()->showMessage(
                    tr("Clicked: %1").arg(action->text()), 2000);
            });
        }
    }

    // --- 第二个工具栏（演示 toolbar break） ---
    auto* secondToolbar = addToolBar(tr("View Toolbar"));
    secondToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    secondToolbar->setMovable(false);

    auto* zoomInAction = secondToolbar->addAction(tr("Zoom In"));
    auto* zoomOutAction = secondToolbar->addAction(tr("Zoom Out"));

    // @note insertToolBarBreak 在两个工具栏之间插入换行
    // 取消下行注释可以看到工具栏分行显示
    // insertToolBarBreak(secondToolbar);

    // --- 状态栏 ---
    m_sizeInfoLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_sizeInfoLabel, 1);

    // 初始更新
    updateToolbarStyle();
}

void ToolbarDemo::updateToolbarStyle()
{
    const int width = this->width();
    Qt::ToolButtonStyle style;

    if (width < kCompactThreshold) {
        // @note 窄窗口时折叠为仅图标，节省水平空间
        style = Qt::ToolButtonIconOnly;
    } else {
        style = Qt::ToolButtonTextBesideIcon;
    }

    // 更新所有工具栏的按钮样式
    for (auto* toolbar : findChildren<QToolBar*>()) {
        toolbar->setToolButtonStyle(style);
    }

    const QString styleName = (style == Qt::ToolButtonIconOnly)
        ? QStringLiteral("IconOnly")
        : QStringLiteral("TextBesideIcon");

    m_sizeInfoLabel->setText(
        tr("Width: %1px | Toolbar style: %2").arg(width).arg(styleName));
}
