// QtWidgets 入门示例 40: QTabBar 独立标签栏
// 演示：QTabBar 与 QTabWidget 的区别（可独立使用）
//       自定义标签栏 + 自定义内容区域组合
//       tabCloseRequested 信号实现可关闭标签页
//       setMovable(true) 可拖动标签排序

#include "TabBarDemoWidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// TabBarDemoWidget: QTabBar 综合演示窗口
// ============================================================================
TabBarDemoWidget::TabBarDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QTabBar 综合演示 — 独立标签栏");
    resize(700, 500);
    initUi();
}

/// @brief 初始化界面
void TabBarDemoWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ================================================================
    // 顶部: QTabBar（独立标签栏）
    // ================================================================
    m_tabBar = new QTabBar;
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);
    m_tabBar->setDrawBase(false);
    m_tabBar->setExpanding(false);

    // 初始标签页
    m_tabBar->addTab("README.md");
    m_tabBar->addTab("main.cpp");
    m_tabBar->addTab("CMakeLists.txt");

    mainLayout->addWidget(m_tabBar);

    // ================================================================
    // 工具栏: 新建文档 + 关闭全部 + 状态
    // ================================================================
    auto *toolbarLayout = new QHBoxLayout;

    auto *newDocBtn = new QPushButton("新建文档");
    newDocBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #388E3C; color: white;"
        "  border: none; border-radius: 4px;"
        "  padding: 5px 14px; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #2E7D32; }");
    toolbarLayout->addWidget(newDocBtn);

    auto *closeAllBtn = new QPushButton("关闭全部");
    closeAllBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #D32F2F; color: white;"
        "  border: none; border-radius: 4px;"
        "  padding: 5px 14px; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #C62828; }");
    toolbarLayout->addWidget(closeAllBtn);

    toolbarLayout->addStretch();

    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet(
        "color: #555; font-size: 11px; padding: 4px 8px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    toolbarLayout->addWidget(m_statusLabel);

    mainLayout->addLayout(toolbarLayout);

    // ================================================================
    // 中央: QSplitter 分割——左侧编辑区 + 右侧预览区
    // ================================================================
    m_editorStack = new QStackedWidget;
    m_previewStack = new QStackedWidget;

    // 为初始标签创建编辑器和预览标签
    createDocumentContent("README.md",
        "# My Project\n\nThis is a sample README file.\n\n"
        "Features:\n- Feature A\n- Feature B\n- Feature C");
    createDocumentContent("main.cpp",
        "#include <QApplication>\n#include <QLabel>\n\n"
        "int main(int argc, char *argv[])\n{\n"
        "    QApplication app(argc, argv);\n"
        "    QLabel label(\"Hello, Qt!\");\n"
        "    label.show();\n"
        "    return app.exec();\n}");
    createDocumentContent("CMakeLists.txt",
        "cmake_minimum_required(VERSION 3.26)\n"
        "project(myapp LANGUAGES CXX)\n\n"
        "set(CMAKE_CXX_STANDARD 17)\n"
        "find_package(Qt6 REQUIRED COMPONENTS Widgets)\n"
        "qt_add_executable(myapp main.cpp)\n"
        "target_link_libraries(myapp PRIVATE Qt6::Widgets)");

    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(m_editorStack);
    splitter->addWidget(m_previewStack);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({500, 200});

    mainLayout->addWidget(splitter, 1);

    // ================================================================
    // 信号连接
    // ================================================================

    // 标签切换 → 同步两个堆叠页面
    connect(m_tabBar, &QTabBar::currentChanged,
            this, [this](int index) {
        if (index < 0) {
            m_statusLabel->setText("所有文档已关闭");
            return;
        }
        m_editorStack->setCurrentIndex(index);
        m_previewStack->setCurrentIndex(index);

        QString name = m_tabBar->tabText(index);
        int total = m_tabBar->count();
        m_statusLabel->setText(
            QString("文档: %1 | 索引: %2 | 总计: %3 个")
                .arg(name)
                .arg(index)
                .arg(total));
    });

    // 关闭标签
    connect(m_tabBar, &QTabBar::tabCloseRequested,
            this, [this](int index) {
        QString name = m_tabBar->tabText(index);

        auto result = QMessageBox::question(
            this, "关闭文档",
            QString("确定关闭「%1」吗？").arg(name),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (result == QMessageBox::Yes) {
            removeDocument(index);
        }
    });

    // 标签拖动 → 同步移动堆叠页面
    connect(m_tabBar, &QTabBar::tabMoved,
            this, [this](int from, int to) {
        // 同步编辑器堆叠页面
        QWidget *editorPage = m_editorStack->widget(from);
        m_editorStack->removeWidget(editorPage);
        m_editorStack->insertWidget(to, editorPage);

        // 同步预览堆叠页面
        QWidget *previewPage = m_previewStack->widget(from);
        m_previewStack->removeWidget(previewPage);
        m_previewStack->insertWidget(to, previewPage);

        // 恢复当前显示
        m_editorStack->setCurrentIndex(m_tabBar->currentIndex());
        m_previewStack->setCurrentIndex(m_tabBar->currentIndex());
    });

    // 新建文档
    connect(newDocBtn, &QPushButton::clicked,
            this, [this]() {
        int count = m_tabBar->count() + 1;
        QString name = QString("untitled_%1.txt").arg(count);
        int idx = m_tabBar->addTab(name);
        createDocumentContent(name, "");

        m_tabBar->setCurrentIndex(idx);
    });

    // 关闭全部
    connect(closeAllBtn, &QPushButton::clicked,
            this, [this]() {
        if (m_tabBar->count() == 0) return;

        auto result = QMessageBox::question(
            this, "关闭全部",
            "确定关闭所有文档吗？",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (result == QMessageBox::Yes) {
            while (m_tabBar->count() > 0) {
                removeDocument(0);
            }
        }
    });

    // 初始化状态
    updateStatus();
}

/// @brief 为指定文档名创建编辑器和预览页
void TabBarDemoWidget::createDocumentContent(const QString &name,
                               const QString &content)
{
    // 编辑器
    auto *editor = new QTextEdit;
    editor->setPlainText(content);
    editor->setAcceptRichText(false);
    editor->setPlaceholderText(
        QString("在 %1 中输入内容...").arg(name));
    m_editorStack->addWidget(editor);

    // 预览
    auto *preview = new QLabel;
    preview->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    preview->setWordWrap(true);
    preview->setStyleSheet("padding: 8px; color: #333;");
    preview->setText(content.isEmpty()
        ? "(空文档)"
        : content.toHtmlEscaped().replace("\n", "<br>"));
    m_previewStack->addWidget(preview);

    // 编辑器内容变化 → 同步预览
    connect(editor, &QTextEdit::textChanged,
            preview, [editor, preview]() {
        QString text = editor->toPlainText();
        preview->setText(text.isEmpty()
            ? "(空文档)"
            : text.toHtmlEscaped().replace("\n", "<br>"));
    });
}

/// @brief 移除指定索引的文档（标签 + 编辑器 + 预览）
void TabBarDemoWidget::removeDocument(int index)
{
    QWidget *editor = m_editorStack->widget(index);
    QWidget *preview = m_previewStack->widget(index);

    m_tabBar->removeTab(index);
    m_editorStack->removeWidget(editor);
    m_previewStack->removeWidget(preview);

    delete editor;
    delete preview;
}

/// @brief 更新底部状态栏
void TabBarDemoWidget::updateStatus()
{
    int index = m_tabBar->currentIndex();
    int total = m_tabBar->count();
    if (total == 0) {
        m_statusLabel->setText("所有文档已关闭");
    } else {
        QString name = m_tabBar->tabText(index);
        m_statusLabel->setText(
            QString("文档: %1 | 索引: %2 | 总计: %3 个")
                .arg(name).arg(index).arg(total));
    }
}
