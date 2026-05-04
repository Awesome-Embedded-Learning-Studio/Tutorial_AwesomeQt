#ifndef USERCONFIGDIALOG_H
#define USERCONFIGDIALOG_H

// QtWidgets 入门示例 60: QDialog 自定义对话框
// 演示：模态 exec() vs 非模态 show()
//       accept() / reject() / done() 返回值约定
//       从对话框返回用户输入数据
//       setModal vs setWindowModality

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

// ============================================================================
// UserConfigDialog: 模态对话框 — 采集用户配置信息
// ============================================================================
class UserConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserConfigDialog(QWidget *parent = nullptr);

    /// @brief 获取用户名
    QString username() const { return m_nameEdit->text(); }

    /// @brief 获取年龄
    int age() const { return m_ageSpin->value(); }

    /// @brief 获取角色
    QString role() const { return m_roleCombo->currentText(); }

private:
    /// @brief 带校验的 accept — 校验失败则不关闭
    void tryAccept();

    QLineEdit *m_nameEdit = nullptr;
    QSpinBox *m_ageSpin = nullptr;
    QComboBox *m_roleCombo = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};

#endif // USERCONFIGDIALOG_H
