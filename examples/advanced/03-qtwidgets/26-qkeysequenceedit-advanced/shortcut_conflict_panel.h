/// @file    shortcut_conflict_panel.h
/// @brief   ShortcutConflictPanel 类声明——多快捷键配置与冲突检测演示。
///
/// 对应教程：进阶层 03-QtWidgets/26-QKeySequenceEdit 进阶。

#pragma once

#include <QKeySequence>
#include <QList>
#include <QWidget>

class QLabel;
class QKeySequenceEdit;

/// 快捷键冲突检测演示面板。
///
/// 展示三个核心知识点：
/// - 多个 QKeySequenceEdit 实例对应不同操作，editingFinished 捕获完整序列
/// - 冲突检测：遍历所有操作的 shortcuts() 比较是否重复
/// - 系统保留快捷键列表检查，冲突时显示警告标签
class ShortcutConflictPanel : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化快捷键配置面板与检测逻辑。
    /// @param[in] parent 父控件指针。
    explicit ShortcutConflictPanel(QWidget* parent = nullptr);

private:
    /// @brief 描述一个快捷键配置项的内部数据结构。
    struct ShortcutEntry
    {
        QString           actionName;      // 操作名称
        QKeySequenceEdit* sequenceEdit;     // 快捷键录入控件
        QLabel*           statusLabel;      // 状态标签（正常/冲突/系统保留）
        QKeySequence      previousSequence; // 录制前的有效值，用于恢复
    };

    /// @brief 初始化操作列表并创建对应的 UI 行。
    void setupEntries();

    /// @brief 检测给定快捷键是否与其他操作冲突。
    /// @param[in] sequence 待检测的快捷键。
    /// @param[in] excludeIndex 排除自身的索引（避免自己和自己比）。
    /// @return 冲突的操作名，无冲突返回空字符串。
    QString findConflict(const QKeySequence& sequence, int excludeIndex) const;

    /// @brief 检测给定快捷键是否在系统保留列表中。
    /// @param[in] sequence 待检测的快捷键。
    /// @return true 表示可能被系统拦截。
    static bool isSystemReserved(const QKeySequence& sequence);

    /// @brief 检测所有快捷键并更新状态标签。
    void refreshAllStatus();

    /// @brief editingFinished 槽：处理录制完成或焦点丢失。
    /// @param[in] entryIndex 对应的 ShortcutEntry 索引。
    void onEditingFinished(int entryIndex);

private:
    QList<ShortcutEntry> m_entries;  // 所有快捷键配置项
};
