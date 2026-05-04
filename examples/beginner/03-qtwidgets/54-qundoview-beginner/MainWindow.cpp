#include "MainWindow.h"

#include "TextEditCommand.h"

#include <QAction>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStyle>
#include <QToolBar>
#include <QUndoStack>
#include <QUndoView>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QUndoView 综合演示 — 文档编辑器");
    resize(900, 560);
    initUi();
    initToolBar();
}

/// @brief 初始化界面布局
void MainWindow::initUi()
{
    auto *splitter = new QSplitter(Qt::Horizontal);

    // 左侧：编辑器 + 操作按钮
    auto *leftPanel = new QWidget;
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(4, 4, 4, 4);

    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText(
        "在此输入文本，或使用下方按钮执行操作...");
    m_editor->setReadOnly(true);
    leftLayout->addWidget(m_editor, 1);

    // 操作按钮栏
    auto *buttonBar = new QWidget;
    auto *buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(0, 4, 0, 0);

    auto *btnInsert = new QPushButton("插入示例文本");
    auto *btnAppend = new QPushButton("追加一行");
    auto *btnDelete = new QPushButton("删除末尾行");
    auto *btnClear = new QPushButton("清空全部");
    auto *btnSave = new QPushButton("保存");

    buttonLayout->addWidget(btnInsert);
    buttonLayout->addWidget(btnAppend);
    buttonLayout->addWidget(btnDelete);
    buttonLayout->addWidget(btnClear);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnSave);

    leftLayout->addWidget(buttonBar);

    // 右侧：撤销历史视图
    auto *rightPanel = new QWidget;
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(4, 4, 4, 4);

    auto *historyLabel = new QLabel("操作历史");
    historyLabel->setStyleSheet(
        "font-weight: bold; font-size: 13px; color: #333;");
    rightLayout->addWidget(historyLabel);

    m_undoView = new QUndoView;
    m_undoView->setStack(m_undoStack);
    m_undoView->setCleanIcon(
        style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_undoView->setEmptyLabel("(无操作)");
    rightLayout->addWidget(m_undoView, 1);

    // 状态标签
    m_statusLabel = new QLabel("已保存");
    m_statusLabel->setStyleSheet(
        "padding: 4px 8px; font-size: 12px; color: #666;");
    rightLayout->addWidget(m_statusLabel);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    // 连接按钮信号
    connect(btnInsert, &QPushButton::clicked,
            this, &MainWindow::onInsertSampleText);
    connect(btnAppend, &QPushButton::clicked,
            this, &MainWindow::onAppendLine);
    connect(btnDelete, &QPushButton::clicked,
            this, &MainWindow::onDeleteLastLine);
    connect(btnClear, &QPushButton::clicked,
            this, &MainWindow::onClearAll);
    connect(btnSave, &QPushButton::clicked,
            this, &MainWindow::onSave);

    // 监听 undoStack 的 clean 状态变化
    connect(m_undoStack, &QUndoStack::cleanChanged,
            this, &MainWindow::onCleanChanged);
}

/// @brief 初始化工具栏（撤销、重做）
void MainWindow::initToolBar()
{
    auto *toolBar = addToolBar("编辑");
    toolBar->setMovable(false);

    // 使用 QUndoStack 创建标准撤销/重做 Action
    auto *undoAction = m_undoStack->createUndoAction(this);
    undoAction->setIcon(
        style()->standardIcon(QStyle::SP_ArrowBack));
    undoAction->setShortcut(QKeySequence::Undo);

    auto *redoAction = m_undoStack->createRedoAction(this);
    redoAction->setIcon(
        style()->standardIcon(QStyle::SP_ArrowForward));
    redoAction->setShortcut(QKeySequence::Redo);

    toolBar->addAction(undoAction);
    toolBar->addAction(redoAction);
    toolBar->addSeparator();
}

/// @brief 获取编辑器最后一行的起始位置
int MainWindow::lastLineStart() const
{
    QString text = m_editor->toPlainText();
    int index = text.lastIndexOf('\n');
    return index < 0 ? 0 : index + 1;
}

/// @brief 获取编辑器全部文本长度
int MainWindow::totalLength() const
{
    return m_editor->toPlainText().length();
}

/// @brief 获取最后一行的文本
QString MainWindow::lastLineText() const
{
    QString text = m_editor->toPlainText();
    int index = text.lastIndexOf('\n');
    return index < 0 ? text : text.mid(index + 1);
}

/// @brief 插入示例文本
void MainWindow::onInsertSampleText()
{
    static int counter = 0;
    ++counter;

    QString sample = QString(
        "这是第 %1 次插入的示例文本。\n"
        "Qt 的 QUndoFramework 非常强大。\n"
        "QUndoView 让操作历史可视化。")
        .arg(counter);

    auto *cmd = new TextEditCommand(
        m_editor, "", sample, totalLength());
    m_undoStack->push(cmd);
}

/// @brief 追加一行
void MainWindow::onAppendLine()
{
    static const QVector<QString> lines = {
        "春眠不觉晓，处处闻啼鸟。",
        "夜来风雨声，花落知多少。",
        "白日依山尽，黄河入海流。",
        "欲穷千里目，更上一层楼。",
        "锄禾日当午，汗滴禾下土。",
        "谁知盘中餐，粒粒皆辛苦。",
    };

    static int lineIdx = 0;
    QString line = lines[lineIdx % lines.size()];
    ++lineIdx;

    // 需要在前面加换行符（如果编辑器已有内容）
    QString text = m_editor->toPlainText();
    QString toInsert = text.isEmpty()
                           ? line
                           : "\n" + line;

    auto *cmd = new TextEditCommand(
        m_editor, "", toInsert, totalLength());
    m_undoStack->push(cmd);
}

/// @brief 删除末尾行
void MainWindow::onDeleteLastLine()
{
    QString text = m_editor->toPlainText();
    if (text.isEmpty()) {
        return;
    }

    int startPos = lastLineStart();
    QString removed = lastLineText();

    // 如果前面有换行符，也一并删除
    if (startPos > 0) {
        --startPos;
        removed = "\n" + removed;
    }

    auto *cmd = new TextEditCommand(
        m_editor, removed, "", startPos);
    m_undoStack->push(cmd);
}

/// @brief 清空全部文本
void MainWindow::onClearAll()
{
    QString text = m_editor->toPlainText();
    if (text.isEmpty()) {
        return;
    }

    auto *cmd = new TextEditCommand(
        m_editor, text, "", 0);
    m_undoStack->push(cmd);
}

/// @brief 保存文档（标记 clean state）
void MainWindow::onSave()
{
    m_undoStack->setClean();
}

/// @brief clean 状态变化时更新标题和状态标签
void MainWindow::onCleanChanged(bool clean)
{
    if (clean) {
        setWindowTitle(
            "QUndoView 综合演示 — 文档编辑器");
        m_statusLabel->setText("已保存");
        m_statusLabel->setStyleSheet(
            "padding: 4px 8px; font-size: 12px;"
            "color: #2E7D32;");
    } else {
        setWindowTitle(
            "QUndoView 综合演示 — 文档编辑器 *");
        m_statusLabel->setText("未保存");
        m_statusLabel->setStyleSheet(
            "padding: 4px 8px; font-size: 12px;"
            "color: #C62828;");
    }
}
