#include "TextEditor.h"

#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>

// ============================================================================
// TextEditor: 带完整菜单系统和右键菜单的文本编辑器
// ============================================================================
TextEditor::TextEditor(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(
        "QMenuBar/QMenu/QAction 综合演示 — 文本编辑器");
    resize(800, 520);

    initCentralWidget();
    createActions();
    initMenuBar();
    initToolBars();
    connectEditorSignals();
}

/// @brief 右键上下文菜单
void TextEditor::contextMenuEvent(
    QContextMenuEvent *event)
{
    auto *menu = new QMenu(this);

    // 根据编辑器状态动态添加菜单项
    bool hasSelection =
        m_editor->textCursor().hasSelection();

    auto *cutAction = menu->addAction("剪切(&T)");
    cutAction->setEnabled(hasSelection);
    cutAction->setShortcut(QKeySequence::Cut);

    auto *copyAction = menu->addAction("复制(&C)");
    copyAction->setEnabled(hasSelection);
    copyAction->setShortcut(QKeySequence::Copy);

    auto *pasteAction = menu->addAction("粘贴(&P)");
    pasteAction->setShortcut(QKeySequence::Paste);

    menu->addSeparator();

    auto *selectAllAction = menu->addAction("全选(&A)");
    selectAllAction->setShortcut(QKeySequence::SelectAll);

    // 执行菜单并处理选择
    QAction *chosen = menu->exec(event->globalPos());

    if (chosen == cutAction) {
        m_editor->cut();
    } else if (chosen == copyAction) {
        m_editor->copy();
    } else if (chosen == pasteAction) {
        m_editor->paste();
    } else if (chosen == selectAllAction) {
        m_editor->selectAll();
    }

    delete menu;
}

/// @brief 初始化中央控件
void TextEditor::initCentralWidget()
{
    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText(
        "在这里输入文本，右键查看上下文菜单...\n\n"
        "本示例演示 Qt 菜单系统的完整功能：\n"
        "- 菜单栏（文件、编辑、格式、帮助）\n"
        "- QAction 共享（菜单 + 工具栏）\n"
        "- setCheckable / QActionGroup\n"
        "- 右键上下文菜单\n");
    setCentralWidget(m_editor);
}

