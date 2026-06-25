/**
 * @file log_viewer.h
 * @brief 滚动日志控件 LogViewer——级别染色 + 自动滚底 + 行数上限裁旧
 * @copyright Copyright (c) 2026 AwesomeQt
 *
 * 组合 QPlainTextEdit（只读、非自绘）。append 时按级别着色、
 * 自动滚到底；超过 maxLines 时从顶部裁旧行，防内存无限膨胀。
 */
#pragma once

#include <QWidget>

class QPlainTextEdit;

namespace AwesomeQt {

/// @brief 滚动日志控件：级别染色 + 自动滚底 + 行数上限。
///
/// 封装一个只读 QPlainTextEdit。append 时按 Level 套前景色（Info 默认、
/// Warning 暗黄、Error 红），可选时间戳前缀；autoScroll 开启时 append 后
/// 滚到底；blockCount 超过 maxLines 时从文档头删旧行。
///
/// 边界：setMaxLines 夹到 >=1；空 message 照常 append（只显示时间戳+级别）；
/// 不重写 paintEvent，渲染完全交给 QPlainTextEdit。
class LogViewer : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：行为开关，可在 Designer / 外部直接驱动 ——
    Q_PROPERTY(int maxLines READ maxLines WRITE setMaxLines NOTIFY maxLinesChanged)
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll NOTIFY autoScrollChanged)
    Q_PROPERTY(
        bool showTimestamp READ showTimestamp WRITE setShowTimestamp NOTIFY showTimestampChanged)

  public:
    /// @brief 日志级别。决定 append 时套的前景色。
    enum class Level { Info, Warning, Error };
    Q_ENUM(Level)

    explicit LogViewer(QWidget* parent = nullptr);

    /// @brief 追加一条日志。
    /// @param level   级别（决定颜色）
    /// @param message 正文（可为空，此时只显示时间戳+级别）
    void append(Level level, const QString& message);

    /// @brief 便捷重载：追加一条 Info 级日志。
    void appendInfo(const QString& message);
    /// @brief 便捷重载：追加一条 Warning 级日志。
    void appendWarning(const QString& message);
    /// @brief 便捷重载：追加一条 Error 级日志。
    void appendError(const QString& message);

    /// @brief 清空所有日志。
    void clear();

    /// @brief 当前日志行数（block 数）。供 demo 观察裁旧行为。
    int lineCount() const;

    // —— Q_PROPERTY 读写 ——
    void setMaxLines(int maxLines);
    int maxLines() const;

    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;

    void setShowTimestamp(bool show);
    bool showTimestamp() const;

    QSize sizeHint() const override;

  signals:
    void maxLinesChanged(int maxLines);
    void autoScrollChanged(bool autoScroll);
    void showTimestampChanged(bool show);

  private:
    /// @brief 按 level 取前景色。Info 返回无效色（用默认），其余返回固定色。
    QColor colorForLevel(Level level) const;

    /// @brief 把 level 转成短标签字符串（INFO / WARN / ERROR）。
    static QString levelLabel(Level level);

    /// @brief 裁旧：若 blockCount 超过 maxLines，从文档头删多余块。
    void trimOldBlocks();

    QPlainTextEdit* view_{nullptr};
    int max_lines_{1000};
    bool auto_scroll_{true};
    bool show_timestamp_{true};
};

} // namespace AwesomeQt
