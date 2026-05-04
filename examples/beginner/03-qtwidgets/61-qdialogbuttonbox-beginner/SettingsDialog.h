#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// QtWidgets 入门示例 61: QDialogButtonBox 标准按钮盒
// 演示：StandardButton 枚举组合
//       accepted / rejected / clicked 信号
//       button(StandardButton) 获取按钮实例
//       与 QDialog 布局结合的标准模板

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

// ============================================================================
// SettingsDialog: 标准按钮盒 + QDialog 的完整模板
// ============================================================================
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    /// @brief 获取应用名称
    QString appName() const { return m_nameEdit->text(); }

    /// @brief 获取最大连接数
    int maxConnections() const { return m_maxConnSpin->value(); }

    /// @brief 获取日志级别
    QString logLevel() const
    { return m_logLevelCombo->currentText(); }

private:
    static constexpr int kDefaultMaxConn = 10;
    static constexpr int kDefaultLogLevel = 1;  // "信息"

    /// @brief 带校验的 accept
    void tryAccept();

    /// @brief 处理非关闭按钮的点击
    void onButtonClicked(QAbstractButton *btn);

    /// @brief 校验输入并控制 OK 按钮状态
    bool validate();

    /// @brief 应用当前设置（不关闭对话框）
    void applySettings();

    /// @brief 恢复为默认值
    void resetToDefaults();

    QLineEdit *m_nameEdit = nullptr;
    QSpinBox *m_maxConnSpin = nullptr;
    QComboBox *m_logLevelCombo = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    QLabel *m_statusLabel = nullptr;
};

#endif // SETTINGSDIALOG_H
