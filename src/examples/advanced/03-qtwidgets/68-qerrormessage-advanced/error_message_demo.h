/// @file    error_message_demo.h
/// @brief   演示 QErrorMessage 结合 QSettings 持久化抑制状态。
///
/// 对应教程：进阶层 03-QtWidgets/68-QErrorMessage 进阶。
/// 核心知识点：QErrorMessage::showMessage() 展示错误、通过 QSettings
///             跟踪 "不再显示" 状态、重置抑制消息。

#pragma once

#include <QMainWindow>
#include <QSet>

class QLabel;
class QPushButton;
class QSettings;

/// @brief 主窗口，演示 QErrorMessage 与 QSettings 联合管理抑制状态。
class ErrorMessageDemo : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit ErrorMessageDemo(QWidget* parent = nullptr);

    /// @brief 析构函数，确保 QSettings 数据写入磁盘。
    ~ErrorMessageDemo() override;

private slots:
    /// @brief 触发 "文件未找到" 类型错误。
    void showFileNotFoundError();

    /// @brief 触发 "网络超时" 类型错误。
    void showNetworkTimeoutError();

    /// @brief 触发 "权限不足" 类型错误。
    void showPermissionDeniedError();

    /// @brief 清除 QSettings 中所有抑制记录，恢复全部错误提示。
    /// @note 仅清除本示例相关的键（suppressed_errors 组），不破坏其他应用设置。
    void resetSuppressedMessages();

private:
    /// @brief 初始化界面布局与信号槽连接。
    void setupUI();

    /// @brief 检查指定消息 ID 是否被抑制，若未抑制则弹出 QErrorMessage。
    /// @param[in] messageId  唯一消息标识符，用于 QSettings 键名。
    /// @param[in] title      对话框标题。
    /// @param[in] message    具体的错误描述文本。
    /// @note QErrorMessage 自身的 "不再显示" 复选框不区分消息 ID，
    ///       因此我们在弹出前自行判断 QSettings 状态。
    void showErrorIfNotSuppressed(const QString& messageId,
                                  const QString& title,
                                  const QString& message);

    /// @brief 将指定消息 ID 标记为已抑制并同步到 QSettings。
    /// @param[in] messageId 唯一消息标识符。
    void markAsSuppressed(const QString& messageId);

    /// @brief 更新状态标签，显示当前被抑制的消息数量。
    void updateSuppressedCount();

    QSettings* m_settings;          ///< 持久化存储，记录被抑制的消息 ID
    QSet<QString> m_suppressedIds;  ///< 运行时缓存，避免频繁读磁盘
    QLabel* m_statusLabel;          ///< 状态显示标签
};
