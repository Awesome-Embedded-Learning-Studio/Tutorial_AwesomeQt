/// @file    undo_editor.h
/// @brief   演示 QUndoView 与 QUndoStack 完整撤销重做系统。
///
/// 对应教程：进阶层 03-QtWidgets/54-QUndoView 高级用法。
/// 本示例通过 QPlainTextEdit + QUndoStack + 自定义 QUndoCommand 子类，
/// 实现完整的文本编辑撤销/重做系统，并使用 QUndoView 作为可停靠的
/// 历史命令面板。

#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QUndoCommand>
#include <QUndoStack>
#include <QUndoView>

/// @brief 插入文本命令，记录光标位置和插入的文本内容。
///
/// 每次用户输入一段文本时，创建此命令压入 QUndoStack。
/// undo() 删除插入的文本，redo() 重新插入。
class InsertTextCommand : public QUndoCommand
{
public:
    /// @brief 构造函数。
    /// @param[in] editor 关联的文本编辑器
    /// @param[in] position 插入位置（字符偏移）
    /// @param[in] text 插入的文本
    /// @param[in] parent 父命令（用于宏命令组合）
    /// @note 命令持有编辑器指针但不拥有它，编辑器生命周期由主窗口管理。
    InsertTextCommand(QPlainTextEdit* editor, int position,
                      const QString& text, QUndoCommand* parent = nullptr);

    /// @brief 撤销插入：删除对应位置的文本。
    void undo() override;

    /// @brief 重做插入：在对应位置重新插入文本。
    void redo() override;

private:
    QPlainTextEdit* m_editor; ///< 关联的文本编辑器
    int m_position;           ///< 插入位置
    QString m_text;           ///< 插入的文本
};

/// @brief 删除文本命令，记录被删除的文本及其原始位置。
///
/// undo() 恢复被删除的文本，redo() 再次删除。
class DeleteTextCommand : public QUndoCommand
{
public:
    /// @brief 构造函数。
    /// @param[in] editor 关联的文本编辑器
    /// @param[in] position 删除起始位置
    /// @param[in] text 被删除的文本
    /// @param[in] parent 父命令
    DeleteTextCommand(QPlainTextEdit* editor, int position,
                      const QString& text, QUndoCommand* parent = nullptr);

    /// @brief 撤销删除：恢复文本。
    void undo() override;

    /// @brief 重做删除：再次移除文本。
    void redo() override;

private:
    QPlainTextEdit* m_editor; ///< 关联的文本编辑器
    int m_position;           ///< 删除起始位置
    QString m_text;           ///< 被删除的文本
};

/// @brief 主窗口，集成文本编辑器、撤销栈和历史视图面板。
class UndoEditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，组装所有 UI 组件和信号槽连接。
    /// @param[in] parent 父控件
    explicit UndoEditorWindow(QWidget* parent = nullptr);

private:
    /// @brief 创建菜单栏和工具栏。
    void createActions();

    /// @brief 创建可停靠的 QUndoView 面板。
    void createUndoPanel();

    /// @brief 连接 QTextDocument::contentsChange 信号。
    void connectSignals();

    /// @brief 处理文档内容变化，创建对应的 QUndoCommand。
    /// @param[in] position 变化起始位置
    /// @param[in] charsRemoved 被删除的字符数
    /// @param[in] charsAdded 新增的字符数
    /// @note 使用 contentsChange 信号获取精确增量，区分插入和删除操作。
    void onContentsChange(int position, int charsRemoved, int charsAdded);

    QPlainTextEdit* m_editor;   ///< 文本编辑器
    QUndoStack* m_undoStack;    ///< 撤销/重做栈
    QUndoView* m_undoView;      ///< 历史命令可视化面板
    bool m_isUndoing;           ///< 标记当前是否正在执行撤销/重做操作
};
