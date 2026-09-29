// QtWidgets 入门示例 06: 对话框体系基础
// FindDialog 实现

#include "finddialog.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("查找");
    setMinimumWidth(320);
    // 非模态对话框设置 WA_DeleteOnClose，关闭时自动销毁
    setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QVBoxLayout(this);

    m_findEdit = new QLineEdit;
    m_findEdit->setPlaceholderText("输入要查找的内容");
    layout->addWidget(m_findEdit);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Close
    );
    layout->addWidget(buttonBox);

    // 非模态对话框不用 exec()，所以用自定义信号通知调用方
    connect(buttonBox->button(QDialogButtonBox::Ok),
            &QPushButton::clicked, this, [this]() {
                emit findRequested(m_findEdit->text().trimmed());
            });
    connect(buttonBox->button(QDialogButtonBox::Close),
            &QPushButton::clicked, this, &QDialog::close);
}
