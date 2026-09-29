// QtWidgets 入门示例 25: QTextBrowser 只读富文本浏览器
// 演示：显示 HTML / Markdown 格式文档
//       setSource 加载本地 HTML 文件
//       anchorClicked 信号处理链接点击
//       历史导航: backward / forward / home

#include <QWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>

// ============================================================================
// HelpBrowser: 迷你帮助文档浏览器
// ============================================================================
class HelpBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit HelpBrowser(QWidget *parent = nullptr);
    ~HelpBrowser() override;

private:
    /// @brief 将帮助文档写入临时目录，供 setSource() 加载
    void createHelpFiles();

    /// @brief 将内容写入文件
    static void write_file(const QString &path, const QString &content);

    /// @brief 初始化界面
    void initUi();

    /// @brief 处理链接点击: 外部链接用系统浏览器打开
    void onAnchorClicked(const QUrl &url);

private:
    QTextBrowser *m_browser = nullptr;
    QPushButton *m_backBtn = nullptr;
    QPushButton *m_forwardBtn = nullptr;
    QPushButton *m_homeBtn = nullptr;
    QLabel *m_sourceLabel = nullptr;
    std::unique_ptr<QTemporaryDir> m_tempDir;
};
