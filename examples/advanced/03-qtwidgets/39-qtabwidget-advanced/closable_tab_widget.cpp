/// @file    closable_tab_widget.cpp
/// @brief   ClosableTabWidget 类实现——可关闭标签页与 QTabBar 协作演示。
///
/// 对应教程：进阶层 03-QtWidgets/39-QTabWidget 进阶。

#include "closable_tab_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTabBar>
#include <QTextEdit>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ClosableTabWidget::ClosableTabWidget(QWidget* parent)
    : QWidget(parent)
    , m_tabCounter(0)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 顶部按钮栏：提供"新建标签"操作
    auto* toolbar = new QHBoxLayout;
    auto* addBtn = new QPushButton(QStringLiteral("新建标签"));
    auto* progressBtn = new QPushButton(QStringLiteral("当前标签加进度条"));
    toolbar->addWidget(addBtn);
    toolbar->addWidget(progressBtn);
    toolbar->addStretch();
    mainLayout->addLayout(toolbar);

    // 核心标签页控件
    m_tabWidget = new QTabWidget;
    m_tabWidget->setTabsClosable(true);       // 开启关闭按钮（仅视觉，不自动移除）
    m_tabWidget->setDocumentMode(true);       // 去掉标签栏外框，现代编辑器风格
    m_tabWidget->setMovable(true);            // 允许拖拽重排标签
    m_tabWidget->setElideMode(Qt::ElideRight);// 标签文字超长时右侧省略

    // documentMode 下标签栏和页面之间没有分隔线，通过 QSS 补一条顶线
    m_tabWidget->setStyleSheet(
        QStringLiteral("QTabWidget::pane { border-top: 1px solid #c0c0c0; }"));

    // 通过 tabBar() 设置图标大小——QTabWidget 没有直接暴露此接口
    m_tabWidget->tabBar()->setIconSize(QSize(20, 20));

    mainLayout->addWidget(m_tabWidget, 1);

    // 底部状态栏
    auto* statusLabel = new QLabel(QStringLiteral("就绪"));
    mainLayout->addWidget(statusLabel);

    // ── 信号槽连接 ──

    // tabCloseRequested → 手动移除标签页
    // QTabWidget 只负责发信号，不负责移除，必须在槽函数中完成 removeTab + deleteLater
    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &ClosableTabWidget::handleCloseRequested);

    // 标签切换 → 更新底部状态栏
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &ClosableTabWidget::updateStatusBar);

    // 新建标签按钮
    connect(addBtn, &QPushButton::clicked, this, &ClosableTabWidget::addNewTab);

    // 为当前标签附加进度条按钮
    connect(progressBtn, &QPushButton::clicked, this, [this]() {
        const int idx = m_tabWidget->currentIndex();
        if (idx >= 0) {
            attachProgressButton(idx);
        }
    });

    // 初始创建三个标签页
    addNewTab();
    addNewTab();
    addNewTab();

    // 保存状态标签引用以便在 updateStatusBar 中使用
    // 通过 parentWidget()->findChild 方式或直接用成员都可以，这里用布局简化
    // 将 statusLabel 存入 m_tabWidget 的属性，便于槽函数访问
    m_tabWidget->setProperty("statusLabel",
                             QVariant::fromValue(static_cast<QWidget*>(statusLabel)));

    setWindowTitle(QStringLiteral("QTabWidget Advanced Demo"));
    resize(700, 500);
}

// ─────────────────────────────────────────────────────────────────────────────
// 新建标签页
// ─────────────────────────────────────────────────────────────────────────────

void ClosableTabWidget::addNewTab()
{
    ++m_tabCounter;
    auto* editor = new QTextEdit;
    editor->setPlaceholderText(QStringLiteral("在这里输入内容..."));

    const QString title = QStringLiteral("未命名 %1").arg(m_tabCounter);
    const int index = m_tabWidget->addTab(editor, title);
    m_tabWidget->setCurrentIndex(index);
}

// ─────────────────────────────────────────────────────────────────────────────
// 处理 tabCloseRequested
// ─────────────────────────────────────────────────────────────────────────────

void ClosableTabWidget::handleCloseRequested(int index)
{
    // 关键步骤：先取出页面指针，再移除标签，最后清理
    // removeTab 后 widget(index) 返回的就是下一个页面了，顺序不能反
    QWidget* page = m_tabWidget->widget(index);
    if (!page) {
        return;
    }

    // 如果页面是 QTextEdit 且内容被修改过，弹出确认对话框
    auto* editor = qobject_cast<QTextEdit*>(page);
    if (editor && editor->document()->isModified()) {
        const int ret = QMessageBox::question(
            this,
            QStringLiteral("关闭确认"),
            QStringLiteral("文档 \"%1\" 已修改但未保存，确定关闭吗？")
                .arg(m_tabWidget->tabText(index)),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret != QMessageBox::Yes) {
            return;
        }
    }

    // 先 removeTab 再 deleteLater，确保 QTabWidget 内部索引一致性
    m_tabWidget->removeTab(index);
    page->deleteLater();
}

// ─────────────────────────────────────────────────────────────────────────────
// 更新底部状态栏
// ─────────────────────────────────────────────────────────────────────────────

void ClosableTabWidget::updateStatusBar()
{
    // 通过 property 取出状态标签（构造时存入）
    auto* statusLabel = m_tabWidget->property("statusLabel").value<QWidget*>();
    if (!statusLabel) {
        return;
    }

    auto* label = qobject_cast<QLabel*>(statusLabel);
    if (!label) {
        return;
    }

    QWidget* current = m_tabWidget->currentWidget();
    if (!current) {
        label->setText(QStringLiteral("无打开的标签"));
        return;
    }

    auto* editor = qobject_cast<QTextEdit*>(current);
    if (editor) {
        const int charCount = editor->toPlainText().length();
        const int tabIdx = m_tabWidget->currentIndex();
        label->setText(QStringLiteral("标签 %1 | 字数: %2")
                           .arg(tabIdx + 1)
                           .arg(charCount));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 通过 tabBar() 的 setTabButton 在标签上嵌入进度条
// ─────────────────────────────────────────────────────────────────────────────

void ClosableTabWidget::attachProgressButton(int index)
{
    // 演示通过 tabBar() 访问 QTabWidget 未直接暴露的 setTabButton 接口
    // 注意：setTabsClosable(true) 已经在 RightSide 放了内置关闭按钮
    // 在 LeftSide 放进度条，避免覆盖关闭按钮
    auto* progress = new QProgressBar;
    progress->setMaximumWidth(80);
    progress->setMaximumHeight(12);
    progress->setValue(67);
    progress->setToolTip(QStringLiteral("加载进度 67%"));

    // LeftSide 不会和 setTabsClosable 的关闭按钮冲突
    m_tabWidget->tabBar()->setTabButton(index, QTabBar::LeftSide, progress);
}
