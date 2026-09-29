// QtWidgets 入门示例 26: QKeySequenceEdit 快捷键录入控件
// 演示：keySequenceChanged 信号获取录入结果
//       setKeySequence 设置默认快捷键
//       与 QAction::setShortcut 结合的完整热键配置流程
//       冲突检测 + 日志记录

#include "shortcut_config_window.h"

#include <QAction>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTime>
#include <QVBoxLayout>

// ============================================================================
// ShortcutConfigWindow: 快捷键配置面板 + 菜单栏 + 触发日志
// ============================================================================
ShortcutConfigWindow::ShortcutConfigWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(
        "QKeySequenceEdit 综合演示 — 快捷键配置面板");
    resize(600, 520);
    createActions();
    createMenuBar();
    createCentralWidget();
}

/// @brief 创建 QAction 并设置默认快捷键
void ShortcutConfigWindow::createActions()
{
    m_newAction = new QAction("新建", this);
    m_newAction->setShortcut(QKeySequence::New);
    connect(m_newAction, &QAction::triggered,
            this, [this]() { onActionTriggered("新建"); });

    m_saveAction = new QAction("保存", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered,
            this, [this]() { onActionTriggered("保存"); });

    m_closeAction = new QAction("关闭", this);
    m_closeAction->setShortcut(QKeySequence::Close);
    connect(m_closeAction, &QAction::triggered,
            this, [this]() { onActionTriggered("关闭"); });

    m_allActions = {m_newAction, m_saveAction, m_closeAction};
}

/// @brief 创建菜单栏
void ShortcutConfigWindow::createMenuBar()
{
    auto *menuBar = this->menuBar();
    auto *fileMenu = menuBar->addMenu("操作(&A)");
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_closeAction);
}

