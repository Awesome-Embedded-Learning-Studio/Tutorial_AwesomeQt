/// @file    shortcut_conflict_panel.cpp
/// @brief   ShortcutConflictPanel 类实现——多快捷键配置与冲突检测。
///
/// 对应教程：进阶层 03-QtWidgets/26-QKeySequenceEdit 进阶。

#include "shortcut_conflict_panel.h"

#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 系统保留快捷键列表
// ─────────────────────────────────────────────────────────────────────────────

/// 已知的系统保留快捷键（不完全列表，按需补充）。
static const QList<QKeySequence> kSystemReservedShortcuts = {
    QKeySequence(QStringLiteral("Ctrl+Space")),    // 输入法切换
    QKeySequence(QStringLiteral("Meta+Space")),    // Spotlight (macOS)
    QKeySequence(QStringLiteral("Meta+L")),        // 锁屏 (Windows)
    QKeySequence(QStringLiteral("Meta+D")),        // 显示桌面 (Windows)
};

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ShortcutConflictPanel::ShortcutConflictPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 标题
    auto* titleLabel = new QLabel(
        QStringLiteral("快捷键冲突检测面板\n"
                       "为每个操作设置快捷键，系统会自动检测冲突和系统保留键。"));
    titleLabel->setWordWrap(true);

    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(8);

    setupEntries();

    // 为每个条目创建一行 UI
    for (int i = 0; i < m_entries.size(); ++i) {
        auto* row = new QHBoxLayout;

        // 操作名称标签
        auto* nameLabel = new QLabel(m_entries[i].actionName);
        nameLabel->setFixedWidth(160);

        // 快捷键录入控件
        m_entries[i].sequenceEdit = new QKeySequenceEdit;

        // 状态标签：显示正常/冲突/系统保留
        m_entries[i].statusLabel = new QLabel;
        m_entries[i].statusLabel->setFixedWidth(260);

        row->addWidget(nameLabel);
        row->addWidget(m_entries[i].sequenceEdit);
        row->addWidget(m_entries[i].statusLabel, 1);

        mainLayout->addLayout(row);

        // 捕获索引用于 lambda——必须用值捕获 i，不能引用
        const int entryIndex = i;

        // editingFinished：录制完成或焦点丢失时触发
        // 用于检测 keySequence 是否为空（焦点丢失导致录制中断）
        connect(m_entries[i].sequenceEdit, &QKeySequenceEdit::editingFinished,
                this, [this, entryIndex]() { onEditingFinished(entryIndex); });

        // keySequenceChanged：快捷键值发生变化时重新检测冲突
        connect(m_entries[i].sequenceEdit, &QKeySequenceEdit::keySequenceChanged,
                this, [this]() { refreshAllStatus(); });

        // 记住录入前的有效值
        m_entries[i].previousSequence = m_entries[i].sequenceEdit->keySequence();
    }

    mainLayout->addStretch();

    // 初始刷新状态
    refreshAllStatus();

    setWindowTitle(QStringLiteral("QKeySequenceEdit Advanced - Shortcut Conflict Panel"));
    resize(620, 420);
}

// ─────────────────────────────────────────────────────────────────────────────
// 初始化操作列表
// ─────────────────────────────────────────────────────────────────────────────

void ShortcutConflictPanel::setupEntries()
{
    // 预设十个操作的名称和默认快捷键
    const QList<QPair<QString, QKeySequence>> kDefaultBindings = {
        {QStringLiteral("新建文件"),   QKeySequence(QKeySequence::New)},
        {QStringLiteral("打开文件"),   QKeySequence(QKeySequence::Open)},
        {QStringLiteral("保存文件"),   QKeySequence(QKeySequence::Save)},
        {QStringLiteral("撤销"),       QKeySequence(QKeySequence::Undo)},
        {QStringLiteral("重做"),       QKeySequence(QKeySequence::Redo)},
        {QStringLiteral("复制"),       QKeySequence(QKeySequence::Copy)},
        {QStringLiteral("粘贴"),       QKeySequence(QKeySequence::Paste)},
        {QStringLiteral("剪切"),       QKeySequence(QKeySequence::Cut)},
        {QStringLiteral("全选"),       QKeySequence(QKeySequence::SelectAll)},
        {QStringLiteral("查找"),       QKeySequence(QKeySequence::Find)},
    };

    for (const auto& binding : kDefaultBindings) {
        ShortcutEntry entry;
        entry.actionName = binding.first;
        // sequenceEdit 在构造函数中创建（需要 this 作为 parent）
        entry.sequenceEdit = nullptr;
        entry.statusLabel = nullptr;
        entry.previousSequence = binding.second;
        m_entries.append(entry);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 冲突检测
// ─────────────────────────────────────────────────────────────────────────────

QString ShortcutConflictPanel::findConflict(const QKeySequence& sequence,
                                             int excludeIndex) const
{
    if (sequence.isEmpty()) {
        return QString();
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        if (i == excludeIndex) {
            continue;
        }
        const QKeySequence existing = m_entries[i].sequenceEdit->keySequence();
        // 精确匹配比较——Ctrl+S 和 Ctrl+Shift+S 不算冲突
        if (!existing.isEmpty() && existing == sequence) {
            return m_entries[i].actionName;
        }
    }
    return QString();
}

bool ShortcutConflictPanel::isSystemReserved(const QKeySequence& sequence)
{
    if (sequence.isEmpty()) {
        return false;
    }
    return kSystemReservedShortcuts.contains(sequence);
}

// ─────────────────────────────────────────────────────────────────────────────
// 状态刷新
// ─────────────────────────────────────────────────────────────────────────────

void ShortcutConflictPanel::refreshAllStatus()
{
    for (int i = 0; i < m_entries.size(); ++i) {
        const QKeySequence seq = m_entries[i].sequenceEdit->keySequence();

        if (seq.isEmpty()) {
            m_entries[i].statusLabel->setText(QStringLiteral("未设置"));
            m_entries[i].statusLabel->setStyleSheet(
                QStringLiteral("color: gray;"));
            continue;
        }

        // 三重检查：系统保留 -> 冲突 -> 正常
        if (isSystemReserved(seq)) {
            m_entries[i].statusLabel->setText(
                QStringLiteral("⚠ 可能被系统拦截"));
            m_entries[i].statusLabel->setStyleSheet(
                QStringLiteral("color: orange; font-weight: bold;"));
            continue;
        }

        const QString conflictWith = findConflict(seq, i);
        if (!conflictWith.isEmpty()) {
            m_entries[i].statusLabel->setText(
                QStringLiteral("⚠ 冲突：%1").arg(conflictWith));
            m_entries[i].statusLabel->setStyleSheet(
                QStringLiteral("color: red; font-weight: bold;"));
            continue;
        }

        // 显示当前快捷键的 NativeText 表示
        m_entries[i].statusLabel->setText(
            QStringLiteral("✓ 已设置 (%1)").arg(seq.toString()));
        m_entries[i].statusLabel->setStyleSheet(
            QStringLiteral("color: green;"));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// editingFinished 处理
// ─────────────────────────────────────────────────────────────────────────────

void ShortcutConflictPanel::onEditingFinished(int entryIndex)
{
    auto& entry = m_entries[entryIndex];
    const QKeySequence current = entry.sequenceEdit->keySequence();

    // 焦点丢失时 keySequence() 可能为空——恢复之前的有效值
    if (current.isEmpty()) {
        entry.sequenceEdit->setKeySequence(entry.previousSequence);
    } else {
        // 录制成功，更新 previousSequence
        entry.previousSequence = current;
    }

    refreshAllStatus();
}