/// @brief 创建所有 QAction
void TextEditor::createActions()
{
    // ================================================================
    // 文件操作
    // ================================================================
    m_newAction = new QAction(
        style()->standardIcon(QStyle::SP_FileIcon),
        "新建(&N)", this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip("创建新文档");
    connect(m_newAction, &QAction::triggered, [this]() {
        m_editor->clear();
        statusBar()->showMessage("已创建新文档", 2000);
    });

    m_openAction = new QAction(
        style()->standardIcon(QStyle::SP_DirOpenIcon),
        "打开(&O)", this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("打开已有文档");
    connect(m_openAction, &QAction::triggered, [this]() {
        statusBar()->showMessage("打开文件（演示）", 2000);
    });

    m_saveAction = new QAction(
        style()->standardIcon(QStyle::SP_DialogSaveButton),
        "保存(&S)", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("保存当前文档");
    connect(m_saveAction, &QAction::triggered, [this]() {
        statusBar()->showMessage("已保存（演示）", 2000);
    });

    m_quitAction = new QAction("退出(&Q)", this);
    m_quitAction->setShortcut(QKeySequence::Quit);
    m_quitAction->setStatusTip("退出应用程序");
    connect(m_quitAction, &QAction::triggered,
            this, &QMainWindow::close);

    // ================================================================
    // 编辑操作
    // ================================================================
    m_undoAction = new QAction(
        style()->standardIcon(QStyle::SP_ArrowBack),
        "撤销(&U)", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setStatusTip("撤销上一步操作");
    m_undoAction->setEnabled(false);
    connect(m_undoAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::undo);

    m_redoAction = new QAction(
        style()->standardIcon(QStyle::SP_ArrowForward),
        "重做(&R)", this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setStatusTip("重做上一步撤销的操作");
    m_redoAction->setEnabled(false);
    connect(m_redoAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::redo);

    m_cutAction = new QAction("剪切(&T)", this);
    m_cutAction->setShortcut(QKeySequence::Cut);
    m_cutAction->setStatusTip("剪切选中文本");
    m_cutAction->setEnabled(false);
    connect(m_cutAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::cut);

    m_copyAction = new QAction("复制(&C)", this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setStatusTip("复制选中文本");
    m_copyAction->setEnabled(false);
    connect(m_copyAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::copy);

    m_pasteAction = new QAction("粘贴(&P)", this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setStatusTip("粘贴剪贴板内容");
    connect(m_pasteAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::paste);

    m_selectAllAction = new QAction("全选(&A)", this);
    m_selectAllAction->setShortcut(QKeySequence::SelectAll);
    m_selectAllAction->setStatusTip("全选文本");
    connect(m_selectAllAction, &QAction::triggered,
            m_editor, &QPlainTextEdit::selectAll);

    // ================================================================
    // 格式操作：字体大小（互斥选择）
    // ================================================================
    auto *sizeGroup = new QActionGroup(this);

    m_smallFontAction = new QAction("小字体", this);
    m_smallFontAction->setCheckable(true);
    m_smallFontAction->setData(10);
    m_smallFontAction->setStatusTip("设置字体大小为 10pt");
    sizeGroup->addAction(m_smallFontAction);

    m_mediumFontAction = new QAction("中字体", this);
    m_mediumFontAction->setCheckable(true);
    m_mediumFontAction->setChecked(true);
    m_mediumFontAction->setData(13);
    m_mediumFontAction->setStatusTip("设置字体大小为 13pt");
    sizeGroup->addAction(m_mediumFontAction);

    m_largeFontAction = new QAction("大字体", this);
    m_largeFontAction->setCheckable(true);
    m_largeFontAction->setData(18);
    m_largeFontAction->setStatusTip("设置字体大小为 18pt");
    sizeGroup->addAction(m_largeFontAction);

    connect(sizeGroup, &QActionGroup::triggered,
            this, &TextEditor::onFontSizeChanged);

    // 格式操作：可勾选项
    m_wordWrapAction = new QAction("自动换行(&W)", this);
    m_wordWrapAction->setCheckable(true);
    m_wordWrapAction->setChecked(true);
    m_wordWrapAction->setStatusTip("切换自动换行");
    connect(m_wordWrapAction, &QAction::toggled,
            this, &TextEditor::toggleWordWrap);

    m_showLineNumbersAction =
        new QAction("显示行号(&L)", this);
    m_showLineNumbersAction->setCheckable(true);
    m_showLineNumbersAction->setChecked(false);
    m_showLineNumbersAction->setStatusTip(
        "切换行号显示（演示 setCheckable）");
    connect(m_showLineNumbersAction, &QAction::toggled,
            [this](bool checked) {
                statusBar()->showMessage(
                    checked ? "行号已开启（演示）"
                            : "行号已关闭（演示）",
                    2000);
            });

    // ================================================================
    // 帮助操作
    // ================================================================
    m_aboutAction = new QAction("关于(&A)", this);
    m_aboutAction->setStatusTip("关于本应用");
    connect(m_aboutAction, &QAction::triggered, [this]() {
        statusBar()->showMessage(
            "菜单系统演示 v1.0 — Qt 6.9.1 "
            "QMenuBar/QMenu/QAction", 4000);
    });
}

/// @brief 初始化菜单栏
void TextEditor::initMenuBar()
{
    // ================================================================
    // 文件菜单
    // ================================================================
    auto *fileMenu = menuBar()->addMenu("文件(&F)");
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_quitAction);

    // ================================================================
    // 编辑菜单
    // ================================================================
    auto *editMenu = menuBar()->addMenu("编辑(&E)");
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editMenu->addSeparator();
    editMenu->addAction(m_cutAction);
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(m_selectAllAction);

    // ================================================================
    // 格式菜单（含子菜单）
    // ================================================================
    auto *formatMenu = menuBar()->addMenu("格式(&O)");

    // 子菜单：字体大小
    auto *fontSizeMenu =
        formatMenu->addMenu("字体大小(&S)");
    fontSizeMenu->addAction(m_smallFontAction);
    fontSizeMenu->addAction(m_mediumFontAction);
    fontSizeMenu->addAction(m_largeFontAction);

    formatMenu->addSeparator();
    formatMenu->addAction(m_wordWrapAction);
    formatMenu->addAction(m_showLineNumbersAction);

    // ================================================================
    // 帮助菜单
    // ================================================================
    auto *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction(m_aboutAction);
}

/// @brief 初始化工具栏（共享菜单的 QAction）
void TextEditor::initToolBars()
{
    // 文件工具栏
    auto *fileToolBar = addToolBar("文件");
    fileToolBar->addAction(m_newAction);
    fileToolBar->addAction(m_openAction);
    fileToolBar->addAction(m_saveAction);

    // 编辑工具栏
    auto *editToolBar = addToolBar("编辑");
    editToolBar->addAction(m_undoAction);
    editToolBar->addAction(m_redoAction);
    editToolBar->addSeparator();
    editToolBar->addAction(m_cutAction);
    editToolBar->addAction(m_copyAction);
    editToolBar->addAction(m_pasteAction);
}

/// @brief 连接编辑器信号以动态更新菜单项状态
void TextEditor::connectEditorSignals()
{
    // 撤销/重做可用状态
    connect(m_editor, &QPlainTextEdit::undoAvailable,
            m_undoAction, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::redoAvailable,
            m_redoAction, &QAction::setEnabled);

    // 复制/剪切可用状态
    connect(m_editor, &QPlainTextEdit::copyAvailable,
            m_cutAction, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::copyAvailable,
            m_copyAction, &QAction::setEnabled);
}

/// @brief 字体大小变更
void TextEditor::onFontSizeChanged(QAction *action)
{
    int size = action->data().toInt();
    QFont font = m_editor->font();
    font.setPointSize(size);
    m_editor->setFont(font);

    statusBar()->showMessage(
        QString("字体大小已设置为 %1pt").arg(size),
        2000);
}

/// @brief 切换自动换行
void TextEditor::toggleWordWrap(bool enabled)
{
    if (enabled) {
        m_editor->setLineWrapMode(
            QPlainTextEdit::WidgetWidth);
        statusBar()->showMessage("自动换行已开启", 2000);
    } else {
        m_editor->setLineWrapMode(
            QPlainTextEdit::NoWrap);
        statusBar()->showMessage("自动换行已关闭", 2000);
    }
}
