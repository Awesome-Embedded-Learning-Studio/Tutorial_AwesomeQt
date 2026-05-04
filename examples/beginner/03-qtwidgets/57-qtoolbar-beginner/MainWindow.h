// QtWidgets 入门示例 57: QToolBar 工具栏
// 演示：addAction / addWidget / addSeparator
//       setMovable / setFloatable / setAllowedAreas
//       setIconSize / setToolButtonStyle
//       溢出菜单 / toggleViewAction

#include <QMainWindow>
#include <QMenu>
#include <QPlainTextEdit>
#include <QToolBar>

// ============================================================================
// MainWindow: 演示三个工具栏的完整配置
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // 菜单栏
    // ====================================================================
    void setupMenuBar();

    // ====================================================================
    // 文件工具栏：addAction + addSeparator
    // ====================================================================
    void setupFileToolBar();

    // ====================================================================
    // 格式工具栏：addWidget 嵌入自定义控件
    // ====================================================================
    void setupFormatToolBar();

    // ====================================================================
    // 绘图工具栏：限制停靠区域 + 文字在图标下方
    // ====================================================================
    void setupDrawingToolBar();

private:
    QPlainTextEdit *m_editor = nullptr;
    QToolBar *m_fileToolBar = nullptr;
    QToolBar *m_formatToolBar = nullptr;
    QToolBar *m_drawToolBar = nullptr;
    QMenu *m_viewMenu = nullptr;
};
