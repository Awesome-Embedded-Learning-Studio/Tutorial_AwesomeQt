// QtWidgets 入门示例 18: QToolButton 工具栏专用按钮
// 演示：setToolButtonStyle（图标/文字/两者）
//       setPopupMode（菜单显示模式：延迟/即时/只有箭头）
//       与 QAction 关联：setDefaultAction()
//       在工具栏中自动调整样式的机制

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QToolBar;
class QTextEdit;
class QToolButton;

// ============================================================================
// MainWindow: QToolButton 综合演示主窗口
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 在 QTextEdit 中追加一行日志
    void log(const QString &message);

private:
    QToolBar *m_toolbar = nullptr;
    QTextEdit *m_textEdit = nullptr;
};

#endif // MAINWINDOW_H