/// @brief 创建中央窗口: 配置面板 + 日志区域
void ShortcutConfigWindow::createCentralWidget()
{
    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 16, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("快捷键配置面板");
    titleLabel->setFont(QFont("Arial", 15, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    auto *descLabel = new QLabel(
        "点击输入框后按下按键组合即可修改快捷键。"
        "按 Escape 或 Backspace 清除快捷键。");
    descLabel->setStyleSheet("color: #666; font-size: 12px;");
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);

    // ================================================================
    // 快捷键配置区域: QFormLayout + QKeySequenceEdit
    // ================================================================
    auto *configGroup = new QGroupBox("快捷键配置");
    auto *formLayout = new QFormLayout(configGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(16, 24, 16, 16);

    // 新建
    m_newEdit = new QKeySequenceEdit();
    m_newEdit->setKeySequence(m_newAction->shortcut());
    formLayout->addRow("新建:", m_newEdit);

    // 保存
    m_saveEdit = new QKeySequenceEdit();
    m_saveEdit->setKeySequence(m_saveAction->shortcut());
    formLayout->addRow("保存:", m_saveEdit);

    // 关闭
    m_closeEdit = new QKeySequenceEdit();
    m_closeEdit->setKeySequence(m_closeAction->shortcut());
    formLayout->addRow("关闭:", m_closeEdit);

    mainLayout->addWidget(configGroup);

    // ---- 重置按钮 ----
    auto *resetBtn = new QPushButton("恢复默认快捷键");
    resetBtn->setAutoDefault(false);
    connect(resetBtn, &QPushButton::clicked, this,
            &ShortcutConfigWindow::resetDefaults);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(resetBtn);
    mainLayout->addLayout(btnLayout);

    // ================================================================
    // 日志区域: 显示动作触发记录
    // ================================================================
    auto *logGroup = new QGroupBox("触发日志");
    auto *logLayout = new QVBoxLayout(logGroup);
    logLayout->setContentsMargins(12, 20, 12, 12);

    m_logView = new QPlainTextEdit();
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(50);
    m_logView->setPlaceholderText(
        "通过菜单或快捷键触发操作时，记录会显示在这里...");

    QFont monoFont("Monospace", 9);
    monoFont.setStyleHint(QFont::Monospace);
    m_logView->setFont(monoFont);

    logLayout->addWidget(m_logView);

    mainLayout->addWidget(logGroup, 1);

    setCentralWidget(centralWidget);

    // ---- 信号连接 ----
    // 注意: 在 setKeySequence() 之后连接信号，避免初始化时触发

    m_editActionMap = {
        {m_newEdit, m_newAction},
        {m_saveEdit, m_saveAction},
        {m_closeEdit, m_closeAction},
    };

    connect(m_newEdit, &QKeySequenceEdit::keySequenceChanged,
            this, [this](const QKeySequence &seq) {
                applyShortcut(m_newEdit, m_newAction, seq);
            });

    connect(m_saveEdit, &QKeySequenceEdit::keySequenceChanged,
            this, [this](const QKeySequence &seq) {
                applyShortcut(m_saveEdit, m_saveAction, seq);
            });

    connect(m_closeEdit, &QKeySequenceEdit::keySequenceChanged,
            this, [this](const QKeySequence &seq) {
                applyShortcut(m_closeEdit, m_closeAction, seq);
            });
}

/// @brief 应用新快捷键到 QAction，带冲突检测
void ShortcutConfigWindow::applyShortcut(QKeySequenceEdit *edit,
                                           QAction *action,
                                           const QKeySequence &newSeq)
{
    // 检查快捷键冲突（排除当前动作自身）
    if (isShortcutConflict(newSeq, action)) {
        QMessageBox::warning(
            this, "快捷键冲突",
            QString("快捷键 \"%1\" 已被其他操作使用，"
                    "请选择其他快捷键。")
                .arg(newSeq.toString()));

        // 恢复原来的快捷键
        edit->blockSignals(true);
        edit->setKeySequence(action->shortcut());
        edit->blockSignals(false);
        return;
    }

    // 应用新快捷键
    action->setShortcut(newSeq);

    appendLog(QString("[配置] %1 快捷键已更新为: %2")
        .arg(action->text())
        .arg(newSeq.isEmpty()
             ? QString("(无)")
             : newSeq.toString()));
}

/// @brief 检查快捷键是否与其他动作冲突
bool ShortcutConfigWindow::isShortcutConflict(const QKeySequence &seq,
                                                QAction *exclude) const
{
    if (seq.isEmpty()) {
        return false;
    }

    for (auto *action : m_allActions) {
        if (action == exclude) {
            continue;
        }
        if (action->shortcut() == seq) {
            return true;
        }
    }
    return false;
}

/// @brief QAction 被触发时的日志记录
void ShortcutConfigWindow::onActionTriggered(const QString &actionName)
{
    appendLog(QString("[触发] %1").arg(actionName));
}

/// @brief 恢复所有快捷键为默认值
void ShortcutConfigWindow::resetDefaults()
{
    // 断开信号避免逐个触发
    m_newEdit->blockSignals(true);
    m_saveEdit->blockSignals(true);
    m_closeEdit->blockSignals(true);

    m_newAction->setShortcut(QKeySequence::New);
    m_newEdit->setKeySequence(QKeySequence::New);

    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveEdit->setKeySequence(QKeySequence::Save);

    m_closeAction->setShortcut(QKeySequence::Close);
    m_closeEdit->setKeySequence(QKeySequence::Close);

    // 恢复信号
    m_newEdit->blockSignals(false);
    m_saveEdit->blockSignals(false);
    m_closeEdit->blockSignals(false);

    appendLog("[配置] 所有快捷键已恢复为默认值");
}

/// @brief 追加日志并自动滚动到底部
void ShortcutConfigWindow::appendLog(const QString &text)
{
    QString timestamp = QTime::currentTime().toString("HH:mm:ss");
    m_logView->appendPlainText(
        QString("[%1] %2").arg(timestamp).arg(text));

    QTextCursor cursor = m_logView->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logView->setTextCursor(cursor);
}
