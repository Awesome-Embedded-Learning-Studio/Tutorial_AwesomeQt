// QtWidgets 入门示例 71: QMdiArea / QMdiSubWindow 多文档界面
// 演示：addSubWindow 添加子窗口
//       tileSubWindows / cascadeSubWindows 排列
//       subWindowActivated 信号追踪活动窗口
//       子窗口菜单项自动更新

#include <QMainWindow>

class QMdiArea;
class QMdiSubWindow;
class QMenu;
class QAction;
class QTextEdit;

// ============================================================================
// MainWindow: MDI 多文档界面主窗口
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupMenus();
    void setupToolBar();

    // ====================================================================
    // 新建文档
    // ====================================================================
    void onNewDocument();

    // ====================================================================
    // 关闭当前活动子窗口
    // ====================================================================
    void onCloseCurrent();

    // ====================================================================
    // 复制 / 粘贴（作用于活动子窗口中的 QTextEdit）
    // ====================================================================
    void onCopy();

    void onPaste();

    // ====================================================================
    // 活动子窗口切换
    // ====================================================================
    void onSubWindowActivated(QMdiSubWindow *subWindow);

    // ====================================================================
    // 动态构建窗口菜单
    // ====================================================================
    void onUpdateWindowMenu();

    // ====================================================================
    // 更新状态栏
    // ====================================================================
    void updateStatusBar();

    /// @brief 获取活动子窗口中的 QTextEdit
    QTextEdit *activeTextEdit() const;

    QMdiArea *m_mdiArea = nullptr;
    QMenu *m_windowMenu = nullptr;
    QAction *m_closeAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_pasteAction = nullptr;
    int m_documentCounter = 0;
};
