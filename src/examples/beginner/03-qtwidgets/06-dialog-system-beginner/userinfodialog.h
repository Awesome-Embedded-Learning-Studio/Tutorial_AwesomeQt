#pragma once

#include "userinfo.h"

#include <QDialog>

class QLineEdit;
class QSpinBox;
class QTextEdit;
class QLabel;
class QDialogButtonBox;

// UserInfoDialog: 个人信息编辑对话框（模态）
class UserInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserInfoDialog(QWidget *parent = nullptr);

    /// @brief 预填已有数据，每次打开对话框时调用
    void setData(const UserInfo &info);

    /// @brief 获取对话框中收集到的数据
    UserInfo getData() const;

private slots:
    /// @brief 输入验证 + accept：验证不通过则不关闭对话框
    void validateAndAccept();

private:
    QLineEdit *m_nameEdit = nullptr;
    QSpinBox *m_ageSpin = nullptr;
    QLineEdit *m_emailEdit = nullptr;
    QTextEdit *m_bioEdit = nullptr;
    QLabel *m_charCountLabel = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};
