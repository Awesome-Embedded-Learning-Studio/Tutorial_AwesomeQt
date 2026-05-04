#include "mainwindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// MainWindow: QSplitter 综合演示主窗口（模拟 IDE 三区布局）
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QSplitter 综合演示 — IDE 布局");
    resize(960, 640);
    initUi();
    restoreSplitterState();
}

MainWindow::~MainWindow() { saveSplitterState(); }

/// @brief 初始化界面：外层水平分割 + 内层垂直分割
void MainWindow::initUi()
{
    // ================================================================
    // 外层水平分割器：左侧文件树 | 右侧垂直分割器
    // ================================================================
    m_outerSplitter = new QSplitter(Qt::Horizontal);

    // ---- 左侧：项目文件树 ----
    m_treeWidget = new QTreeWidget;
    m_treeWidget->setHeaderLabel("项目文件");
    m_treeWidget->setMinimumWidth(120);
    populateSampleTree();

    // ---- 右侧：垂直分割器（编辑区 + 输出面板） ----
    m_innerSplitter = new QSplitter(Qt::Vertical);

    m_editor = new QTextEdit;
    m_editor->setPlaceholderText(
        "// 在此编写代码...\n// 拖动分割线调整各区域大小\n"
        "// 关闭窗口后分割比例会被保存");

    m_outputPanel = new QTextEdit;
    m_outputPanel->setReadOnly(true);
    m_outputPanel->setPlaceholderText("输出面板（只读）");

    m_innerSplitter->addWidget(m_editor);
    m_innerSplitter->addWidget(m_outputPanel);
    // 输出面板默认 150 像素高
    m_innerSplitter->setSizes({450, 150});
    m_innerSplitter->setCollapsible(0, false);
    m_innerSplitter->setCollapsible(1, false);

    // 组装外层分割器
    m_outerSplitter->addWidget(m_treeWidget);
    m_outerSplitter->addWidget(m_innerSplitter);
    // 文件树默认 200 像素宽
    m_outerSplitter->setSizes({200, 700});
    m_outerSplitter->setCollapsible(0, false);
    m_outerSplitter->setCollapsible(1, false);

    setCentralWidget(m_outerSplitter);

    // ================================================================
    // 工具栏
    // ================================================================
    auto *toolbar = addToolBar("操作");
    toolbar->setMovable(false);

    auto *resetBtn = new QPushButton("重置默认布局");
    connect(resetBtn, &QPushButton::clicked, this,
            &MainWindow::resetToDefaultLayout);
    toolbar->addWidget(resetBtn);

    auto *showSizesBtn = new QPushButton("打印当前 sizes()");
    connect(showSizesBtn, &QPushButton::clicked, this,
            &MainWindow::showCurrentSizes);
    toolbar->addWidget(showSizesBtn);

    // ---- 初始日志 ----
    log("QSplitter 综合演示已启动");
    log("外层水平分割器：文件树 (左) | 编辑区+输出 (右)");
    log("内层垂直分割器：编辑区 (上) | 输出面板 (下)");
    log("所有区域已禁止折叠 (setCollapsible = false)");
    log("关闭窗口时分割比例自动保存到 QSettings");
}

/// @brief 填充示例文件树
void MainWindow::populateSampleTree()
{
    auto *srcItem = new QTreeWidgetItem(m_treeWidget, {"src"});
    new QTreeWidgetItem(srcItem, {"main.cpp"});
    new QTreeWidgetItem(srcItem, {"widget.cpp"});
    new QTreeWidgetItem(srcItem, {"widget.h"});

    auto *resItem = new QTreeWidgetItem(m_treeWidget, {"resources"});
    new QTreeWidgetItem(resItem, {"icon.png"});
    new QTreeWidgetItem(resItem, {"style.qss"});

    new QTreeWidgetItem(m_treeWidget, {"CMakeLists.txt"});
    new QTreeWidgetItem(m_treeWidget, {"README.md"});

    m_treeWidget->expandAll();
}

/// @brief 重置分割器到默认布局
void MainWindow::resetToDefaultLayout()
{
    m_outerSplitter->setSizes({200, 700});
    m_innerSplitter->setSizes({450, 150});
    log("布局已重置为默认值：文件树=200, 编辑区=450, 输出=150");
}

/// @brief 打印当前各分割器的 sizes() 值
void MainWindow::showCurrentSizes()
{
    QList<int> outer = m_outerSplitter->sizes();
    QList<int> inner = m_innerSplitter->sizes();
    log(QString("外层 sizes(): [%1, %2]")
            .arg(outer.value(0))
            .arg(outer.value(1)));
    log(QString("内层 sizes(): [%1, %2]")
            .arg(inner.value(0))
            .arg(inner.value(1)));
}

/// @brief 保存分割器状态到 QSettings
void MainWindow::saveSplitterState()
{
    QSettings settings("AwesomeQt", "QSplitterDemo");
    settings.setValue("outerSplitter", m_outerSplitter->saveState());
    settings.setValue("innerSplitter", m_innerSplitter->saveState());
}

/// @brief 从 QSettings 恢复分割器状态
void MainWindow::restoreSplitterState()
{
    QSettings settings("AwesomeQt", "QSplitterDemo");
    m_outerSplitter->restoreState(
        settings.value("outerSplitter").toByteArray());
    m_innerSplitter->restoreState(
        settings.value("innerSplitter").toByteArray());
}

/// @brief 在输出面板中追加一行日志
void MainWindow::log(const QString &message)
{
    m_outputPanel->append(message);
}
