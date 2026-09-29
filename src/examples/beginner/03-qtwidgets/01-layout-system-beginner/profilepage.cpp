#include "profilepage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

ProfilePage::ProfilePage(QWidget *parent) : QWidget(parent)
{
    auto *formLayout = new QFormLayout(this);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(20, 20, 20, 20);

    auto *nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("请输入姓名");
    formLayout->addRow("姓名:", nameEdit);

    auto *emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("example@email.com");
    formLayout->addRow("邮箱:", emailEdit);

    auto *phoneEdit = new QLineEdit;
    phoneEdit->setPlaceholderText("13800138000");
    formLayout->addRow("电话:", phoneEdit);

    auto *cityCombo = new QComboBox;
    cityCombo->addItems({"北京", "上海", "广州", "深圳", "杭州"});
    formLayout->addRow("城市:", cityCombo);

    auto *bioEdit = new QTextEdit;
    bioEdit->setMaximumHeight(80);
    bioEdit->setPlaceholderText("简短介绍一下自己...");
    formLayout->addRow("简介:", bioEdit);

    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(new QPushButton("保存"));
    buttonLayout->addWidget(new QPushButton("重置"));
    formLayout->addRow(buttonLayout);
}
