// QtWidgets 入门示例 23: QTextEdit 富文本多行编辑器
// 演示：纯文本 vs 富文本模式切换
//       setHtml / toHtml / toPlainText 内容读写
//       光标操作：QTextCursor 插入/选中/格式化
//       document()->setModified() 追踪修改状态

#include <QWidget>

class QTextEdit;
class QLabel;
class QTextCharFormat;
class QCloseEvent;

// ============================================================================
// MiniEditor: 迷你文本编辑器，展示 QTextEdit 的核心能力
// ============================================================================
class MiniEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MiniEditor(QWidget *parent = nullptr);

protected:
    /// @brief 关闭事件: 检查文档修改状态
    void closeEvent(QCloseEvent *event) override;

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 对当前选中文字应用字符格式
    void applyFormat(const QTextCharFormat &format);

    /// @brief 在当前光标位置插入一段蓝色加粗的标题文字
    void insertBlueTitle();

    /// @brief 更新状态栏的字符数和行数
    void updateStatusBar();

    /// @brief 文档修改状态变化: 更新标签和窗口标题
    void onModificationChanged(bool modified);

private:
    QTextEdit *m_editor = nullptr;
    QLabel *m_charCountLabel = nullptr;
    QLabel *m_lineCountLabel = nullptr;
    QLabel *m_modifiedLabel = nullptr;
};
