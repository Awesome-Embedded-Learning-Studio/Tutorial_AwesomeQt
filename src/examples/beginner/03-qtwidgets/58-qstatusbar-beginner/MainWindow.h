// QtWidgets 入门示例 58: QStatusBar 状态栏
// 演示：showMessage 临时消息
//       addWidget / addPermanentWidget 嵌入控件
//       QProgressBar 嵌入状态栏
//       clearMessage 与消息优先级

#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProgressBar>

// ============================================================================
// MainWindow: 演示状态栏的完整配置
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // 工具栏
    // ====================================================================
    void setupToolBar();

    // ====================================================================
    // 状态栏
    // ====================================================================
    void setupStatusBar();

    // ====================================================================
    // 模拟加载：演示进度条嵌入状态栏
    // ====================================================================
    void startSimulatedLoad();

    // ====================================================================
    // 状态更新
    // ====================================================================
    void updateCursorPosition();

    void updateCharCount();

private:
    QPlainTextEdit *m_editor = nullptr;
    QLabel *m_positionLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_charCountLabel = nullptr;
};
