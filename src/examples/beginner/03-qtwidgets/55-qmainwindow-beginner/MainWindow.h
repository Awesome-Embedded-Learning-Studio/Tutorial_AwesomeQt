// QtWidgets 入门示例 55: QMainWindow 主窗口完整配置
// 演示：setCentralWidget 中央控件
//       菜单栏/工具栏/状态栏/Dock 完整搭建
//       saveGeometry/restoreGeometry 窗口尺寸持久化
//       多 Dock 窗口布局策略（tabify、嵌套）

#include <QAction>
#include <QCloseEvent>
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPlainTextEdit>
#include <QToolBar>

// ============================================================================
// MainWindow: 完整的主窗口配置
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    /// @brief 关闭事件：保存窗口状态
    void closeEvent(QCloseEvent *event) override;

private:
    /// @brief 初始化中央控件
    void initCentralWidget();

    /// @brief 初始化菜单栏
    void initMenuBar();

    /// @brief 初始化工具栏
    void initToolBars();

    /// @brief 初始化状态栏
    void initStatusBar();

    /// @brief 初始化 Dock 窗口
    void initDockWidgets();

    /// @brief 保存窗口状态到 QSettings
    void saveWindowState();

    /// @brief 从 QSettings 恢复窗口状态
    void restoreWindowState();

    /// @brief 恢复默认布局
    void resetLayout();

    /// @brief 更新状态栏中的光标位置
    void updateCursorPosition();

    /// @brief 更新字符统计
    void updateCharCount();

private:
    // 中央控件
    QPlainTextEdit *m_editor = nullptr;

    // 菜单 Action
    QAction *m_newAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_saveAction = nullptr;
    QMenu *m_viewMenu = nullptr;

    // 工具栏
    QToolBar *m_fileToolBar = nullptr;
    QToolBar *m_editToolBar = nullptr;

    // 状态栏控件
    QLabel *m_positionLabel = nullptr;
    QLabel *m_charCountLabel = nullptr;

    // Dock 窗口
    QDockWidget *m_outlineDock = nullptr;
    QDockWidget *m_outputDock = nullptr;
    QDockWidget *m_propertiesDock = nullptr;
};
