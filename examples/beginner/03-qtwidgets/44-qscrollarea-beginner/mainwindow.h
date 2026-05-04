// QtWidgets 入门示例 44: QScrollArea 滚动区域容器
// 演示：setWidget 设置被滚动的内容控件
//       setWidgetResizable(true) 内容自适应宽度
//       动态添加内容后自动滚动到底部
//       自定义滚动条样式 QSS

#include <QMainWindow>

class QScrollArea;
class QVBoxLayout;
class QWidget;
class QLineEdit;
class QLabel;

// ============================================================================
// MainWindow: QScrollArea 综合演示主窗口（消息日志查看器）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 发送用户输入的消息
    void onSendMessage();

    /// @brief 批量添加 20 条测试消息
    void onAddBatch();

    /// @brief 追加一条用户消息
    void appendUserMessage(const QString &text);

    /// @brief 追加一条系统消息
    void appendSystemMessage(const QString &text);

    /// @brief 清空所有消息标签
    void onClearMessages();

    /// @brief 滚动到底部（使用 ensureWidgetVisible）
    void scrollToBottom();

    /// @brief 更新消息计数标签
    void updateCountLabel();

private:
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_contentWidget = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QWidget *m_anchor = nullptr;
    QLineEdit *m_inputEdit = nullptr;
    QLabel *m_countLabel = nullptr;
    int m_messageCount = 0;
};
