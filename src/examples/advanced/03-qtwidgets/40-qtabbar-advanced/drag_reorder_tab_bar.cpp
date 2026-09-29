/// @file    drag_reorder_tab_bar.cpp
/// @brief   DragReorderTabBar 类实现——QTabBar 拖拽排序与自定义按钮演示。
///
/// 对应教程：进阶层 03-QtWidgets/40-QTabBar 进阶。

#include "drag_reorder_tab_bar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

DragReorderTabBar::DragReorderTabBar(QWidget* parent)
    : QWidget(parent)
    , m_tabBar(nullptr)
    , m_stackedWidget(nullptr)
    , m_infoLabel(nullptr)
    , m_dragging(false)
    , m_dragStartIndex(-1)
{
    auto* mainLayout = new QVBoxLayout(this);

    // ── 独立标签栏 ──
    m_tabBar = new QTabBar;
    m_tabBar->setMovable(true);             // 启用内置拖拽排序
    m_tabBar->setTabsClosable(true);        // 内置关闭按钮（RightSide）
    m_tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab); // 关闭后回到上次访问的标签
    m_tabBar->setUsesScrollButtons(true);   // 标签溢出时显示滚动按钮
    m_tabBar->setElideMode(Qt::ElideRight); // 超长标签文字右侧省略

    mainLayout->addWidget(m_tabBar);

    // ── 堆叠页面容器 ──
    // 手动管理 QTabBar + QStackedWidget 的协作（QTabWidget 内部也是这么做的）
    m_stackedWidget = new QStackedWidget;
    mainLayout->addWidget(m_stackedWidget, 1);

    // ── 底部信息标签 ──
    m_infoLabel = new QLabel;
    mainLayout->addWidget(m_infoLabel);

    // ── 信号槽连接 ──

    // 标签切换 → 同步 QStackedWidget 当前页面
    connect(m_tabBar, &QTabBar::currentChanged,
            m_stackedWidget, &QStackedWidget::setCurrentIndex);

    // 内置关闭按钮 → tabCloseRequested → 移除标签和页面
    connect(m_tabBar, &QTabBar::tabCloseRequested,
            this, &DragReorderTabBar::removeTabAtIndex);

    // 标签移动 → 同步 QStackedWidget 的页面顺序
    connect(m_tabBar, &QTabBar::tabMoved,
            this, &DragReorderTabBar::syncStackedWidget);

    // 标签切换/移动/移除 → 更新底部信息
    connect(m_tabBar, &QTabBar::currentChanged, this, &DragReorderTabBar::updateInfoLabel);
    connect(m_tabBar, &QTabBar::tabMoved, this, &DragReorderTabBar::updateInfoLabel);

    // 创建初始标签页
    setupInitialTabs();
    updateInfoLabel();

    // 鼠标追踪——让 mouseMoveEvent 在不按下按钮时也能触发
    setMouseTracking(true);

    setWindowTitle(QStringLiteral("QTabBar Advanced Demo"));
    resize(700, 400);
}

// ─────────────────────────────────────────────────────────────────────────────
// 鼠标事件——拖拽重排（演示 setMovable 的底层机制）
// ─────────────────────────────────────────────────────────────────────────────

void DragReorderTabBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // 检查鼠标是否在标签栏的某个标签上
        const int tabIndex = m_tabBar->tabAt(m_tabBar->mapFromParent(event->pos()));
        if (tabIndex >= 0) {
            m_dragging = true;
            m_dragStartIndex = tabIndex;
            m_dragStartPos = event->pos();
        }
    }
    QWidget::mousePressEvent(event);
}

void DragReorderTabBar::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        // 检查拖拽距离是否超过阈值
        const int dragDistance = (event->pos() - m_dragStartPos).manhattanLength();
        if (dragDistance <= QApplication::startDragDistance()) {
            QWidget::mouseMoveEvent(event);
            return;
        }

        // 计算当前鼠标位置对应的目标标签索引
        const int targetIndex = m_tabBar->tabAt(m_tabBar->mapFromParent(event->pos()));

        // 如果目标位置有效且不同于起始位置，执行 moveTab
        // 注意：setMovable(true) 已经内置了这个逻辑，
        // 这里手动演示 moveTab 的调用方式，供教学理解
        if (targetIndex >= 0 && targetIndex != m_dragStartIndex) {
            m_tabBar->moveTab(m_dragStartIndex, targetIndex);
            m_dragStartIndex = targetIndex; // 更新起始索引，跟踪连续移动
        }
    }
    QWidget::mouseMoveEvent(event);
}

