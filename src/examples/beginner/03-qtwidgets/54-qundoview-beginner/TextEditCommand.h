// QtWidgets 入门示例 54: QUndoView 撤销历史视图
// 演示：与 QUndoStack 绑定显示操作历史
//       setCleanIcon 标记保存点
//       点击历史条目实现时间线跳转
//       文档编辑器中的完整撤销重做系统

#include <QUndoCommand>

class QPlainTextEdit;

// ============================================================================
// TextEditCommand: 封装文本插入/删除操作的撤销命令
// ============================================================================
class TextEditCommand : public QUndoCommand
{
public:
    /// @brief 构造文本编辑命令
    /// @param editor 目标编辑器
    /// @param removedText 被删除的文本
    /// @param insertedText 被插入的文本
    /// @param position 操作发生的起始位置
    /// @param parent 父命令
    TextEditCommand(QPlainTextEdit *editor,
                    const QString &removedText,
                    const QString &insertedText,
                    int position,
                    QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    /// @brief 截断过长文本用于显示
    static QString truncated(const QString &text, int maxLen);

    QPlainTextEdit *m_editor;
    QString m_removedText;
    QString m_insertedText;
    int m_position;
};
