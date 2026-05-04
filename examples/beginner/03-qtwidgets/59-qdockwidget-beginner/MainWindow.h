// QtWidgets 入门示例 59: QDockWidget 可停靠浮动面板
// 演示：setAllowedAreas 限制停靠位置
//       setFeatures 控制关闭/浮动/移动
//       topLevelChanged / dockLocationChanged 信号
//       多 Dock tabify 标签化合并

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// ============================================================================
// MainWindow: 模拟简易 IDE 布局
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // 创建 Dock 面板的通用方法
    // ====================================================================
    QDockWidget* createDock(const QString &title,
                             QWidget *widget,
                             Qt::DockWidgetArea area,
                             Qt::DockWidgetAreas allowedAreas);

    // ====================================================================
    // 停靠面板布局
    // ====================================================================
    void setupDockWidgets();

    // ====================================================================
    // 连接 Dock 状态信号
    // ====================================================================
    void connectDockSignals(QDockWidget *dock);

    // ====================================================================
    // 菜单栏
    // ====================================================================
    void setupMenuBar();

    // ====================================================================
    // 填充模拟文件树
    // ====================================================================
    void populateFileTree(QTreeWidget *tree);

private:
    QPlainTextEdit *m_editor = nullptr;
    QDockWidget *m_fileDock = nullptr;
    QDockWidget *m_symbolDock = nullptr;
    QDockWidget *m_bookmarkDock = nullptr;
    QDockWidget *m_outputDock = nullptr;
    QDockWidget *m_problemDock = nullptr;
};