void DragReorderTabBar::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
    m_dragStartIndex = -1;
    QWidget::mouseReleaseEvent(event);
}

// ─────────────────────────────────────────────────────────────────────────────
// 创建初始标签页
// ─────────────────────────────────────────────────────────────────────────────

void DragReorderTabBar::setupInitialTabs()
{
    const int kTabCount = 5;
    for (int i = 0; i < kTabCount; ++i) {
        const QString title = QStringLiteral("文档 %1").arg(i + 1);
        auto* page = new QLabel(QStringLiteral("这是 %1 的内容区域。\n\n"
                                               "可以拖拽标签重排顺序，"
                                               "点击关闭按钮移除标签。").arg(title));
        page->setAlignment(Qt::AlignCenter);
        page->setWordWrap(true);

        const int index = m_tabBar->addTab(title);
        m_stackedWidget->addWidget(page);

        // 为每个标签添加自定义按钮
        attachTabButtons(index);
    }

    if (m_tabBar->count() > 0) {
        m_tabBar->setCurrentIndex(0);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 为标签添加自定义按钮：左侧状态指示点 + 右侧由 setTabsClosable 管理
// ─────────────────────────────────────────────────────────────────────────────

void DragReorderTabBar::attachTabButtons(int index)
{
    // 在标签左侧放一个绿色的状态指示点
    // setTabsClosable(true) 已经占用了 RightSide，自定义控件放 LeftSide 避免冲突
    auto* dot = new QLabel;
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QStringLiteral("background: #4caf50; border-radius: 4px;"));
    dot->setToolTip(QStringLiteral("状态正常"));

    m_tabBar->setTabButton(index, QTabBar::LeftSide, dot);
}

// ─────────────────────────────────────────────────────────────────────────────
// 移除标签并同步 QStackedWidget
// ─────────────────────────────────────────────────────────────────────────────

void DragReorderTabBar::removeTabAtIndex(int index)
{
    if (index < 0 || index >= m_stackedWidget->count()) {
        return;
    }

    // 先取出页面指针，再移除，最后清理——顺序和 QTabWidget 一样
    QWidget* page = m_stackedWidget->widget(index);
    m_tabBar->removeTab(index);
    m_stackedWidget->removeWidget(page);
    page->deleteLater();

    updateInfoLabel();
}

// ─────────────────────────────────────────────────────────────────────────────
// tabMoved → 同步 QStackedWidget 页面顺序
// ─────────────────────────────────────────────────────────────────────────────

void DragReorderTabBar::syncStackedWidget(int from, int to)
{
    // tabMoved 在 moveTab 时实时触发，每次交换相邻标签都会发一次信号
    // 必须同步 QStackedWidget 的页面顺序，否则标签和内容会错位
    QWidget* page = m_stackedWidget->widget(from);
    m_stackedWidget->removeWidget(page);
    m_stackedWidget->insertWidget(to, page);

    // 保持当前显示页面与标签栏选中一致
    m_stackedWidget->setCurrentIndex(m_tabBar->currentIndex());
}

// ─────────────────────────────────────────────────────────────────────────────
// 更新底部信息标签
// ─────────────────────────────────────────────────────────────────────────────

void DragReorderTabBar::updateInfoLabel()
{
    const int current = m_tabBar->currentIndex();
    const int total = m_tabBar->count();

    if (total == 0) {
        m_infoLabel->setText(QStringLiteral("所有标签已关闭"));
    } else {
        m_infoLabel->setText(
            QStringLiteral("当前标签: %1 / %2 | 选择策略: SelectPrevious | 拖拽标签可重排")
                .arg(current + 1)
                .arg(total));
    }
}
