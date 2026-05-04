// QtWidgets 入门示例 24: QPlainTextEdit 纯文本高性能编辑器
// 演示：appendPlainText 追加日志的正确用法
//       setMaximumBlockCount 限制行数防内存溢出
//       highlightCurrentLine 实现行高亮效果

#pragma once

#include <QWidget>
#include "highlight_plain_text_edit.h"

class QPushButton;
class QComboBox;
class QLabel;
class QTimer;

// ============================================================================
// LogTerminal: 模拟日志终端，展示 QPlainTextEdit 的核心能力
// ============================================================================
class LogTerminal : public QWidget
{
    Q_OBJECT

public:
    explicit LogTerminal(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 开始模拟日志输出
    void startLogging();

    /// @brief 停止模拟日志输出
    void stopLogging();

    /// @brief 定时器回调: 随机生成一条日志并追加
    void onTimerTick();

    /// @brief 追加一条日志并自动滚动到底部
    void appendLogEntry(const QString &text);

    /// @brief 行数限制选择变化时更新 QPlainTextEdit 的上限
    void onLimitChanged(int index);

    /// @brief 更新状态栏的当前行数显示
    void updateLineCount();

private:
    HighlightPlainTextEdit *m_logView = nullptr;
    QPushButton *m_startBtn = nullptr;
    QPushButton *m_stopBtn = nullptr;
    QComboBox *m_limitCombo = nullptr;
    QLabel *m_lineCountLabel = nullptr;
    QLabel *m_limitLabel = nullptr;
    QTimer *m_timer = nullptr;
};
