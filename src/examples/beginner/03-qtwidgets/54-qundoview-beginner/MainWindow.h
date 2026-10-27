#include <QMainWindow>
#include <QUndoStack>

class QLabel;
class QPlainTextEdit;
class QUndoStack;
class QUndoView;

// ============================================================================
// MainWindow: 文档编辑器（带撤销历史面板）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面布局
    void initUi();

    /// @brief 初始化工具栏（撤销、重做）
    void initToolBar();

    /// @brief 获取编辑器最后一行的起始位置
    int lastLineStart() const;

    /// @brief 获取编辑器全部文本长度
    int totalLength() const;

    /// @brief 获取最后一行的文本
    QString lastLineText() const;

    /// @brief 插入示例文本
    void onInsertSampleText();

    /// @brief 追加一行
    void onAppendLine();

    /// @brief 删除末尾行
    void onDeleteLastLine();

    /// @brief 清空全部文本
    void onClearAll();

    /// @brief 保存文档（标记 clean state）
    void onSave();

    /// @brief clean 状态变化时更新标题和状态标签
    void onCleanChanged(bool clean);

private:
    QUndoStack *m_undoStack = new QUndoStack(this);
    QPlainTextEdit *m_editor = nullptr;
    QUndoView *m_undoView = nullptr;
    QLabel *m_statusLabel = nullptr;
};
