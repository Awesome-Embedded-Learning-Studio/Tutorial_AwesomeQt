// QtWidgets 入门示例 56: QMenuBar / QMenu / QAction 菜单系统
// 演示：menuBar()->addMenu() 添加顶级菜单
//       QAction 创建菜单项、图标、快捷键
//       setCheckable / QActionGroup / addSeparator
//       contextMenuEvent 右键上下文菜单
//       菜单与工具栏共享 QAction

#include <QAction>
#include <QActionGroup>
#include <QContextMenuEvent>
#include <QMainWindow>
#include <QPlainTextEdit>

// ============================================================================
// TextEditor: 带完整菜单系统和右键菜单的文本编辑器
// ============================================================================
class TextEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget *parent = nullptr);

protected:
    /// @brief 右键上下文菜单
    void contextMenuEvent(
        QContextMenuEvent *event) override;

private:
    /// @brief 初始化中央控件
    void initCentralWidget();

    /// @brief 创建所有 QAction
    void createActions();

    /// @brief 初始化菜单栏
    void initMenuBar();

    /// @brief 初始化工具栏（共享菜单的 QAction）
    void initToolBars();

    /// @brief 连接编辑器信号以动态更新菜单项状态
    void connectEditorSignals();

    /// @brief 字体大小变更
    void onFontSizeChanged(QAction *action);

    /// @brief 切换自动换行
    void toggleWordWrap(bool enabled);

private:
    QPlainTextEdit *m_editor = nullptr;

    // 文件 Action
    QAction *m_newAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_quitAction = nullptr;

    // 编辑 Action
    QAction *m_undoAction = nullptr;
    QAction *m_redoAction = nullptr;
    QAction *m_cutAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_pasteAction = nullptr;
    QAction *m_selectAllAction = nullptr;

    // 格式 Action
    QAction *m_smallFontAction = nullptr;
    QAction *m_mediumFontAction = nullptr;
    QAction *m_largeFontAction = nullptr;
    QAction *m_wordWrapAction = nullptr;
    QAction *m_showLineNumbersAction = nullptr;

    // 帮助 Action
    QAction *m_aboutAction = nullptr;
};
