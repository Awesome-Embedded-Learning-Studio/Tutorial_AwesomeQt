// QtWidgets 入门示例 19: QRadioButton 单选按钮
// 演示：自动互斥：同一 parent 下单选按钮天然互斥
//       QButtonGroup 跨 parent 实现互斥分组
//       toggled(bool) 信号监听状态变化
//       自定义样式 QSS 圆形按钮美化

#pragma once

#include <QButtonGroup>
#include <QStringList>
#include <QWidget>

class QLabel;

// ============================================================================
// SettingsWidget: QRadioButton 综合演示 — 应用设置窗口
// ============================================================================
class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新底部状态标签
    void updateStatus();

    /// @brief 追加 toggled 信号日志
    void appendToggleLog(const QString &message);

private:
    QButtonGroup *m_themeGroup = nullptr;
    QButtonGroup *m_langGroup = nullptr;
    QButtonGroup *m_fontGroup = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_toggleLabel = nullptr;
    QStringList m_toggleLog;
};
