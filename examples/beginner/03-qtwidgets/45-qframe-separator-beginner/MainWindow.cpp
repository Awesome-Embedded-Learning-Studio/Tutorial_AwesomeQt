// QtWidgets 入门示例 45: QFrame 作为分隔线的用法
// 演示：水平分隔线 HLine+Sunken
//       垂直分隔线在工具栏中使用
//       QFrame 作为有边框容器配置
//       QFrame vs addSpacing 区别

#include "MainWindow.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// 工具函数：创建一条水平分隔线 (HLine + Sunken)
// ============================================================================
QFrame *createHSeparator(int height)
{
    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedHeight(height);
    return separator;
}

// ============================================================================
// 工具函数：创建一条垂直分隔线 (VLine + Sunken)
// ============================================================================
QFrame *createVSeparator(int width)
{
    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(width);
    return separator;
}

// ============================================================================
// 工具函数：创建带标题的水平分隔线 (线 - 文字 - 线)
// ============================================================================
QLayout *createTitleSeparator(const QString &title)
{
    auto *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 6, 0, 6);

    auto *leftLine = new QFrame;
    leftLine->setFrameShape(QFrame::HLine);
    leftLine->setFrameShadow(QFrame::Sunken);
    leftLine->setFixedHeight(2);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(
        "color: #555; font-weight: bold; padding: 0 8px;");

    auto *rightLine = new QFrame;
    rightLine->setFrameShape(QFrame::HLine);
    rightLine->setFrameShadow(QFrame::Sunken);
    rightLine->setFixedHeight(2);

    layout->addWidget(leftLine, 1);
    layout->addWidget(titleLabel);
    layout->addWidget(rightLine, 1);

    return layout;
}

// ============================================================================
// 工具函数：创建一个带边框的容器 QFrame
// ============================================================================
QFrame *createBorderedContainer()
{
    auto *container = new QFrame;
    container->setFrameShape(QFrame::StyledPanel);
    container->setFrameShadow(QFrame::Raised);
    container->setLineWidth(1);
    container->setStyleSheet(
        "QFrame {"
        "  border: 1px solid #CCC;"
        "  border-radius: 6px;"
        "  background-color: #FAFAFA;"
        "}");
    return container;
}

// ============================================================================
// MainWindow: QFrame 分隔线综合演示
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QFrame 分隔线综合演示");
    resize(680, 520);
    initToolbar();
    initCentralWidget();
}

void MainWindow::initToolbar()
{
    auto *toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);

    // 文件操作组
    toolbar->addAction("New");
    toolbar->addAction("Open");
    toolbar->addAction("Save");

    // 垂直分隔线：手动创建 VLine 加入工具栏
    toolbar->addWidget(createVSeparator());

    // 编辑操作组
    toolbar->addAction("Cut");
    toolbar->addAction("Copy");
    toolbar->addAction("Paste");

    // 再加一条 VLine
    toolbar->addWidget(createVSeparator());

    // 其他操作组
    toolbar->addAction("Help");
    toolbar->addAction("About");
}

void MainWindow::initCentralWidget()
{
    auto *centralWidget = new QWidget;
    auto *mainHLayout = new QHBoxLayout(centralWidget);
    mainHLayout->setContentsMargins(8, 8, 8, 8);

    // ==============================================================
    // 左侧：主内容区域
    // ==============================================================
    auto *contentFrame = createBorderedContainer();
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(12, 12, 12, 12);

    // --- 个人信息区域 ---
    contentLayout->addLayout(createTitleSeparator("个人信息"));

    auto *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(new QLabel("姓名:"));
    auto *nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("请输入姓名");
    nameLayout->addWidget(nameEdit, 1);
    contentLayout->addLayout(nameLayout);

    auto *emailLayout = new QHBoxLayout;
    emailLayout->addWidget(new QLabel("邮箱:"));
    auto *emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("请输入邮箱");
    emailLayout->addWidget(emailEdit, 1);
    contentLayout->addLayout(emailLayout);

    // 纯 HLine 分隔两个区域
    contentLayout->addWidget(createHSeparator());

    // --- 工作信息区域 ---
    contentLayout->addLayout(createTitleSeparator("工作信息"));

    auto *deptLayout = new QHBoxLayout;
    deptLayout->addWidget(new QLabel("部门:"));
    auto *deptEdit = new QLineEdit;
    deptEdit->setPlaceholderText("请输入部门");
    deptLayout->addWidget(deptEdit, 1);
    contentLayout->addLayout(deptLayout);

    auto *posLayout = new QHBoxLayout;
    posLayout->addWidget(new QLabel("职位:"));
    auto *posEdit = new QLineEdit;
    posEdit->setPlaceholderText("请输入职位");
    posLayout->addWidget(posEdit, 1);
    contentLayout->addLayout(posLayout);

    contentLayout->addStretch(1);

    // --- 底部：addSpacing 演示 ---
    contentLayout->addWidget(createHSeparator());
    auto *spacingNote = new QLabel(
        "上方表单使用 QFrame 分隔线划分区域，"
        "而不是 addSpacing。分隔线提供可见的视觉分界，"
        "addSpacing 只产生透明间距。");
    spacingNote->setWordWrap(true);
    spacingNote->setStyleSheet("color: #888; font-size: 11px;");
    contentLayout->addWidget(spacingNote);

    // ==============================================================
    // 垂直分隔线：分隔主内容与右侧状态面板
    // ==============================================================
    auto *vSeparator = createVSeparator();

    // ==============================================================
    // 右侧：状态面板
    // ==============================================================
    auto *sideFrame = createBorderedContainer();
    sideFrame->setFixedWidth(160);
    auto *sideLayout = new QVBoxLayout(sideFrame);
    sideLayout->setContentsMargins(10, 10, 10, 10);

    sideLayout->addWidget(new QLabel("状态面板"));

    auto *statusLabel = new QLabel("就绪");
    statusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
    sideLayout->addWidget(statusLabel);

    sideLayout->addWidget(createHSeparator());

    auto *infoLabel = new QLabel(
        "QFrame 分隔线数量:\n"
        "  工具栏 VLine: 2\n"
        "  区域 HLine: 4\n"
        "  侧栏 VLine: 1\n"
        "  总计: 7 条");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; font-size: 11px;");
    sideLayout->addWidget(infoLabel);

    sideLayout->addStretch(1);

    // ==============================================================
    // 组装水平主布局
    // ==============================================================
    mainHLayout->addWidget(contentFrame, 1);
    mainHLayout->addWidget(vSeparator);
    mainHLayout->addWidget(sideFrame);

    setCentralWidget(centralWidget);
}
