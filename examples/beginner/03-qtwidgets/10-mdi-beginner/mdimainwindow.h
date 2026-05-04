// QtWidgets 入门示例 10: QMdiArea 多文档界面基础
// 演示：QMdiArea 子窗口创建与管理
//       QMdiSubWindow 标题、图标与关闭行为
//       子窗口排列模式（级联 / 平铺）
//       激活子窗口与信号监听
//       简易多文档文本编辑器

#ifndef MDIMAINWINDOW_H
#define MDIMAINWINDOW_H

#include <QMainWindow>
#include <QMdiSubWindow>

class QMdiArea;
class QMenu;
class QLabel;

// ============================================================================
// MdiMainWindow: MDI 多文档编辑器主窗口
// ============================================================================
class MdiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MdiMainWindow(QWidget *parent = nullptr);

private slots:
    /// @brief 子窗口激活时更新状态栏和窗口菜单
    void onSubWindowActivated(QMdiSubWindow *subWin);

    /// @brief 更新状态栏信息
    void updateStatusBar(QMdiSubWindow *active);

private:
    /// @brief 创建菜单栏
    void setupMenuBar();

    /// @brief 创建工具栏
    void setupToolBar();

    /// @brief 创建一个新的文档子窗口
    /// @param title 子窗口标题（为空时自动生成）
    /// @param text 编辑器初始文本
    void createNewDocument(const QString &title = QString(),
                           const QString &text = QString());

    /// @brief 动态更新"窗口"菜单，列出所有子窗口
    void updateWindowMenu();

protected:
    /// @brief 关闭主窗口前检查
    void closeEvent(QCloseEvent *event) override;

private:
    QMdiArea *m_mdiArea = nullptr;
    QMenu *m_windowMenu = nullptr;
    QLabel *m_statusDocLabel = nullptr;
    QLabel *m_statusCountLabel = nullptr;
    int m_docCounter = 0;
};

#endif // MDIMAINWINDOW_H
