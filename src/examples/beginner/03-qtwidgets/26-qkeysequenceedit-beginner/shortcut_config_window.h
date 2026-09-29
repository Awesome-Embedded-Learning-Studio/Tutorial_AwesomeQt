// QtWidgets 入门示例 26: QKeySequenceEdit 快捷键录入控件
// 演示：keySequenceChanged 信号获取录入结果
//       setKeySequence 设置默认快捷键
//       与 QAction::setShortcut 结合的完整热键配置流程
//       冲突检测 + 日志记录

#pragma once

#include <QMainWindow>

class QAction;
class QKeySequenceEdit;
class QPlainTextEdit;

// ============================================================================
// ShortcutConfigWindow: 快捷键配置面板 + 菜单栏 + 触发日志
// ============================================================================
class ShortcutConfigWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShortcutConfigWindow(QWidget *parent = nullptr);

private:
    /// @brief 创建 QAction 并设置默认快捷键
    void createActions();

    /// @brief 创建菜单栏
    void createMenuBar();

    /// @brief 创建中央窗口: 配置面板 + 日志区域
    void createCentralWidget();

    /// @brief 应用新快捷键到 QAction，带冲突检测
    void applyShortcut(QKeySequenceEdit *edit,
                       QAction *action,
                       const QKeySequence &newSeq);

    /// @brief 检查快捷键是否与其他动作冲突
    bool isShortcutConflict(const QKeySequence &seq,
                            QAction *exclude) const;

    /// @brief QAction 被触发时的日志记录
    void onActionTriggered(const QString &actionName);

    /// @brief 恢复所有快捷键为默认值
    void resetDefaults();

    /// @brief 追加日志并自动滚动到底部
    void appendLog(const QString &text);

private:
    // QAction
    QAction *m_newAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_closeAction = nullptr;
    QList<QAction *> m_allActions;

    // QKeySequenceEdit
    QKeySequenceEdit *m_newEdit = nullptr;
    QKeySequenceEdit *m_saveEdit = nullptr;
    QKeySequenceEdit *m_closeEdit = nullptr;

    // 映射: edit -> action
    QMap<QKeySequenceEdit *, QAction *> m_editActionMap;

    // 日志
    QPlainTextEdit *m_logView = nullptr;
};
