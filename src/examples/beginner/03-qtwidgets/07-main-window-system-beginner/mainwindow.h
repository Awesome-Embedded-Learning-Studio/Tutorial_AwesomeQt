// QtWidgets 入门示例 07: QMainWindow 主窗口体系基础
// 演示：QMenuBar / QMenu / QAction 菜单系统
//       QToolBar 工具栏按钮与分隔线
//       QStatusBar 状态栏（临时消息 + 永久组件）
//       QDockWidget 可停靠面板
//       简易文本编辑器

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QTextEdit;
class QLabel;
class QDockWidget;
class QTableWidget;
class QAction;

// ============================================================================
// MainWindow: 简易文本编辑器
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    // 菜单栏构建
    void setupMenuBar();

    // 工具栏构建
    void setupToolBar();

    // 状态栏构建
    void setupStatusBar();

    // 停靠面板构建
    void setupDockWidgets();

    /// @brief 设置文件信息面板中指定行的属性和值
    void setFileInfoRow(int row, const QString &property,
                        const QString &value);

private slots:
    // 文件操作
    void onNewFile();
    void onOpenFile();
    void onSaveFile();

    // 状态更新
    void updateCursorPosition();
    void updateFileInfo();

private:
    // ---- 成员变量 ----
    QTextEdit *m_textEdit = nullptr;
    QLabel *m_positionLabel = nullptr;
    QLabel *m_charCountLabel = nullptr;
    QDockWidget *m_fileInfoDock = nullptr;
    QTableWidget *m_fileInfoTable = nullptr;

    // QAction 成员变量：菜单和工具栏共享
    QAction *m_newAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_pasteAction = nullptr;

    QString m_currentFilePath;
};

#endif // MAINWINDOW_H
