#include "MainWindow.h"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QKeySequence>
#include <QMenuBar>
#include <QSize>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>

// ============================================================================
// MainWindow: 演示三个工具栏的完整配置
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QToolBar 工具栏演示");
    resize(960, 640);

    // ---- 中央控件 ----
    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText("在这里输入文本...");
    setCentralWidget(m_editor);

    // ---- 构建菜单栏 ----
    setupMenuBar();

    // ---- 构建三个工具栏 ----
    setupFileToolBar();
    setupFormatToolBar();
    setupDrawingToolBar();
}

// ====================================================================
// 菜单栏
// ====================================================================
void MainWindow::setupMenuBar()
{
    auto *viewMenu = menuBar()->addMenu("视图(&V)");
    // toggleViewAction 让用户控制各工具栏的可见性
    // 这些 QAction 在 setupXxxToolBar 中创建后补充到菜单
    m_viewMenu = viewMenu;
}

// ====================================================================
// 文件工具栏：addAction + addSeparator
// ====================================================================
void MainWindow::setupFileToolBar()
{
    m_fileToolBar = addToolBar("文件操作");
    m_fileToolBar->setObjectName("fileToolBar");
    m_fileToolBar->setMovable(true);
    m_fileToolBar->setFloatable(true);
    m_fileToolBar->setIconSize(QSize(20, 20));
    m_fileToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // 新建
    auto *newAction = new QAction("新建", this);
    newAction->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip("新建文件");
    connect(newAction, &QAction::triggered, this, [this]() {
        m_editor->clear();
        statusBar()->showMessage("已新建文件", 2000);
    });
    m_fileToolBar->addAction(newAction);

    // 打开
    auto *openAction = new QAction("打开", this);
    openAction->setIcon(
        style()->standardIcon(QStyle::SP_DirOpenIcon));
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("打开文件");
    connect(openAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("打开文件（示例）", 2000);
    });
    m_fileToolBar->addAction(openAction);

    // 保存
    auto *saveAction = new QAction("保存", this);
    saveAction->setIcon(
        style()->standardIcon(QStyle::SP_DialogSaveButton));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("保存文件");
    connect(saveAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("文件已保存", 3000);
    });
    m_fileToolBar->addAction(saveAction);

    // 分隔线：文件操作和编辑操作之间
    m_fileToolBar->addSeparator();

    // 撤销
    auto *undoAction = new QAction("撤销", this);
    undoAction->setIcon(
        style()->standardIcon(QStyle::SP_ArrowBack));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::undo);
    m_fileToolBar->addAction(undoAction);

    // 重做
    auto *redoAction = new QAction("重做", this);
    redoAction->setIcon(
        style()->standardIcon(QStyle::SP_ArrowForward));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::redo);
    m_fileToolBar->addAction(redoAction);

    // 添加到视图菜单
    m_viewMenu->addAction(m_fileToolBar->toggleViewAction());
}

// ====================================================================
// 格式工具栏：addWidget 嵌入自定义控件
// ====================================================================
void MainWindow::setupFormatToolBar()
{
    m_formatToolBar = addToolBar("格式");
    m_formatToolBar->setObjectName("formatToolBar");
    m_formatToolBar->setIconSize(QSize(20, 20));
    m_formatToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // 嵌入 QComboBox：字体选择
    auto *fontLabel = new QLabel(" 字体: ");
    m_formatToolBar->addWidget(fontLabel);

    auto *fontCombo = new QComboBox;
    fontCombo->addItems({"Monospace", "Sans Serif", "Serif"});
    fontCombo->setMaximumWidth(120);
    m_formatToolBar->addWidget(fontCombo);

    connect(fontCombo, &QComboBox::currentTextChanged,
            this, [this](const QString &family) {
        auto font = m_editor->font();
        font.setFamily(family);
        m_editor->setFont(font);
    });

    // 嵌入 QSpinBox：字号
    auto *sizeLabel = new QLabel(" 字号: ");
    m_formatToolBar->addWidget(sizeLabel);

    auto *sizeSpin = new QSpinBox;
    sizeSpin->setRange(8, 36);
    sizeSpin->setValue(12);
    sizeSpin->setMaximumWidth(60);
    m_formatToolBar->addWidget(sizeSpin);

    connect(sizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int size) {
        auto font = m_editor->font();
        font.setPointSize(size);
        m_editor->setFont(font);
    });

    // 分隔线
    m_formatToolBar->addSeparator();

    // 加粗按钮
    auto *boldAction = new QAction("加粗", this);
    boldAction->setCheckable(true);
    boldAction->setIcon(
        style()->standardIcon(QStyle::SP_DialogOkButton));
    connect(boldAction, &QAction::toggled, this, [this](bool bold) {
        auto font = m_editor->font();
        font.setBold(bold);
        m_editor->setFont(font);
    });
    m_formatToolBar->addAction(boldAction);

    // 添加到视图菜单
    m_viewMenu->addAction(m_formatToolBar->toggleViewAction());
}

// ====================================================================
// 绘图工具栏：限制停靠区域 + 文字在图标下方
// ====================================================================
void MainWindow::setupDrawingToolBar()
{
    m_drawToolBar = addToolBar("绘图");
    m_drawToolBar->setObjectName("drawToolBar");
    // 只允许停靠在左侧或右侧
    m_drawToolBar->setAllowedAreas(Qt::LeftToolBarArea |
                                    Qt::RightToolBarArea);
    // 大图标 + 文字在下方
    m_drawToolBar->setIconSize(QSize(32, 32));
    m_drawToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // 画笔
    auto *penAction = new QAction("画笔", this);
    penAction->setIcon(
        style()->standardIcon(QStyle::SP_DialogResetButton));
    penAction->setCheckable(true);
    penAction->setChecked(true);
    connect(penAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("选中画笔工具", 2000);
    });
    m_drawToolBar->addAction(penAction);

    // 矩形
    auto *rectAction = new QAction("矩形", this);
    rectAction->setIcon(
        style()->standardIcon(QStyle::SP_FileDialogContentsView));
    rectAction->setCheckable(true);
    connect(rectAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("选中矩形工具", 2000);
    });
    m_drawToolBar->addAction(rectAction);

    // 圆形
    auto *circleAction = new QAction("圆形", this);
    circleAction->setIcon(
        style()->standardIcon(QStyle::SP_MediaSeekForward));
    circleAction->setCheckable(true);
    connect(circleAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("选中圆形工具", 2000);
    });
    m_drawToolBar->addAction(circleAction);

    // 添加到视图菜单
    m_viewMenu->addAction(m_drawToolBar->toggleViewAction());
}
